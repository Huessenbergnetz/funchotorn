# SPDX-FileCopyrightText: (C) 2021 Matthias Fehring / www.huessenbergnetz.de
# SPDX-License-Identifier: AGPL-3.0-or-later

cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)
cmake_policy(SET CMP0048 NEW)

project(funchotorn
        VERSION 0.1.0
        DESCRIPTION "Downloader and converter for Mozilla Location Service databases"
        LANGUAGES CXX)

find_package(Qt5 5.6.0 REQUIRED COMPONENTS Core Network)
find_package(SimpleMail2Qt5 2.1.0 REQUIRED)
find_package(PkgConfig)

if(PkgConfig_FOUND)
    message(STATUS "Found pkgconfig")
    pkg_check_modules(SYSTEMD QUIET libsystemd)
endif(PkgConfig_FOUND)

# Auto generate moc files
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Forbid in-tree building
if(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
      message(STATUS "Please do an out-of-tree build:")
      message(STATUS "rm -f CMakeCache.txt && mkdir build && cd build; cmake .. && make")
      message(FATAL_ERROR "In-tree-build detected!")
endif(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})

option(ENABLE_MAINTAINER_CFLAGS "Enable maintainer CFlags" OFF)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "funchotorn default install prefix" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

include(GNUInstallDirs)

set(FUNCHOTORN_USER "funchotorn" CACHE STRING "System user for funchotorn")
set(FUNCHOTORN_GROUP "funchotorn" CACHE STRING "System group for funchotorn")
set(FUNCHOTORN_DATADIR "/srv/www/funchotorn" CACHE PATH "Directory to serve files from")
set(SYSTEMD_UNITDIR "${CMAKE_INSTALL_PREFIX}/lib/systemd/system" CACHE PATH "Systemd unit directory")

add_subdirectory(supplementary)
add_subdirectory(cmd)
add_subdirectory(translations)
