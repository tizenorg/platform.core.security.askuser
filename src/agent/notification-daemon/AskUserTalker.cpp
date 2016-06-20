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

#include <socket/Socket.h>
#include <socket/SelectRead.h>
#include <types/NotificationResponse.h>
#include <types/Protocol.h>
#include <types/NotificationRequest.h>
#include <exception/ErrnoException.h>
#include <exception/Exception.h>
#include <translator/Translator.h>
#include <config/Path.h>
#include <config/Limits.h>

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

void setSecurityLevel(const std::string &app, const std::string &perm, const std::string &level)
{
    int ret;

    policy_update_req *policyUpdateRequest = nullptr;
    policy_entry *policyEntry = nullptr;

    try {
        if (level != "Allow" && level != "Deny")
            throw std::invalid_argument("Not allowed security level <" + level + ">");

        ALOGD("SecurityManager: Setting security level to " << level);

        ret = security_manager_policy_update_req_new(&policyUpdateRequest);
        throwOnSecurityPrivilegeError("security_manager_policy_update_req_new", ret);

        ret = security_manager_policy_entry_new(&policyEntry);
        throwOnSecurityPrivilegeError("security_manager_policy_entry_new", ret);

        ret = security_manager_policy_entry_set_application(policyEntry,
                                                        dropPrefix(app.c_str()));
        throwOnSecurityPrivilegeError("security_manager_policy_entry_set_application", ret);

        ret = security_manager_policy_entry_set_privilege(policyEntry, perm.c_str());
        throwOnSecurityPrivilegeError("security_manager_policy_entry_set_privilege", ret);

        ret = security_manager_policy_entry_set_level(policyEntry, level.c_str());
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
    try {
        Socket::close(sockfd);
    } catch (const std::exception &e) {
        ALOGE(std::string("~AskUserTalker") + e.what());
    } catch (...) {
        ALOGE("~AskUserTalker: Unknow error");
    }
}

void AskUserTalker::run()
{
    sockfd = Socket::connect(Path::getSocketPath());

    while (!stopFlag) {
        size_t size;
        char *buf;
        NotificationResponse response;

        ALOGD("Waiting for request...");

        if (!Socket::recv(sockfd, &size, sizeof(size))) {
            ALOGI("Askuserd closed connection, closing...");
            break;
        }

        Limits::checkSizeLimit(size);

        buf = new char[size];

        if (!Socket::recv(sockfd, buf, size)) {
            ALOGI("Askuserd closed connection, closing...");
            break;
        }

        NotificationRequest request = Translator::Gui::dataToNotificationRequest(buf);
        delete[] buf;
        ALOGD("Recieved data " << request.data.client << " " << request.data.privilege);

        response.response = m_gui->popupRun(request.data.client, request.data.privilege);
        response.id = request.id;

        if (response.response == NResponseType::None) {
            continue;
        }

        if (!Socket::send(sockfd, &response, sizeof(response))) {
            ALOGI("Askuserd closed connection, closing...");
            break;
        }

        uint8_t ack = 0x00;
        if (!Socket::recv(sockfd, &ack, sizeof(ack))) {
            ALOGI("Askuserd closed connection, closing...");
            break;
        }

        if (ack != Protocol::ackCode)
            throw Exception("Incorrect ack");

        switch (response.response) {
        case NResponseType::Error:
            throw Exception(m_gui->getErrorMsg());
        case NResponseType::Allow:
        case NResponseType::Never:
            setSecurityLevel(request.data.client, request.data.privilege,
                             Translator::Gui::responseToString(response.response));
        default:
            break;
        }
    }
}

void AskUserTalker::stop()
{
    m_gui->stop();
    Socket::close(sockfd);
}

bool AskUserTalker::shouldDismiss()
{
    Socket::SelectRead select;
    select.add(sockfd);
    if (select.exec() == 0)
        return false;

    uint8_t a = 0x00;
    Socket::recv(sockfd, &a, sizeof(a));

    if (a != Protocol::dissmisCode)
        throw Exception("Incorrect dismiss flag");

    return true;
}

} /* namespace Notification */

} /* namespace AskUser */
