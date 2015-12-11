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

PKG_CHECK_MODULES(AGENT_DEP
    REQUIRED
    cynara-agent
    cynara-plugin
    cynara-creds-socket
    )

PKG_CHECK_MODULES(ELM_DEP
    QUIET
    elementary
    )

PKG_CHECK_MODULES(LIBNOTIFY_DEP
    QUIET
    libnotify
    dbus-1
    dbus-glib-1
    glib-2.0
    )

SET(ASKUSER_AGENT_PATH ${ASKUSER_PATH}/agent)

SET(ASKUSER_SOURCES
    ${ASKUSER_AGENT_PATH}/main/Agent.cpp
    ${ASKUSER_AGENT_PATH}/main/CynaraTalker.cpp
    ${ASKUSER_AGENT_PATH}/main/main.cpp
    )

IF (ELM_DEP_FOUND)
SET(ASKUSER_SOURCES ${ASKUSER_SOURCES}
    ${ASKUSER_AGENT_PATH}/main/NotificationTalker.cpp
    ${ASKUSER_AGENT_PATH}/ui/NotificationBackend.cpp
    )
ADD_DEFINITIONS("-DBUILD_WITH_NOTIFICATION")
ENDIF (ELM_DEP_FOUND)

IF (LIBNOTIFY_DEP_FOUND)
SET(ASKUSER_SOURCES ${ASKUSER_SOURCES}
    ${ASKUSER_AGENT_PATH}/ui/AskUILibNotifyBackend.cpp
    )
ADD_DEFINITIONS("-DBUILD_WITH_LIBNOTIFY")
ENDIF (LIBNOTIFY_DEP_FOUND)

INCLUDE_DIRECTORIES(
    ${ASKUSER_PATH}/common
    ${AGENT_DEP_INCLUDE_DIRS}
    ${SYSTEMD_DEP_INCLUDE_DIRS}
    ${NOTIFICATION_DEP_INCLUDE_DIRS}
    ${LIBNOTIFY_DEP_INCLUDE_DIRS}
    ${ASKUSER_AGENT_PATH}
    )

ADD_DEFINITIONS("-fPIE")
ADD_EXECUTABLE(${TARGET_ASKUSER} ${ASKUSER_SOURCES})

TARGET_LINK_LIBRARIES(${TARGET_ASKUSER}
    ${AGENT_DEP_LIBRARIES}
    ${ASKUSER_DEP_LIBRARIES}
    ${SYSTEMD_DEP_LIBRARIES}
    ${NOTIFICATION_DEP_LIBRARIES}
    ${LIBNOTIFY_DEP_LIBRARIES}
    ${TARGET_ASKUSER_COMMON}
    "-pie"
    )

INSTALL(TARGETS ${TARGET_ASKUSER} DESTINATION ${BIN_DIR})

IF (ELM_DEP_FOUND)
ADD_SUBDIRECTORY(notification-daemon)
ENDIF (ELM_DEP_FOUND)
