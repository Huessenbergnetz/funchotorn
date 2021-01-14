# SPDX-FileCopyrightText: (C) 2021 Matthias Fehring / www.huessenbergnetz.de
# SPDX-License-Identifier: AGPL-3.0-or-later

add_executable(funchotorn
    main.cpp
    updater.cpp
    updater.h
)

target_compile_features(funchotorn PRIVATE cxx_std_14)

target_link_libraries(funchotorn
    PRIVATE
        Qt5::Core
        Qt5::Network
)

target_compile_definitions(funchotorn
    PRIVATE
        QT_NO_CAST_TO_ASCII
        QT_NO_CAST_FROM_ASCII
        QT_STRICT_ITERATORS
        QT_NO_URL_CAST_FROM_STRING
        QT_NO_CAST_FROM_BYTEARRAY
        QT_USE_QSTRINGBUILDER
        QT_USE_FAST_OPERATOR_PLUS
        QT_DISABLE_DEPRECATED_BEFORE=0x050500
        FUNCHOTORN_VERSION="${PROJECT_VERSION}"
        FUNCHOTORN_CONFIGFILE="${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME}.ini"
        FUNCHOTORN_CACHEDIR="${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/cache/${PROJECT_NAME}"
        FUNCHOTORN_DATADIR="${FUNCHOTORN_DATADIR}"
        $<BUILD_INTERFACE:FUNCHOTORN_TRANSDIR="${CMAKE_BINARY_DIR}/translations">
        $<INSTALL_INTERFACE:FUNCHOTORN_TRANSDIR="${CMAKE_INSTALL_LOCALEDIR}">
)

if(ENABLE_MAINTAINER_FLAGS)
    target_compile_definitions(qhrobot
        PRIVATE
            QT_DEPRECATED_WARNINGS
    )
    target_compile_options(qhrobot
        PRIVATE
            -Wall
            -Wcast-align
            -Wno-uninitialized
            -Wempty-body
            -Wformat-security
            -Wformat
            -Winit-self
    )
endif(ENABLE_MAINTAINER_FLAGS)

install(TARGETS funchotorn DESTINATION ${CMAKE_INSTALL_BINDIR})