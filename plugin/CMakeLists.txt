# Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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
# @author      Zofia Abramowska <z.abramowska@samsung.com>
#

SET(PLUGIN_PATH ${CYNARA_PATH}/plugin)

INCLUDE_DIRECTORIES(
    ${PLUGIN_PATH}
    ${PLUGIN_PATH}/service
    ${CYNARA_PATH}/include
    )

SET(SERVICE_PLUGIN_SOURCES
    ${PLUGIN_PATH}/service/ServicePlugin.cpp
    )

SET(CLIENT_PLUGIN_SOURCES
    ${PLUGIN_PATH}/client/ClientPlugin.cpp
    )

ADD_DEFINITIONS("-fvisibility=default")

ADD_LIBRARY(${TARGET_SERVICE_PLUGIN} SHARED ${SERVICE_PLUGIN_SOURCES})
ADD_LIBRARY(${TARGET_CLIENT_PLUGIN} SHARED ${CLIENT_PLUGIN_SOURCES})

TARGET_LINK_LIBRARIES(${TARGET_SERVICE_PLUGIN}
    ${TARGET_CYNARA_COMMON}
    )
TARGET_LINK_LIBRARIES(${TARGET_CLIENT_PLUGIN}
    ${TARGET_CYNARA_COMMON}
    )

INSTALL(TARGETS ${TARGET_SERVICE_PLUGIN}
        DESTINATION ${LIB_INSTALL_DIR}/cynara/plugin/service/)
INSTALL(TARGETS ${TARGET_CLIENT_PLUGIN}
    DESTINATION ${LIB_INSTALL_DIR}/cynara/plugin/client/)
