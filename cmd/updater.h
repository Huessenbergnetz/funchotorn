/*
 * SPDX-FileCopyrightText: (C) 2021 Matthias Fehring / www.huessenbergnetz.de
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QMap>
#include <QProcess>
#include <QDir>
#include <QQueue>
#include <QJsonArray>
#include <QLocale>
#include <utility>
#include <chrono>

class QNetworkReply;
class QFile;
class QTemporaryDir;
class QProcess;

class Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater(const QMap<QString,QString> &config, QObject *parent = nullptr);

    ~Updater() override;

    void start();

private slots:
    void do_start();
    void onDownloadReadyRead();
    void onDownloadFinished();

private:
    void decompress();
    void startConversion();
    void doConversion();
    void createTarball();
    void compressTarball();
    void createChecksum();
    void writeJson();
    void setSymlink();
    void cleanup();

    void onDecompressFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onConversionFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onTarballFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onCompressionFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void handleError(const QString &msg, int exitCode);

    QString findExecutable(const QString &executable) const;

    QMap<QString,QString> m_config;
    QDir m_cacheDir;
    QDir m_dataDir;
    QQueue<std::pair<QString,QString>> m_countryQueue;
    const QString m_currentDateString;
    const QString m_mlsFileName;
    const QString m_geocluePath;
    const QString m_gunzipPath;
    const QString m_tarPath;
    const QString m_pixzPath;
    std::pair<QString,QString> m_currentCountry;
    QJsonArray m_jsonData;
    QLocale m_locale;
    QNetworkReply *m_reply = nullptr;
    QFile *m_mlsGzFile = nullptr;
    QTemporaryDir *m_countryTempDir = nullptr;
    QProcess *m_conversionProc = nullptr;
    QProcess *m_tarProc = nullptr;
    QProcess *m_xzProc = nullptr;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_timeStart;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_countryTimeStart;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_overallTimeStart;
};

#endif // UPDATER_H