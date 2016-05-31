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
# @file        src/agent/main/CMakeLists.txt
# @author      Adam Malinowski <a.malinowsk2@partner.samsung.com>
#

SET(DAEMON_PATH ${PROJECT_SOURCE_DIR}/src/agent/main/)

PKG_CHECK_MODULES(ASKUSERD_DEP
    REQUIRED
    cynara-agent
    cynara-creds-socket
    libsystemd-daemon
    libsystemd-journal
)

INCLUDE_DIRECTORIES(
    ${ASKUSER_PATH}
    ${ASKUSER_PATH}/common/
)

SET(ASKUSERD_SOURCES
    ${DAEMON_PATH}/CynaraTalker.cpp
    ${DAEMON_PATH}/main.cpp
    ${DAEMON_PATH}/NotificationTalker.cpp
   )

ADD_DEFINITIONS(${ASKUSERD_DEP_CFLAGS})

ADD_EXECUTABLE(${TARGET_ASKUSERD} ${ASKUSERD_SOURCES})

SET_TARGET_PROPERTIES(${TARGET_ASKUSERD} PROPERTIES
    COMPILE_FLAGS
    -fpie
)

TARGET_LINK_LIBRARIES(${TARGET_ASKUSERD}
    ${TARGET_ASKUSER_COMMON}
    ${ASKUSERD_DEP_LIBRARIES}
    -pie
)

INSTALL(TARGETS ${TARGET_ASKUSERD} DESTINATION ${BIN_INSTALL_DIR})
