# Copyright (c) 2014-2016 Samsung Electronics Co., Ltd All Rights Reserved
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#
# @file        CMakeLists.txt
# @author      Adam Malinowski <a.malinowsk2@partner.samsung.com>
#
PKG_CHECK_MODULES(COMMON_DEP
    REQUIRED
    cynara-plugin
    cynara-agent
    libsystemd-journal
    )

SET(ASKUSER_COMMON_VERSION_MAJOR 0)
SET(ASKUSER_COMMON_VERSION ${ASKUSER_COMMON_VERSION_MAJOR}.1.0)

INCLUDE_DIRECTORIES(SYSTEM
    ${COMMON_DEP_INCLUDE_DIRS}
    )

SET(COMMON_PATH ${ASKUSER_PATH}/common)

INCLUDE_DIRECTORIES(
    ${COMMON_PATH}
    )

SET(COMMON_SOURCES
    ${COMMON_PATH}/log/alog.cpp
    ${COMMON_PATH}/socket/Socket.cpp
    ${COMMON_PATH}/socket/SelectRead.cpp
    ${COMMON_PATH}/translator/Translator.cpp
    ${COMMON_PATH}/types/AgentErrorMsg.cpp
    ${COMMON_PATH}/util/SafeFunction.cpp
    ${COMMON_PATH}/config/Limits.cpp
    ${COMMON_PATH}/config/Path.cpp
    )

ADD_DEFINITIONS("-fvisibility=default")

ADD_LIBRARY(${TARGET_ASKUSER_COMMON} SHARED ${COMMON_SOURCES})

SET_TARGET_PROPERTIES(
    ${TARGET_ASKUSER_COMMON}
    PROPERTIES
        SOVERSION ${ASKUSER_COMMON_VERSION_MAJOR}
        VERSION ${ASKUSER_COMMON_VERSION}
    )

TARGET_LINK_LIBRARIES(${TARGET_ASKUSER_COMMON}
    ${COMMON_DEP_LIBRARIES}
    )

INSTALL(TARGETS ${TARGET_ASKUSER_COMMON} DESTINATION ${LIB_INSTALL_DIR})
