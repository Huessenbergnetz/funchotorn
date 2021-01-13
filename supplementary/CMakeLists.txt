# SPDX-FileCopyrightText: (C) 2021 Matthias Fehring / www.huessenbergnetz.de
# SPDX-License-Identifier: AGPL-3.0-or-later

add_custom_target(funchotornsupplementary SOURCES
    config.in
)

configure_file(config.in ${CMAKE_BINARY_DIR}/supplementary/${PROJECT_NAME}.ini @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/supplementary/${PROJECT_NAME}.ini
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}
        PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ)
