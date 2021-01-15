<?xml version="1.0" ?><!DOCTYPE TS><TS language="de" sourcelanguage="en" version="2.1">
<context>
    <name/>
    <message id="FUNCHOTORN_CLI_ERR_CONFIG_NOT_FOUND">
        <source>Can not find configuration file at %1</source>
        <extracomment>CLI error message, %1 will be replaced by the file path</extracomment>
        <translation>Kann die Konfigurationsdatei unter %1 nicht finden</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_CONFIG_UNREADABLE">
        <source>Can not read configuration file at %1. Permission denied.</source>
        <extracomment>CLI error message, %1 will be replaced by the file path</extracomment>
        <translation>Kann die Konfigurationsdatei unter %1 nicht lesen. Zugriff verweigert.</translation>
    </message>
    <message id="FUNCHOTORN_CLI_OPT_CONFIG_PATH">
        <source>Path to the configuration file to be used. Default: %1</source>
        <extracomment>Option description in the cli help, %1 will be replaced by the default path</extracomment>
        <translation>Pfad zur Konfigurationsdatei. Standard: %1</translation>
    </message>
    <message id="FUNCHOTORN_CLI_OPT_CONFIG_PATH_VAL">
        <source>file</source>
        <extracomment>Option value name in the cli help for the configuration file path</extracomment>
        <translation>Datei</translation>
    </message>
    <message id="FUNCHOTORN_CLI_OPT_CACHE_DIR">
        <source>Path to the cache directory to be used. Default: %1</source>
        <extracomment>Option description in the cli help, %1 will be replaced by the default path</extracomment>
        <translation>Pfad zum Cache-Verzeichnis. Standard: %1</translation>
    </message>
    <message id="FUNCHOTORN_CLI_OPT_CACHE_DIR_VAL">
        <source>directory</source>
        <extracomment>Option value name in the cli help for the cache directory path</extracomment>
        <translation>Verzeichnis</translation>
    </message>
    <message id="FUNCHOTORN_CLI_OPT_DATA_DIR">
        <source>Path to the directory to be used to serve the data from. Default: %1</source>
        <extracomment>Option description in the cli help, %1 wil be replaced by the default path</extracomment>
        <translation>Pfad zum Verzeichnis aus dem die Daten bereitgestellt werden. Standard: %1</translation>
    </message>
    <message id="FUNCHOTORN_CLI_OPT_DATA_DIR_VAL">
        <source>directory</source>
        <extracomment>OPtion value name in the cli help for the data directory path</extracomment>
        <translation>Verzeichnis</translation>
    </message>
    <message id="FUNCHOTORN_CLI_OPT_UPDATE">
        <source>Run the update and fetch new MLS data.</source>
        <extracomment>Option description in the cli help</extracomment>
        <translation>Starte die Aktualisierung und hole neue MLS-Daten.</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_CACHEDIR_NOT_EXISTS">
        <source>Can not find cache directory: %1</source>
        <extracomment>CLI error message, %1 will be replaced by the path</extracomment>
        <translation>Kann Cache-Verzeichnis nicht finden: %1</translation>
    </message>
    <message id="FUNCHOTORNCLI_ERR_CACHEDIR_NOT_READWRITE">
        <source>The cache directory is eather not readable or not writabel: %1</source>
        <extracomment>CLI error message, %1 will be replaced by the path</extracomment>
        <translation>Das Cache-Verzeichnis ist entweder nicht les- oder nicht beschreibbar: %1</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_DATADIR_NOT_EXISTS">
        <source>Can not find data directory: %1</source>
        <extracomment>CLI error message, %1 will be replaced by the path</extracomment>
        <translation>Kann das Datenverzeichnis nicht finden: %1</translation>
    </message>
    <message id="FUNCHOTORNCLI_ERR_DATADIR_NOT_READWRITE">
        <source>The data directory is eather not readable or not writabel: %1</source>
        <extracomment>CLI error message, %1 will be replaced by the path</extracomment>
        <translation>Das Datenverzeichnis ist entweder nicht les- oder nicht beschreibbar: %1</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_EMPTY_HOST">
        <source>Can not download MLS data from an empty host name!</source>
        <extracomment>CLI error message</extracomment>
        <translation>Kann MLS-Daten nicht ohne Hostnamen herunterladen!</translation>
    </message>
    <message id="FUNCHOTORN_CLI_INFO_GZ_EXISTS">
        <source>%1 already exists, skip downloading</source>
        <extracomment>CLI info message, %1 will be replaced by the full file path</extracomment>
        <translation>%1 existiert bereit, überspringe Herunterladen</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_FAILED_OPEN_DOWNLOAD_FILE_WRITE">
        <source>Failed to open file for writing: %1</source>
        <extracomment>CLI error message, %1 will be replaced by the file path</extracomment>
        <translation>Konnte Datei nicht zum Schreiben öffnen: %1</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_DOWNLOAD_ERR">
        <source>Failed to download MLS data: %1</source>
        <extracomment>CLI error message, %1 will be replaced by an error string</extracomment>
        <translation>MLS-Daten konnten nicht heruntergeladen werden: %1</translation>
    </message>
    <message id="FUNCHOTORN_CLI_INFO_DECOMPRESS_START">
        <source>Started decompression of %1</source>
        <oldsource>Starting decompression of %1</oldsource>
        <extracomment>CLI info message, %1 will be replaced by the file path</extracomment>
        <translation>Entpacken von %1 gestartet</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_DECOMPRESSION_FAILED">
        <source>Failed to decompress downloaded data at %1</source>
        <oldsource>Failed to decompress downloaded data at %s</oldsource>
        <extracomment>CLI error message, %1 will be replaced by the local file path</extracomment>
        <translation>Heruntergeladene Daten unter %1 konnten nicht entpackt werden</translation>
    </message>
    <message id="FUNCHOTORN_CLI_INFO_FINISHED">
        <source>Finished converting MLS database in %1 minutes</source>
        <oldsource>Finished converting MLS database</oldsource>
        <translation>Das Konvertieren der MLS-Datenbank wurde innerhalb von %1 Minuten abgeschlossen</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_CREATE_DATEDIR_FAILED">
        <source>Failed to create directory %1 at %2</source>
        <oldsource>Failed to create directory %1 at %2
</oldsource>
        <extracomment>CLI error message, %1 will be replaced by the directory name to create, %2 by path to the parent directory</extracomment>
        <translation>Konnte das Verzeichnis %1 nicht unter %2 erstellen</translation>
    </message>
    <message id="FUNCHOTORN_CLI_INFO_START_CONVERTING_COUNTRY">
        <source>Started database conversion of %1</source>
        <oldsource>Converting %1</oldsource>
        <extracomment>CLI info message, %1 will be replaced by the English country name</extracomment>
        <translation>Datenbankkonvertierung für %1 gestartet</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_CONVERSION_FAILED">
        <source>Failed to convert MLS data for %1</source>
        <extracomment>CLI error message, %1 will be replaced by the English country name</extracomment>
        <translation>MLS-Daten für %1 konnten nicht konvertiert werden</translation>
    </message>
    <message id="FUNCHOTORN_CLI_INFO_START_TARBALL">
        <source>Started creating tarball for %1</source>
        <oldsource>Creating tarball for %1</oldsource>
        <extracomment>CLI info message, %1 will be replaced by the English country name</extracomment>
        <translation>Erstellen von Tarball für %1 gestartet</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_TAR_FAILED">
        <source>Failed to create tarball for %1</source>
        <extracomment>CLI error message, %1 will be replaced by the English country name</extracomment>
        <translation>Tarball für %1 konnte nicht erstellt werden</translation>
    </message>
    <message id="FUNCHOTORN_CLI_INFO_START_COMPRESS">
        <source>Started compressing tarball for %1</source>
        <oldsource>Compressing tarball %1</oldsource>
        <extracomment>CLI info message, %1 will be replaced by the English country name</extracomment>
        <translation>Komprimierung von Tarball für %1 gestartet</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_XZ_FAILED">
        <source>Failed to compress tarball for %1</source>
        <extracomment>CLI error message, %1 will be replaced by the English country name</extracomment>
        <translation>Tarball für %1 konnte nicht komprimiert werden</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_HASH_FAILED">
        <source>Failed to open %1 to create checksum</source>
        <extracomment>CLI error message, %1 will be replaced by the file path</extracomment>
        <translation>Erstellen der Prüfsumme fehlgeschlagen, %1 konnte nicht geöffnet werden</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_OPEN_JSON_FILE_FAILED">
        <source>Failed to open %1 for writing</source>
        <extracomment>CLI error message, %1 will be replaced by the file path</extracomment>
        <translation>%1 konnte nicht zum Beschreiben geöffnet werden</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_WRITE_JSON_FAILED">
        <source>Failed to write JSON data to %1</source>
        <extracomment>CLI error message, %1 will be replaced by the file path</extracomment>
        <translation>JSON-Daten konnten nicht in %1 geschrieben werden</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_FAILED_REMOVE_LINK">
        <source>Failed to remove old symbolic link at %1</source>
        <oldsource>Failed to remove old linkt at %1</oldsource>
        <extracomment>CLI error message, %1 will be replaced by the path to the link</extracomment>
        <translation>Symbolischer Link unter %1 konnte nicht entfernt werden</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_FAILED_CREATE_LINK">
        <source>Failed to create new symbolic link to latest data at %1</source>
        <oldsource>Failed to create new symbolic link to latest data %1</oldsource>
        <extracomment>CLI error message, %1 will be replaced by the path to the link</extracomment>
        <translation>Symbolischer Link zu aktuellen Daten unter %1 konnte nicht erstellt werden</translation>
    </message>
    <message id="FUNCHOTORN_CLI_INFO_COUNTRY_FINISHED">
        <source>Finished processing of %1 in %2 seconds. Size: %3, SHA256: %4</source>
        <oldsource>Finished processing of %1 in %2 seconds. Size: %3 bytes, SHA256: %4</oldsource>
        <extracomment>CLI info message, %1 will be replaced by the English country name, %2 by the duration in seconds, %3 by the size in bytes and %4 by the sha256 checksum</extracomment>
        <translation>Verarbeitung von %1 in %2 Sekunden abgeschlossen. Größe: %3, SHA256: %4</translation>
    </message>
    <message id="FUNCHOTORN_CLI_INFO_START_DOWNLOAD">
        <source>Started download of %1</source>
        <translation>Herunterladen von %1 gestartet</translation>
    </message>
    <message id="FUNCHOTORN_CLI_INFO_DOWNLOAD_FINISHED">
        <source>Finished download of %1 (%2) in %3 seconds</source>
        <translation>Herunterladen von %1 (%2) in %3 Sekunden abgeschlossen</translation>
    </message>
    <message id="FUNCHOTORN_CLI_INFO_DECOMPRESS_FINISHED">
        <source>Finished decompression of %1 in %2 seconds</source>
        <extracomment>CLI info message, %1 will be replaced by file name, %2 by time in seconds</extracomment>
        <translation>Entpacken von %1 in %2 Sekunden abgeschlossen</translation>
    </message>
    <message id="FUNCHOTORN_CLI_INFO_START_COUNTR">
        <source>Started processing of %1</source>
        <extracomment>CLI info message, %1 will be replaced by the English country name</extracomment>
        <translation>Verarbeitung von %1 gestartet</translation>
    </message>
    <message id="FUNCHOTORN_CLI_INFO_FINISH_CONVERTING_COUNTRY">
        <source>Finished database conversion of %1 in %2 seconds</source>
        <extracomment>CLI info message, %1 will be replaced by the English country name, %2 by the time used in seconds</extracomment>
        <translation>Datenbankkonvertierung für %1 in %2 Sekunden abgeschlossen</translation>
    </message>
    <message id="FUNCHOTORN_CLI_INFO_FINISHED_TARBALL">
        <source>Finished creating tarball for %1 in %2 seconds</source>
        <extracomment>CLI info message, %1 will be replaced by the English country name, %2 by the time used in seconds</extracomment>
        <translation>Erstellen von Tarball für %1 in %2 Sekunden abgeschlossen</translation>
    </message>
    <message id="FUNCHOTORN_CLI_INFO_FINISHED_COMPRESS">
        <source>Finished compressing tarball for %1 in %2 seconds</source>
        <extracomment>CLI info message, %1 will be replaced be the English country name, %2 by the time used in seconds</extracomment>
        <translation>Komprimierung von Tarball für %1 in %2 Sekunden abgeschlossen</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_CONFIG_FAILED_OPEN">
        <source>Can not open configuration file at %1: %2</source>
        <extracomment>CLI error message, %1 will be replaced by the file path, %2 by the error string</extracomment>
        <translation>Kann die Konfigurationsdatei unter %1 nicht öffnen: %2</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_JSON_PARSE">
        <source>Failed to parse JSON configuration file at %1: %2</source>
        <extracomment>CLI error message, %1 will be replaced by the file path, %2 by the JSON error string</extracomment>
        <translation>JSON-Daten in Konfigurationsdatei unter %1 können nicht verarbeitet werden: %2</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_JSON_NO_OBJECT">
        <source>JSON configuration file at %1 does not contain an object as root</source>
        <extracomment>CLI error message, %1 will be replaced by the file path</extracomment>
        <translation>Die JSON-Daten in der Konfigurationsdatei unter %1 enthalten als Wurzelelement kein Objekt.</translation>
    </message>
    <message id="FUNCHOTORN_CLI_OPT_TEST_EMAIL">
        <source>Send a test email.</source>
        <extracomment>Option description in the cli help</extracomment>
        <translation>Versende eine Test-E-Mail.</translation>
    </message>
    <message id="FUNCHOTORN_TEST_MAIL_SUBJECT">
        <source>Funchotorn Test Mail</source>
        <extracomment>test email subject</extracomment>
        <translation>Test-E-Mail von Funchotorn</translation>
    </message>
    <message id="FUNCHOTORN_TEST_MAIL_BODY">
        <source>This is a test email to check if your email settings are working.</source>
        <extracomment>test email body</extracomment>
        <translation>Dies ist eine Test-E-Mail zum Überprüfen Ihrer E-Mail-Einstellungen.</translation>
    </message>
    <message id="FUNCHOTORN_CLI_ERR_CMD_NOT_FOUND">
        <source>Can not find %1 executable</source>
        <extracomment>CLI error message, %1 will be replaced by the executable name</extracomment>
        <translation>Kann keine ausführbare Programmdatei für %1 finden</translation>
    </message>
    <message id="FUNCHOTORN_MAIL_ERROR_BODY_INTRO">
        <source>While trying to update the MLS database, the following error occured:</source>
        <extracomment>error notification email body intro text</extracomment>
        <translation>Beim Versuch die MLS-Datenbank zu aktualisieren ist der folgende Fehler aufgetreten:</translation>
    </message>
    <message id="FUNCHOTORN_MAIL_ERROR_SUBJECT">
        <source>Error while updating MLS database</source>
        <extracomment>error notification email subject</extracomment>
        <translation>Fehler beim Aktualisieren der MLS-Datenbank</translation>
    </message>
    <message id="FUNCHOTORN_MAIL_BODY_SIGNATURE">
        <source>This message was generated automatically by %1 %2 on %3.</source>
        <oldsource>This message was generated automatically by %1 %2 on %3</oldsource>
        <extracomment>email body signature, %1 will be replaced by the application name, %2 by the app version and %3 by the local host name</extracomment>
        <translation>Diese Nachricht wurde automatisch von %1 %2 auf %3 erstellt.</translation>
    </message>
    <message id="FUNCHOTORN_CLI_OPT_JOURNALD">
        <source>Write all output to systemd’s journal instead of stdout.</source>
        <extracomment>Option description in the cli help</extracomment>
        <translation>Schreibe alle Ausgaben in systemds journal an Stelle von stdout.</translation>
    </message>
</context>
</TS>