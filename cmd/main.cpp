/*
 * SPDX-FileCopyrightText: (C) 2021 Matthias Fehring / www.huessenbergnetz.de
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QSettings>
#include <QMap>
#include <QTranslator>
#include <QLocale>

#include "updater.h"

QMap<QString,QString> loadConfig(const QString &filepath)
{
    QMap<QString,QString> map;

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

    QSettings settings(cfi.absoluteFilePath(), QSettings::IniFormat);
    if (Q_UNLIKELY(settings.status() != QSettings::NoError)) {
        //: CLI error message, %1 will be replaced by the file path
        //% "Failed to read configuration file at %1. Please check the syntax of the configuration file."
        qCritical("%s", qUtf8Printable(qtTrId("FUNCHOTORN_CLI_ERR_CONFIG_INVALID").arg(cfi.absoluteFilePath())));
        return map;
    }

    settings.beginGroup(QStringLiteral("MlsDb"));
    map.insert(QStringLiteral("mlsdb_base_host"), settings.value(QStringLiteral("base_host"), QString()).toString());
    map.insert(QStringLiteral("mlsdb_base_path"), settings.value(QStringLiteral("base_path"), QString()).toString());
    settings.endGroup();

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

    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(app);

    if (parser.isSet(update)) {
        auto config = loadConfig(parser.value(configPath));
        if (config.empty()) {
            return 1;
        }

        config.insert(QStringLiteral("mlsdb_cache_dir"), parser.value(cacheDirPath));
        config.insert(QStringLiteral("mlsdb_data_dir"), parser.value(dataDirPath));

        auto updater = new Updater(config, &app);
        updater->start();

    } else {
        parser.showHelp(1);
    }

    return QCoreApplication::exec();
}
