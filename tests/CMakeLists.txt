SET(TESTS_PATH ${PROJECT_SOURCE_DIR}/tests/)

FIND_PACKAGE(Threads REQUIRED) # Required by gmock

PKG_CHECK_MODULES(TESTS_DEP
    QUIET gmock
    cynara-client
    cynara-creds-socket
    cynara-admin
    libsystemd-daemon
)

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/common
    ${PROJECT_SOURCE_DIR}/src/agent
    ${PROJECT_SOURCE_DIR}/src/agent/main
    ${gmock_INCLUDE_DIRS}
)

SET(TESTS_SOURCES
    ${TESTS_PATH}/main.cpp
    ${TESTS_PATH}/common/exception.cpp
    ${TESTS_PATH}/common/translator.cpp
    ${TESTS_PATH}/daemon/notificationTalker.cpp

    ${PROJECT_SOURCE_DIR}/src/common/config/Path.cpp
    ${PROJECT_SOURCE_DIR}/src/common/log/alog.cpp
    ${PROJECT_SOURCE_DIR}/src/common/socket/Socket.cpp
    ${PROJECT_SOURCE_DIR}/src/common/socket/SelectRead.cpp
    ${PROJECT_SOURCE_DIR}/src/common/translator/Translator.cpp
    ${PROJECT_SOURCE_DIR}/src/common/types/AgentErrorMsg.cpp
    ${PROJECT_SOURCE_DIR}/src/agent/main/NotificationTalker.cpp
   )

ADD_DEFINITIONS(${TESTS_DEP_CFLAGS})

ADD_EXECUTABLE(${TARGET_TESTS} ${TESTS_SOURCES})

SET_TARGET_PROPERTIES(${TARGET_TESTS} PROPERTIES
    COMPILE_FLAGS
    -fpie
)

TARGET_LINK_LIBRARIES(${TARGET_TESTS}
    ${TESTS_DEP_LIBRARIES}
        ${gmock_LDFLAGS}
        ${gmock_LIBRARIES}
    -pie
)

INSTALL(TARGETS ${TARGET_TESTS} DESTINATION ${BIN_INSTALL_DIR})
