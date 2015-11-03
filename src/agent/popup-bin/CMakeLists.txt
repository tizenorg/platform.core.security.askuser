# Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
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
#
# @file     CMakeLists.txt
# @author   Janusz Kozerski (j.kozerski@samsung.com)
#
# @version  1.0
#

PKG_CHECK_MODULES(ASKUSER_POPUP_DEP
    elementary
    libsystemd-daemon
    REQUIRED)

set(ASKUSER_AGENT_POPUP_SRC_PATH ${ASKUSER_AGENT_PATH}/popup-bin)

set(ASKUSER_POPUP_SOURCES
    ${ASKUSER_AGENT_POPUP_SRC_PATH}/popup.cpp
    ${ASKUSER_AGENT_PATH}/ui/popup-runner.cpp
    # logs
    ${ASKUSER_AGENT_PATH}/log/alog.cpp
    # dpl
    ${ASKUSER_AGENT_PATH}/ui/serialization.cpp
)

ADD_DEFINITIONS(${ASKUSER_POPUP_DEP_CFLAGS})

INCLUDE_DIRECTORIES(
    ${ASKUSER_AGENT_PATH}
    ${ASKUSER_AGENT_PATH}/ui/popup-bin/
    ${ASKUSER_AGENT_PATH}/ui/
)

ADD_EXECUTABLE(${TARGET_ASKUSER_POPUP}
    ${ASKUSER_POPUP_SOURCES}
)

SET_TARGET_PROPERTIES(${TARGET_ASKUSER_POPUP} PROPERTIES
    COMPILE_FLAGS
    -fpie
)

TARGET_LINK_LIBRARIES(${TARGET_ASKUSER_POPUP}
    ${ASKUSER_POPUP_DEP_LIBRARIES}
    -pie
)

INSTALL(TARGETS ${TARGET_ASKUSER_POPUP} DESTINATION ${BIN_INSTALL_DIR})
