# SPDX-FileCopyrightText: (C) 2021 Matthias Fehring / www.huessenbergnetz.de
# SPDX-License-Identifier: AGPL-3.0-or-later

add_custom_target(funchotornsupplementary SOURCES
    config.json
    funchotorn-updater.service.in
    funchotorn-updater.timer
)

install(FILES ${CMAKE_SOURCE_DIR}/supplementary/config.json
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ
        RENAME ${PROJECT_NAME}.json
)

if(SYSTEMD_FOUND)
    configure_file(funchotorn-updater.service.in ${CMAKE_CURRENT_BINARY_DIR}/funchotorn-updater.service @ONLY)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/funchotorn-updater.service
        funchotorn-updater.timer
        DESTINATION ${SYSTEMD_UNITDIR}
    )
endif(SYSTEMD_FOUND)
