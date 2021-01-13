# SPDX-FileCopyrightText: (C) 2021 Matthias Fehring / www.huessenbergnetz.de
# SPDX-License-Identifier: AGPL-3.0-or-later

find_program(LRELEASE_CMD_PATH NAMES lrelease-qt5 lrelease)
set(LRELEASE_CMD ${LRELEASE_CMD_PATH})

if (LRELEASE_CMD)
    message(STATUS "Found lrelease at ${LRELEASE_CMD}")
    message(STATUS "Translations are enabled")

    execute_process(COMMAND ${LRELEASE_CMD} -idbased -silent ${CMAKE_CURRENT_SOURCE_DIR}/funchotorn.ts -qm ${CMAKE_CURRENT_BINARY_DIR}/funchotorn.qm)
    list(APPEND QM_FILES ${CMAKE_CURRENT_BINARY_DIR}/funchotorn.qm)

    set(LANGS de)

    foreach(l ${LANGS})
        execute_process(COMMAND ${LRELEASE_CMD} -idbased -silent ${CMAKE_CURRENT_SOURCE_DIR}/funchotorn_${l}.ts -qm ${CMAKE_CURRENT_BINARY_DIR}/funchotorn_${l}.qm)
        list(APPEND QM_FILES ${CMAKE_CURRENT_BINARY_DIR}/funchotorn_${l}.qm)
    endforeach(l ${LANGS})

    install (FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_LOCALEDIR})
endif (LRELEASE_CMD)
