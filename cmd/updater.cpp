/*
 * SPDX-FileCopyrightText: (C) 2021 Matthias Fehring / www.huessenbergnetz.de
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "updater.h"
#include <QTimer>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDateTime>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QHostInfo>
#include <QStandardPaths>
#include <SimpleMail/SimpleMail>

Updater::Updater(const QVariantMap &config, QObject *parent) :
    QObject(parent),
    m_config(config),
    m_currentDateString(QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyy-MM-dd"))),
    m_mlsFileName(QLatin1String("MLS-full-cell-export-") + m_currentDateString + QLatin1String("T000000.csv")),
    m_geocluePath(QStandardPaths::findExecutable(QStringLiteral("geoclue-mlsdb-tool"))),
    m_gunzipPath(QStandardPaths::findExecutable(QStringLiteral("gunzip"))),
    m_tarPath(QStandardPaths::findExecutable(QStringLiteral("tar"))),
    m_pixzPath(QStandardPaths::findExecutable(QStringLiteral("pixz")))
{

}

Updater::~Updater()
{

}

void Updater::start()
{
    QTimer::singleShot(0, this, &Updater::do_start);
}

void Updater::do_start()
{
    m_overallTimeStart = std::chrono::high_resolution_clock::now();

    if (Q_UNLIKELY(m_geocluePath.isEmpty())) {
        //: CLI error message, %1 will be replaced by the executable name
        //% "Can not find %1 executable"
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_CMD_NOT_FOUND").arg(QStringLiteral("geoclu-mlsdb-tool")), 1);
        return;
    }

    if (Q_UNLIKELY(m_gunzipPath.isEmpty())) {
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_CMD_NOT_FOUND").arg(QStringLiteral("gunzip")), 1);
        return;
    }

    if (Q_UNLIKELY(m_tarPath.isEmpty())) {
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_CMD_NOT_FOUND").arg(QStringLiteral("tar")), 1);
        return;
    }

    if (Q_UNLIKELY(m_pixzPath.isEmpty())) {
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_CMD_NOT_FOUND").arg(QStringLiteral("pixz")), 1);
        return;
    }

    if (Q_UNLIKELY(!m_cacheDir.exists())) {
        //: CLI error message, %1 will be replaced by the path
        //% "Can not find cache directory: %1"
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_CACHEDIR_NOT_EXISTS").arg(m_cacheDir.path()), 2);
        return;
    }

    QFileInfo cacheDirFi(m_cacheDir.path());

    if (Q_UNLIKELY(!cacheDirFi.isReadable() || !cacheDirFi.isWritable())) {
        //: CLI error message, %1 will be replaced by the path
        //% "The cache directory is eather not readable or not writabel: %1"
        handleError(qtTrId("FUNCHOTORNCLI_ERR_CACHEDIR_NOT_READWRITE").arg(cacheDirFi.absoluteFilePath()), 2);
        return;
    }

    if (Q_UNLIKELY(!m_dataDir.exists())) {
        //: CLI error message, %1 will be replaced by the path
        //% "Can not find data directory: %1"
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_DATADIR_NOT_EXISTS").arg(m_dataDir.path()), 2);
        return;
    }

    QFileInfo dataDirFi(m_dataDir.path());

    if (Q_UNLIKELY(!dataDirFi.isReadable() || !dataDirFi.isWritable())) {
        //: CLI error message, %1 will be replaced by the path
        //% "The data directory is eather not readable or not writabel: %1"
        handleError(qtTrId("FUNCHOTORNCLI_ERR_DATADIR_NOT_READWRITE").arg(cacheDirFi.absoluteFilePath()), 2);
        return;
    }

    const QVariantMap mlsdbConfig = m_config.value(QStringLiteral("mlsdb")).toMap();

    const QString mlsHost = mlsdbConfig.value(QStringLiteral("host")).toString().trimmed();
    if (Q_UNLIKELY(mlsHost.isEmpty())) {
        //: CLI error message
        //% "Can not download MLS data from an empty host name!"
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_EMPTY_HOST"), 1);
        return;
    }
    QString mlsPath = mlsdbConfig.value(QStringLiteral("path")).toString().trimmed();
    if (Q_UNLIKELY(!mlsPath.startsWith(QLatin1Char('/')))) {
        mlsPath.prepend(QLatin1Char('/'));
    }
    if (Q_LIKELY(!mlsPath.endsWith(QLatin1Char('/')))) {
        mlsPath.append(QLatin1Char('/'));
    }

    const QString mlsGzFileName = m_mlsFileName + QLatin1String(".gz");
    m_mlsGzFile = new QFile(m_cacheDir.absoluteFilePath(mlsGzFileName), this);

    if (m_mlsGzFile->exists()) {

        //: CLI info message, %1 will be replaced by the full file path
        //% "%1 already exists, skip downloading"
        qInfo("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_INFO_GZ_EXISTS").arg(m_mlsGzFile->fileName())));
        decompress();

    } else {

        if (Q_UNLIKELY(!m_mlsGzFile->open(QIODevice::WriteOnly|QIODevice::Append))) {
            //: CLI error message, %1 will be replaced by the file path
            //% "Failed to open file for writing: %1"
            handleError(qtTrId("FUNCHOTORN_CLI_ERR_FAILED_OPEN_DOWNLOAD_FILE_WRITE").arg(m_mlsGzFile->fileName()), 2);
            return;
        }

        QUrl url;
        url.setScheme(QStringLiteral("https"));
        url.setHost(mlsHost);
        mlsPath.append(mlsGzFileName);
        url.setPath(mlsPath);

        //% "Started download of %1"
        qInfo("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_INFO_START_DOWNLOAD").arg(url.toString())));
        qDebug("Saving data to %s", qUtf8Printable(m_mlsGzFile->fileName()));

        QNetworkRequest req(url);

        m_timeStart = std::chrono::high_resolution_clock::now();
        auto nam = new QNetworkAccessManager(this);
        m_reply = nam->get(req);
        connect(m_reply, &QNetworkReply::readyRead, this, &Updater::onDownloadReadyRead);
        connect(m_reply, &QNetworkReply::finished, this, &Updater::onDownloadFinished);

    }
}

void Updater::onDownloadReadyRead()
{
    m_mlsGzFile->write(m_reply->readAll());
}

void Updater::onDownloadFinished()
{
    const auto gzFileSize = m_mlsGzFile->size();
    m_mlsGzFile->close();

    if (m_reply->error() != QNetworkReply::NoError) {
        m_mlsGzFile->remove();
        //: CLI error message, %1 will be replaced by an error string
        //% "Failed to download MLS data: %1"
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_DOWNLOAD_ERR").arg(m_reply->errorString()), 3);
        return;
    }

    const auto timeEnd = std::chrono::high_resolution_clock::now();
    const auto timeUsed = static_cast<qint64>(std::chrono::duration_cast<std::chrono::seconds>(timeEnd - m_timeStart).count());

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    const QString sizeString = m_locale.toString(gzFileSize) + QLatin1String(" Bytes");
#else
    const QString sizeString = m_locale.formattedDataSize(gzFileSize);
#endif

    //% "Finished download of %1 (%2) in %3 seconds"
    qInfo("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_INFO_DOWNLOAD_FINISHED").arg(m_reply->url().toString(), sizeString, m_locale.toString(timeUsed))));

    m_reply->deleteLater();

    decompress();
}

void Updater::decompress()
{
    //: CLI info message, %1 will be replaced by the file path
    //% "Started decompression of %1"
    qInfo("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_INFO_DECOMPRESS_START").arg(m_mlsGzFile->fileName())));

    auto gunzip = new QProcess(this);
    gunzip->setWorkingDirectory(m_cacheDir.path());
    gunzip->setProgram(m_gunzipPath);
    gunzip->setArguments({QStringLiteral("-k"), QStringLiteral("-f"), m_mlsGzFile->fileName()});
    connect(gunzip, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
        [=](int exitCode, QProcess::ExitStatus exitStatus){ onDecompressFinished(exitCode, exitStatus); });
    m_timeStart = std::chrono::high_resolution_clock::now();
    gunzip->start();
}

void Updater::onDecompressFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    const auto timeEnd = std::chrono::high_resolution_clock::now();
    const auto timeUsed = static_cast<qint64>(std::chrono::duration_cast<std::chrono::seconds>(timeEnd - m_timeStart).count());

    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        //: CLI info message, %1 will be replaced by file name, %2 by time in seconds
        //% "Finished decompression of %1 in %2 seconds"
        qInfo("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_INFO_DECOMPRESS_FINISHED").arg(m_mlsGzFile->fileName(), m_locale.toString(timeUsed))));
        startConversion();
    } else {
        m_mlsGzFile->remove();
        QFile mlsCsvFile(m_cacheDir.absoluteFilePath(m_mlsFileName));
        mlsCsvFile.remove();

        //: CLI error message, %1 will be replaced by the local file path
        //% "Failed to decompress downloaded data at %1"
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_DECOMPRESSION_FAILED").arg(m_mlsGzFile->fileName()), 2);

        return;
    }
}

void Updater::startConversion()
{
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("dz"), QStringLiteral("Algeria")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ao"), QStringLiteral("Angola")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("bj"), QStringLiteral("Benin")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("bw"), QStringLiteral("Botswana")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("bf"), QStringLiteral("Burkina_Faso")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("bi"), QStringLiteral("Burundi")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("cm"), QStringLiteral("Cameroon")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ic"), QStringLiteral("Canary_Islands")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("cv"), QStringLiteral("Cape_Verde")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("cf"), QStringLiteral("Central_African_Republic")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ea"), QStringLiteral("Ceuta")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("td"), QStringLiteral("Chad")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("km"), QStringLiteral("Comoros")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("cg"), QStringLiteral("Congo")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("cd"), QStringLiteral("Democratic_Republic_of_the_Congo")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("dj"), QStringLiteral("Djibouti")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("eg"), QStringLiteral("Egypt")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("gq"), QStringLiteral("Equatorial_Guinea")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("er"), QStringLiteral("Eritrea")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("et"), QStringLiteral("Ethiopia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ga"), QStringLiteral("Gabon")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("gm"), QStringLiteral("Gambia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("gh"), QStringLiteral("Ghana")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("gn"), QStringLiteral("Guinea")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("gw"), QStringLiteral("Guinea_Bissau")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ci"), QStringLiteral("Ivory_Coast")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ke"), QStringLiteral("Kenya")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ls"), QStringLiteral("Lesotho")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("lr"), QStringLiteral("Liberia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ly"), QStringLiteral("Libya")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("mg"), QStringLiteral("Madagascar")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("xx"), QStringLiteral("Madeira")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("xx"), QStringLiteral("Malawi")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("mw"), QStringLiteral("Mali")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("mr"), QStringLiteral("Mauritania")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("yt"), QStringLiteral("Mayotte")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ea"), QStringLiteral("Melilla")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ms"), QStringLiteral("Morocco")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("mz"), QStringLiteral("Mozambique")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("na"), QStringLiteral("Namibia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ni"), QStringLiteral("Niger")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ng"), QStringLiteral("Nigeria")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("re"), QStringLiteral("Reunion")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("rw"), QStringLiteral("Rwanda")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("st"), QStringLiteral("Sao_Tome_and_Principe")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("sn"), QStringLiteral("Senegal")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("sc"), QStringLiteral("Seychelles")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("sl"), QStringLiteral("Sierra_Leone")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("so"), QStringLiteral("Somalia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("za"), QStringLiteral("South_Africa")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ss"), QStringLiteral("South_Sudan")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("sd"), QStringLiteral("Sudan")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("sz"), QStringLiteral("Swaziland")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("tz"), QStringLiteral("Tanzania")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("tg"), QStringLiteral("Togo")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("tn"), QStringLiteral("Tunisia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ug"), QStringLiteral("Uganda")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("zm"), QStringLiteral("Zambia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("zw"), QStringLiteral("Zimbabwe")));

    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ax"), QStringLiteral("Aland_Islands")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("al"), QStringLiteral("Albania")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ad"), QStringLiteral("Andorra")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("am"), QStringLiteral("Armenia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("at"), QStringLiteral("Austria")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("az"), QStringLiteral("Azerbaijan")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("by"), QStringLiteral("Belarus")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("be"), QStringLiteral("Belgium")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ba"), QStringLiteral("Bosnia_and_Herzegovina")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("bg"), QStringLiteral("Bulgaria")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("hr"), QStringLiteral("Croatia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("cy"), QStringLiteral("Cyprus")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("cz"), QStringLiteral("Czech_Republic")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("dk"), QStringLiteral("Denmark")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ee"), QStringLiteral("Estonia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("fo"), QStringLiteral("Faroe_Islands")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("fi"), QStringLiteral("Finland")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("fr"), QStringLiteral("France")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ge"), QStringLiteral("Georgia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("de"), QStringLiteral("Germany")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("gi"), QStringLiteral("Gibraltar")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("gr"), QStringLiteral("Greece")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("gg"), QStringLiteral("Guernsey")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("hu"), QStringLiteral("Hungary")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("xx"), QStringLiteral("Iceland")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("is"), QStringLiteral("Ireland")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("im"), QStringLiteral("Isle_of_Man")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("it"), QStringLiteral("Italy")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("je"), QStringLiteral("Jersey")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("jo"), QStringLiteral("Kazakhstan")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("xk"), QStringLiteral("Kosovo")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("lv"), QStringLiteral("Latvia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("li"), QStringLiteral("Liechtenstein")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("lt"), QStringLiteral("Lithuania")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("lu"), QStringLiteral("Luxembourg")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("mk"), QStringLiteral("Macedonia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("mt"), QStringLiteral("Malta")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("md"), QStringLiteral("Moldova")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("mc"), QStringLiteral("Monaco")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("me"), QStringLiteral("Montenegro")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("nl"), QStringLiteral("Netherlands")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("no"), QStringLiteral("Norway")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("pl"), QStringLiteral("Poland")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("pt"), QStringLiteral("Portugal")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ro"), QStringLiteral("Romania")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("sm"), QStringLiteral("San_Marino")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("rs"), QStringLiteral("Serbia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("sk"), QStringLiteral("Slovakia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("si"), QStringLiteral("Slovenia")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("es"), QStringLiteral("Spain")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("se"), QStringLiteral("Sweden")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ch"), QStringLiteral("Switzerland")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("ua"), QStringLiteral("Ukraine")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("gb"), QStringLiteral("United_Kingdom")));

    m_countryQueue.enqueue(std::make_pair(QStringLiteral("in"), QStringLiteral("India")));
    m_countryQueue.enqueue(std::make_pair(QStringLiteral("au"), QStringLiteral("Australia")));

    doConversion();
}

void Updater::doConversion()
{
    if (m_countryQueue.empty()) {
        cleanup();
        writeJson();
        setSymlink();
        const auto timeEnd = std::chrono::high_resolution_clock::now();
        const auto timeUsed = static_cast<qint32>(std::chrono::duration_cast<std::chrono::minutes>(timeEnd - m_overallTimeStart).count());
        // CLI info message
        //% "Finished converting MLS database in %1 minutes"
        qInfo("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_INFO_FINISHED").arg(m_locale.toString(timeUsed))));
        QCoreApplication::exit();
        return;
    }

    m_countryTimeStart = std::chrono::high_resolution_clock::now();

    if (!m_dataDir.exists(m_currentDateString)) {
        if (Q_UNLIKELY(!m_dataDir.mkdir(m_currentDateString))) {
            //: CLI error message, %1 will be replaced by the directory name to create, %2 by path to the parent directory
            //% "Failed to create directory %1 at %2"
            handleError(qtTrId("FUNCHOTORN_CLI_ERR_CREATE_DATEDIR_FAILED").arg(m_currentDateString, m_dataDir.path()), 2);
            return;
        }
    }

    m_currentCountry = m_countryQueue.dequeue();

    //: CLI info message, %1 will be replaced by the English country name
    //% "Started processing of %1"
    qInfo("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_INFO_START_COUNTR").arg(m_currentCountry.second)));

    //: CLI info message, %1 will be replaced by the English country name
    //% "Started database conversion of %1"
    qInfo("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_INFO_START_CONVERTING_COUNTRY").arg(m_currentCountry.second)));

    m_countryTempDir = new QTemporaryDir;

    m_conversionProc = new QProcess(this);
    m_conversionProc->setWorkingDirectory(m_countryTempDir->path());
    m_conversionProc->setProgram(m_geocluePath);
    m_conversionProc->setArguments({QStringLiteral("-c"), m_currentCountry.second, m_cacheDir.absoluteFilePath(m_mlsFileName)});
    connect(m_conversionProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
        [=](int exitCode, QProcess::ExitStatus exitStatus){ onConversionFinished(exitCode, exitStatus); });
    m_timeStart = std::chrono::high_resolution_clock::now();
    m_conversionProc->start();
}

void Updater::onConversionFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    auto timeEnd = std::chrono::high_resolution_clock::now();
    auto timeUsed = static_cast<qint64>(std::chrono::duration_cast<std::chrono::seconds>(timeEnd - m_timeStart).count());

    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        //: CLI info message, %1 will be replaced by the English country name, %2 by the time used in seconds
        //% "Finished database conversion of %1 in %2 seconds"
        qInfo("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_INFO_FINISH_CONVERTING_COUNTRY").arg(m_currentCountry.second, m_locale.toString(timeUsed))));
        m_conversionProc->deleteLater();
        m_conversionProc = nullptr;
        createTarball();
    } else {
        delete m_countryTempDir;
        m_countryTempDir = nullptr;

        //: CLI error message, %1 will be replaced by the English country name
        //% "Failed to convert MLS data for %1"
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_CONVERSION_FAILED").arg(m_currentCountry.second), 2);

        return;
    }
}

void Updater::createTarball()
{
    const QString tarName = m_currentCountry.second + QLatin1String(".tar");

    //: CLI info message, %1 will be replaced by the English country name
    //% "Started creating tarball for %1"
    qInfo("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_INFO_START_TARBALL").arg(m_currentCountry.second)));

    m_tarProc = new QProcess(this);
    m_tarProc->setWorkingDirectory(m_countryTempDir->path());
    m_tarProc->setProgram(m_tarPath);
    m_tarProc->setArguments({QStringLiteral("--create"), QStringLiteral("--file"), tarName, QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3"), QStringLiteral("4"), QStringLiteral("5"), QStringLiteral("6"), QStringLiteral("7"), QStringLiteral("8"), QStringLiteral("9")});
    connect(m_tarProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
        [=](int exitCode, QProcess::ExitStatus exitStatus){ onTarballFinished(exitCode, exitStatus); });
    m_timeStart = std::chrono::high_resolution_clock::now();
    m_tarProc->start();
}

void Updater::onTarballFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    auto timeEnd = std::chrono::high_resolution_clock::now();
    auto timeUsed = static_cast<qint64>(std::chrono::duration_cast<std::chrono::seconds>(timeEnd - m_timeStart).count());

    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        //: CLI info message, %1 will be replaced by the English country name, %2 by the time used in seconds
        //% "Finished creating tarball for %1 in %2 seconds"
        qInfo("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_INFO_FINISHED_TARBALL").arg(m_currentCountry.second, m_locale.toString(timeUsed))));
        m_tarProc->deleteLater();
        m_tarProc = nullptr;
        compressTarball();
    } else {
        delete m_countryTempDir;
        m_countryTempDir = nullptr;

        //: CLI error message, %1 will be replaced by the English country name
        //% "Failed to create tarball for %1"
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_TAR_FAILED").arg(m_currentCountry.second), 2);
        return;
    }
}

void Updater::compressTarball()
{
    const QString tarName = m_currentCountry.second + QLatin1String(".tar");
    const QString xzName = tarName + QLatin1String(".xz");

    const QDir dateDir = m_dataDir.absoluteFilePath(m_currentDateString);

    //: CLI info message, %1 will be replaced by the English country name
    //% "Started compressing tarball for %1"
    qInfo("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_INFO_START_COMPRESS").arg(m_currentCountry.second)));

    m_xzProc = new QProcess(this);
    m_xzProc->setWorkingDirectory(m_countryTempDir->path());
    m_xzProc->setProgram(m_pixzPath);
    m_xzProc->setArguments({QStringLiteral("-9"), tarName, dateDir.absoluteFilePath(xzName)});
    connect(m_xzProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
        [=](int exitCode, QProcess::ExitStatus exitStatus){ onCompressionFinished(exitCode, exitStatus); });
    m_timeStart = std::chrono::high_resolution_clock::now();
    m_xzProc->start();
}

void Updater::onCompressionFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    auto timeEnd = std::chrono::high_resolution_clock::now();
    auto timeUsed = static_cast<qint64>(std::chrono::duration_cast<std::chrono::seconds>(timeEnd - m_timeStart).count());

    if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
        //: CLI info message, %1 will be replaced be the English country name, %2 by the time used in seconds
        //% "Finished compressing tarball for %1 in %2 seconds"
        qInfo("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_INFO_FINISHED_COMPRESS").arg(m_currentCountry.second, m_locale.toString(timeUsed))));
        m_xzProc->deleteLater();
        m_xzProc = nullptr;
        delete m_countryTempDir;
        m_countryTempDir = nullptr;
        createChecksum();
    } else {
        delete m_countryTempDir;
        m_countryTempDir = nullptr;

        //: CLI error message, %1 will be replaced by the English country name
        //% "Failed to compress tarball for %1"
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_XZ_FAILED").arg(m_currentCountry.second), 2);
        return;
    }
}

void Updater::createChecksum()
{
    QFile xzFile(m_dataDir.absoluteFilePath(m_currentDateString + QLatin1Char('/') + m_currentCountry.second + QLatin1String(".tar.xz")));

    if (Q_UNLIKELY(!xzFile.open(QIODevice::ReadOnly))) {
        //: CLI error message, %1 will be replaced by the file path
        //% "Failed to open %1 to create checksum"
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_HASH_FAILED").arg(xzFile.fileName()), 2);
        return;
    }

    const qint64 xzFileSize = xzFile.size();

    QCryptographicHash hasher(QCryptographicHash::Sha256);
    hasher.addData(&xzFile);

    const QByteArray hash = hasher.result();
    const QString hexHash = QString::fromLatin1(hash.toHex());

    QJsonObject o;
    o.insert(QStringLiteral("cc"), m_currentCountry.first);
    o.insert(QStringLiteral("name"), m_currentCountry.second);
    o.insert(QStringLiteral("sha256"), hexHash);
    o.insert(QStringLiteral("size"), xzFileSize);

    m_jsonData.append(o);

    auto timeEnd = std::chrono::high_resolution_clock::now();
    auto timeUsed = static_cast<qint64>(std::chrono::duration_cast<std::chrono::seconds>(timeEnd - m_countryTimeStart).count());

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
    const QString sizeString = m_locale.toString(xzFileSize) + QLatin1String(" Bytes");
#else
    const QString sizeString = m_locale.formattedDataSize(xzFileSize);
#endif

    //: CLI info message, %1 will be replaced by the English country name, %2 by the duration in seconds, %3 by the size in bytes and %4 by the sha256 checksum
    //% "Finished processing of %1 in %2 seconds. Size: %3, SHA256: %4"
    qInfo("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_INFO_COUNTRY_FINISHED").arg(m_currentCountry.second, m_locale.toString(timeUsed), sizeString, hexHash)));

    doConversion();
}

void Updater::writeJson()
{
    QFile jsonFile(m_dataDir.absoluteFilePath(m_currentDateString + QLatin1String("/data.json")));

    qDebug("Writing JSON data to %s", qUtf8Printable(jsonFile.fileName()));

    if (Q_UNLIKELY(!jsonFile.open(QIODevice::WriteOnly))) {
        //: CLI error message, %1 will be replaced by the file path
        //% "Failed to open %1 for writing"
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_OPEN_JSON_FILE_FAILED").arg(jsonFile.fileName()), 2);
        return;
    }

    QJsonObject o;
    o.insert(QStringLiteral("creationTime"), QDateTime::currentDateTimeUtc().toSecsSinceEpoch());
    o.insert(QStringLiteral("data"), m_jsonData);

    QJsonDocument doc(o);

    if (Q_UNLIKELY(jsonFile.write(doc.toJson(QJsonDocument::Indented)) < 0)) {
        //: CLI error message, %1 will be replaced by the file path
        //% "Failed to write JSON data to %1"
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_WRITE_JSON_FAILED").arg(jsonFile.fileName()), 2);
        return;
    }
}

void Updater::setSymlink()
{
    {
        QFile currentLink(m_dataDir.absoluteFilePath(QStringLiteral("latest")));
        if (Q_LIKELY(currentLink.exists())) {
            qDebug("Removing old symlink to latest data: %s", qUtf8Printable(currentLink.fileName()));
            if (Q_UNLIKELY(!currentLink.remove())) {
                //: CLI error message, %1 will be replaced by the path to the link
                //% "Failed to remove old symbolic link at %1"
                handleError(qtTrId("FUNCHOTORN_CLI_ERR_FAILED_REMOVE_LINK").arg(currentLink.fileName()), 2);
                return;
            }
        }
    }

    const QString currentPath = QDir::currentPath();
    QDir::setCurrent(m_dataDir.absolutePath());

    qDebug("Creating new symlink %s -> %s", qUtf8Printable(m_dataDir.absoluteFilePath(QStringLiteral("latest"))), qUtf8Printable(m_currentDateString));

    if (Q_UNLIKELY(!QFile::link(m_currentDateString, QStringLiteral("latest")))) {
        //: CLI error message, %1 will be replaced by the path to the link
        //% "Failed to create new symbolic link to latest data at %1"
        handleError(qtTrId("FUNCHOTORN_CLI_ERR_FAILED_CREATE_LINK").arg(m_dataDir.absoluteFilePath(QStringLiteral("latest"))), 2);
        return;
    }

    QDir::setCurrent(currentPath);
}

void Updater::cleanup() const
{
    QFile mlsCsvFile(m_cacheDir.absoluteFilePath(m_mlsFileName));
    mlsCsvFile.remove();
}

void Updater::handleError(const QString &msg, int exitCode) const
{
    qCritical("%s", qUtf8Printable(msg));

    //: error notification email body intro text
    //% "While trying to update the MLS database, the following error occured:"
    const QString errMsg = qtTrId("FUNCHOTORN_MAIL_ERROR_BODY_INTRO") + QLatin1String("\n\n") + msg;

    //: error notification email subject
    //% "Error while updating MLS database"
    sendMail(qtTrId("FUNCHOTORN_MAIL_ERROR_SUBJECT"), errMsg, MailType::Error);
    QCoreApplication::exit(exitCode);
}

bool Updater::sendMail(const QString &subject, const QString &msg, MailType mailType) const
{
    const QVariantMap conf = m_config.value(QStringLiteral("mail")).toMap();
    const bool notifyOnError = conf.value(QStringLiteral("notifyOnError"), false).toBool();
    const bool notifyOnSuccess = conf.value(QStringLiteral("notifyOnSuccess"), false).toBool();

    if (mailType == MailType::Error && !notifyOnError) {
        return true;
    }

    if (mailType == MailType::Success && !notifyOnSuccess) {
        return true;
    }

    const QString user = conf.value(QStringLiteral("user")).toString().trimmed();
    const QString pass = conf.value(QStringLiteral("password")).toString().trimmed();
    const QString host = conf.value(QStringLiteral("host"), QStringLiteral("localhost")).toString().trimmed();
    const int port = conf.value(QStringLiteral("port"), 465).toInt();

    const QString authMethodString = conf.value(QStringLiteral("authMethod"), QStringLiteral("plain")).toString().trimmed();
    SimpleMail::Sender::AuthMethod authMethod = SimpleMail::Sender::AuthPlain;
    if (authMethodString.compare(QLatin1String("plain"), Qt::CaseInsensitive) == 0) {
        authMethod = SimpleMail::Sender::AuthPlain;
    } else if (authMethodString.compare(QLatin1String("login"), Qt::CaseInsensitive) == 0) {
        authMethod = SimpleMail::Sender::AuthLogin;
    } else if (authMethodString.compare(QLatin1String("crammd5"), Qt::CaseInsensitive) == 0) {
        authMethod = SimpleMail::Sender::AuthCramMd5;
    } else if (authMethodString.compare(QLatin1String("none"), Qt::CaseInsensitive) == 0) {
        authMethod = SimpleMail::Sender::AuthNone;
    } else {
        qWarning("Can not send email. Invalid authentication method: %s", qUtf8Printable(authMethodString));
        return false;
    }

    const QString encryptionString = conf.value(QStringLiteral("encryption"), QStringLiteral("ssl")).toString().trimmed();
    SimpleMail::Sender::ConnectionType conType = SimpleMail::Sender::SslConnection;
    if (encryptionString.compare(QLatin1String("ssl"), Qt::CaseInsensitive) == 0 || encryptionString.compare(QLatin1String("tls")) == 0) {
        conType = SimpleMail::Sender::SslConnection;
    } else if (encryptionString.compare(QLatin1String("starttls"), Qt::CaseInsensitive) == 0) {
        conType = SimpleMail::Sender::TlsConnection;
    } else if (encryptionString.compare(QLatin1String("none")) == 0) {
        conType = SimpleMail::Sender::TcpConnection;
    } else {
        qWarning("Can not send email. Invalid encryption type: %s", qUtf8Printable(encryptionString));
        return false;
    }

    const QString fromAddress = conf.value(QStringLiteral("fromAddress")).toString().trimmed();
    if (fromAddress.isEmpty()) {
        qWarning("%s", "Can not send email. Empty sender address.");
        return false;
    }
    const QString fromName = conf.value(QStringLiteral("fromName")).toString().trimmed();
    SimpleMail::EmailAddress senderAddress = fromName.isEmpty() ? SimpleMail::EmailAddress(fromAddress) : SimpleMail::EmailAddress(fromAddress, fromName);

    const QString toAddress = conf.value(QStringLiteral("toAddress")).toString().trimmed();
    if (toAddress.isEmpty()) {
        qWarning("%s", "Cand not send email. Empty recipient address.");
        return false;
    }
    const QString toName = conf.value(QStringLiteral("toName")).toString().trimmed();
    SimpleMail::EmailAddress recipientAddress = toName.isEmpty() ? SimpleMail::EmailAddress(toAddress) : SimpleMail::EmailAddress(toAddress, toName);

    SimpleMail::Sender sender(host, port, conType);
    sender.setUser(user);
    sender.setPassword(pass);
    sender.setAuthMethod(authMethod);

    SimpleMail::MimeMessage message;
    message.setSender(senderAddress);
    message.addTo(recipientAddress);
    message.setSubject(subject);

    //: email body signature, %1 will be replaced by the application name, %2 by the app version and %3 by the local host name
    //% "This message was generated automatically by %1 %2 on %3."
    const QString mailBody = msg + QLatin1String("\n\n---\n") + qtTrId("FUNCHOTORN_MAIL_BODY_SIGNATURE").arg(QCoreApplication::applicationName(), QCoreApplication::applicationVersion(), QHostInfo::localHostName());

    auto text = new SimpleMail::MimeText(mailBody);

    message.addPart(text);

    if (Q_UNLIKELY(!sender.sendMail(message))) {
        qWarning("Failed to send email: %s", qUtf8Printable(sender.lastError()));
        return false;
    }

    return true;
}

void Updater::setCacheDir(const QString &path)
{
    m_cacheDir.setPath(path);
}

void Updater::setDataDir(const QString &path)
{
    m_dataDir.setPath(path);
}

bool Updater::sendTestMail() const
{
    //: test email subject
    //% "Funchotorn Test Mail"
    return sendMail(qtTrId("FUNCHOTORN_TEST_MAIL_SUBJECT"),
                    //: test email body
                    //% "This is a test email to check if your email settings are working."
                    qtTrId("FUNCHOTORN_TEST_MAIL_BODY"),
                    MailType::Forced);
}

#include "moc_updater.cpp"
