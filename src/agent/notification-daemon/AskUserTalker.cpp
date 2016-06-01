/*
 *  Copyright (c) 2016 Samsung Electronics Co.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License
 */
/**
 * @file        src/notification-daemon/GuiRunner.h
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Definition of AskUserTalker class
 */

#include "AskUserTalker.h"

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <types/NotificationResponse.h>
#include <types/Protocol.h>
#include <types/NotificationRequest.h>
#include <exception/ErrnoException.h>
#include <exception/Exception.h>
#include <translator/Translator.h>
#include <config/Path.h>

#include <security-manager.h>

namespace AskUser {

namespace Notification {

namespace {

inline void throwOnSecurityPrivilegeError(std::string err, int ret)
{
    if (ret != SECURITY_MANAGER_SUCCESS)
        throw Exception(err + " : " + std::to_string(ret));
}

inline const char *dropPrefix(const char* app)
{
    constexpr char prefix[] = "User::App::";
    constexpr size_t prefixSize = sizeof(prefix) - 1;
    return strncmp(app, prefix, prefixSize) ? app : app + prefixSize;
}

void setSecurityLevel(const std::string &app, const std::string &perm, NResponseType response)
{
    int ret;
    std::string responseStr = Translator::Gui::responseToString(response);
    ALOGD("SecurityManager: Setting security level to " << responseStr);

    policy_update_req *policyUpdateRequest = nullptr;
    policy_entry *policyEntry = nullptr;

    try {
        ret = security_manager_policy_update_req_new(&policyUpdateRequest);
        throwOnSecurityPrivilegeError("security_manager_policy_update_req_new", ret);

        ret = security_manager_policy_entry_new(&policyEntry);
        throwOnSecurityPrivilegeError("security_manager_policy_entry_new", ret);

        ret = security_manager_policy_entry_set_application(policyEntry,
                                                        dropPrefix(app.c_str()));
        throwOnSecurityPrivilegeError("security_manager_policy_entry_set_application", ret);

        ret = security_manager_policy_entry_set_privilege(policyEntry, perm.c_str());
        throwOnSecurityPrivilegeError("security_manager_policy_entry_set_privilege", ret);

        ret = security_manager_policy_entry_set_level(policyEntry, responseStr.c_str());
        throwOnSecurityPrivilegeError("security_manager_policy_entry_admin_set_level", ret);

        ret = security_manager_policy_update_req_add_entry(policyUpdateRequest, policyEntry);
        throwOnSecurityPrivilegeError("security_manager_policy_update_req_add_entry", ret);

        ret = security_manager_policy_update_send(policyUpdateRequest);
        throwOnSecurityPrivilegeError("security_manager_policy_update_send", ret);

        ALOGD("SecurityManager: Setting level succeeded");
    } catch (std::exception &e) {
        ALOGE("SecurityManager: Failed <" << e.what() << ">");
    }

    security_manager_policy_entry_free(policyEntry);
    security_manager_policy_update_req_free(policyUpdateRequest);
}

} /* namespace */


AskUserTalker::AskUserTalker(GuiRunner *gui) : m_gui(gui) {
    m_gui->setDropHandler([&](){return this->shouldDismiss();});
}

AskUserTalker::~AskUserTalker()
{
    close(sockfd);
}

void AskUserTalker::run()
{
    int ret;
    struct sockaddr_un remote;

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1)
        throw ErrnoException("Creating socket failed");

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, Path::getSocketPath().c_str());

    int len = strlen(remote.sun_path) + sizeof(remote.sun_family);

    ret = connect(sockfd, (struct sockaddr *)&remote, len);
    if (ret == -1)
        throw ErrnoException("Connecting to socket failed");

    while (!stopFlag) {
        size_t size;
        int len;
        char *buf;
        NotificationResponse response;

        len = recv(sockfd, &size, sizeof(size), 0);
        if (len < 0) {
            throw ErrnoException("Recieving data from socket error");
        } else if (len == 0) {
              ALOGI("Askuserd closed connection, closing...");
              break;
        }

        buf = new char[size];

        len = recv(sockfd, buf, size, 0);
        if (len < 0) {
            throw ErrnoException("Recieving data from socket error");
        } else if (len == 0) {
              ALOGI("Askuserd closed connection, closing...");
              break;
        }

        NotificationRequest request = Translator::Gui::dataToNotificationRequest(buf);
        delete[] buf;
        ALOGD("Recieved data " << request.data.client << " " << request.data.privilege);

        response.response = m_gui->popupRun(request.data.client, request.data.privilege);
        response.id = request.id;

        if (response.response == NResponseType::None)
            continue;

        len = send(sockfd, &response, sizeof(response), 0);
        if (len < 0)
            throw ErrnoException("Sending data to socket error");

        uint8_t ack = 0x00;
        len = recv(sockfd, &ack, sizeof(ack), 0);

        if (ack != Protocol::ackCode)
            throw Exception("Incorrect ack");

        switch (response.response) {
        case NResponseType::Allow:
        case NResponseType::Never:
            setSecurityLevel(request.data.client, request.data.privilege, response.response);
        default:
            break;
        }
    }
}

void AskUserTalker::stop()
{
      m_gui->stop();
      close(sockfd);
}

bool AskUserTalker::shouldDismiss()
{
      struct timeval time;
      time.tv_usec = 0;
      time.tv_sec = 0;
      fd_set set;
      FD_ZERO(&set);
      FD_SET(sockfd, &set);

      int ret = select(sockfd + 1, &set, nullptr, nullptr, &time);
      if (ret == 0)
          return false;

      uint8_t a = 0x00;
      recv(sockfd, &a, sizeof(a), 0);

      if (a != Protocol::dissmisCode)
          throw Exception("Incorrect dismiss flag");

      return true;
}

} /* namespace Notification */

} /* namespace AskUser */
