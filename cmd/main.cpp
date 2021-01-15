/*
 * SPDX-FileCopyrightText: (C) 2021 Matthias Fehring / www.huessenbergnetz.de
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QFile>
#include <QSettings>
#include <QVariantMap>
#include <QTranslator>
#include <QLocale>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#ifdef WITH_SYSTEMD
#include <cstring>
extern "C"
{
#define SD_JOURNAL_SUPPRESS_LOCATION
#include <systemd/sd-journal.h>
}
#endif // WITH_SYSTEMD

#include "updater.h"

#ifdef WITH_SYSTEMD
void journaldMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    int prio = LOG_INFO;
    switch (type) {
    case QtDebugMsg:
        prio = LOG_DEBUG;
        break;
    case QtInfoMsg:
        prio = LOG_INFO;
        break;
    case QtWarningMsg:
        prio = LOG_WARNING;
        break;
    case QtCriticalMsg:
        prio = LOG_CRIT;
        break;
    case QtFatalMsg:
        prio = LOG_ALERT;
        break;
    }

    const QString ctx = QString::fromLatin1(context.category);
    const QString id = ctx == QLatin1String("default") ? QCoreApplication::applicationName() : QCoreApplication::applicationName() + QLatin1Char('.') + ctx;

#ifdef QT_DEBUG
    sd_journal_send("PRIORITY=%i", prio, "SYSLOG_FACILITY=%hhu", 1, "SYSLOG_IDENTIFIER=%s", qUtf8Printable(id), "SYSLOG_PID=%lli", QCoreApplication::applicationPid(), "MESSAGE=%s", qFormatLogMessage(type, context, msg).toUtf8().constData(), "CODE_FILE=%s", context.file, "CODE_LINE=%i", context.line, "CODE_FUNC=%s", context.function, NULL);
#else
    sd_journal_send("PRIORITY=%i", prio, "SYSLOG_FACILITY=%hhu", 1, "SYSLOG_IDENTIFIER=%s", qUtf8Printable(id), "SYSLOG_PID=%lli", QCoreApplication::applicationPid(), "MESSAGE=%s", qFormatLogMessage(type, context, msg).toUtf8().constData(), NULL);
#endif

    if (type == QtFatalMsg) {
        abort();
    }
}
#endif // WITH_SYSTEMD

QVariantMap loadConfig(const QString &filepath)
{
    QVariantMap map;

    QFileInfo cfi(filepath);

    if (Q_UNLIKELY(!cfi.exists())) {
        //: CLI error message, %1 will be replaced by the file path
        //% "Can not find configuration file at %1"
        qCritical("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_ERR_CONFIG_NOT_FOUND").arg(cfi.absoluteFilePath())));
        return map;
    }

    if (Q_UNLIKELY(!cfi.isReadable())) {
        //: CLI error message, %1 will be replaced by the file path
        //% "Can not read configuration file at %1. Permission denied."
        qCritical("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_ERR_CONFIG_UNREADABLE").arg(cfi.absoluteFilePath())));
        return map;
    }

    qDebug("Reading settings from %s", qUtf8Printable(cfi.absoluteFilePath()));

    QFile fi(filepath);

    if (Q_UNLIKELY(!fi.open(QIODevice::ReadOnly|QIODevice::Text))) {
        //: CLI error message, %1 will be replaced by the file path, %2 by the error string
        //% "Can not open configuration file at %1: %2"
        qCritical("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_ERR_CONFIG_FAILED_OPEN").arg(cfi.absoluteFilePath(), fi.errorString())));
        return map;
    }

    QJsonParseError jsonError;
    const QJsonDocument json = QJsonDocument::fromJson(fi.readAll(), &jsonError);
    fi.close();
    if (jsonError.error != QJsonParseError::NoError) {
        //: CLI error message, %1 will be replaced by the file path, %2 by the JSON error string
        //% "Failed to parse JSON configuration file at %1: %2"
        qCritical("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_ERR_JSON_PARSE").arg(cfi.absoluteFilePath(), jsonError.errorString())));
        return map;
    }

    if (!json.isObject()) {
        //: CLI error message, %1 will be replaced by the file path
        //% "JSON configuration file at %1 does not contain an object as root"
        qCritical("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_ERR_JSON_NO_OBJECT").arg(cfi.absoluteFilePath())));
        return map;
    }

    map = json.object().toVariantMap();

    return map;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("Huessenbergnetz"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("huessenbergnetz.de"));
    QCoreApplication::setApplicationName(QStringLiteral("funchotorn"));
    QCoreApplication::setApplicationVersion(QStringLiteral(FUNCHOTORN_VERSION));

    {
        qDebug("Loading translations from %s", FUNCHOTORN_TRANSDIR);
        const QLocale locale;
        auto trans = new QTranslator(&app);
        if (Q_LIKELY(trans->load(locale, QCoreApplication::applicationName(), QStringLiteral("_"), QStringLiteral(FUNCHOTORN_TRANSDIR), QStringLiteral(".qm")))) {
            if (Q_UNLIKELY(!QCoreApplication::installTranslator(trans))) {
                qWarning("Can not install translator for locale %s", qUtf8Printable(locale.name()));
            }
        } else {
            qWarning("Can not load translations for locale %s", qUtf8Printable(locale.name()));
        }
    }

    QCommandLineParser parser;

    QCommandLineOption configPath(QStringList({QStringLiteral("c"), QStringLiteral("config")}),
                                  //: Option description in the cli help, %1 will be replaced by the default path
                                  //% "Path to the configuration file to be used. Default: %1"
                                  qtTrId("FUNCHOTORN_CLI_OPT_CONFIG_PATH").arg(QStringLiteral(FUNCHOTORN_CONFIGFILE)),
                                  //: Option value name in the cli help for the configuration file path
                                  //% "file"
                                  qtTrId("FUNCHOTORN_CLI_OPT_CONFIG_PATH_VAL"),
                                  QStringLiteral(FUNCHOTORN_CONFIGFILE));
    parser.addOption(configPath);

    QCommandLineOption cacheDirPath(QStringLiteral("cache-dir"),
                                    //: Option description in the cli help, %1 will be replaced by the default path
                                    //% "Path to the cache directory to be used. Default: %1"
                                    qtTrId("FUNCHOTORN_CLI_OPT_CACHE_DIR").arg(QStringLiteral(FUNCHOTORN_CACHEDIR)),
                                    //: Option value name in the cli help for the cache directory path
                                    //% "directory"
                                    qtTrId("FUNCHOTORN_CLI_OPT_CACHE_DIR_VAL"),
                                    QStringLiteral(FUNCHOTORN_CACHEDIR));
    parser.addOption(cacheDirPath);

    QCommandLineOption dataDirPath(QStringLiteral("data-dir"),
                                   //: Option description in the cli help, %1 wil be replaced by the default path
                                   //% "Path to the directory to be used to serve the data from. Default: %1"
                                   qtTrId("FUNCHOTORN_CLI_OPT_DATA_DIR").arg(QStringLiteral(FUNCHOTORN_DATADIR)),
                                   //: OPtion value name in the cli help for the data directory path
                                   //% "directory"
                                   qtTrId("FUNCHOTORN_CLI_OPT_DATA_DIR_VAL"),
                                   QStringLiteral(FUNCHOTORN_DATADIR));
    parser.addOption(dataDirPath);

    QCommandLineOption update(QStringList({QStringLiteral("u"), QStringLiteral("update")}),
                              //: Option description in the cli help
                              //% "Run the update and fetch new MLS data."
                              qtTrId("FUNCHOTORN_CLI_OPT_UPDATE"));
    parser.addOption(update);

#ifdef WITH_SYSTEMD
    QCommandLineOption journald(QStringLiteral("journald"),
                                //: Option description in the cli help
                                //% "Write all output to systemd’s journal instead of stdout."
                                qtTrId("FUNCHOTORN_CLI_OPT_JOURNALD"));
    parser.addOption(journald);
#endif // WITH_SYSTEMD

    QCommandLineOption testEmail(QStringLiteral("test-email"),
                                 //: Option description in the cli help
                                 //% "Send a test email."
                                 qtTrId("FUNCHOTORN_CLI_OPT_TEST_EMAIL"));
    parser.addOption(testEmail);

    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(app);

#ifdef WITH_SYSTEMD
    if (parser.isSet(journald)) {
        qSetMessagePattern(QStringLiteral("%{message}"));
        qInstallMessageHandler(journaldMessageOutput);
    }
#endif // WITH_SYSTEMD

    if (parser.isSet(update)) {
        auto config = loadConfig(parser.value(configPath));
        if (config.empty()) {
            return 1;
        }

        auto updater = new Updater(config, &app);
        updater->setCacheDir(parser.value(cacheDirPath));
        updater->setDataDir(parser.value(dataDirPath));
        updater->start();

    } else if (parser.isSet(testEmail)) {
        auto config = loadConfig(parser.value(configPath));
        if (config.isEmpty()) {
            return 1;
        }

        auto updater = new Updater(config, &app);
        if (updater->sendTestMail()) {
            return 0;
        } else {
            return 4;
        }

    } else {
        parser.showHelp(1);
    }

    return QCoreApplication::exec();
}
