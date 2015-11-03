/*
 *  Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file        AskUINotificationBackend.cpp
 * @author      Adam Malinowski <a.malinowsk2@partner.samsung.com>
 * @brief       This file implements class for ask user window
 */

#include <bundle.h>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libintl.h>
#include <privilegemgr/privilege_info.h>

#include <attributes/attributes.h>

#include <log/alog.h>

#include "AskUINotificationBackend.h"
#include "popup-runner.h"

namespace AskUser {

namespace Agent {

AskUINotificationBackend::AskUINotificationBackend() :
    m_dismissing(false)
{}

AskUINotificationBackend::~AskUINotificationBackend(){
}

bool AskUINotificationBackend::start(const std::string &client, const std::string &user,
                                     const std::string &privilege, RequestId requestId,
                                     UIResponseCallback responseCallback) {
    if (!responseCallback) {
        ALOGE("Empty response callback is not allowed");
        return false;
    }

    char *privilegeDisplayName;
    int ret = privilege_info_get_privilege_display_name(privilege.c_str(), &privilegeDisplayName);
    if (ret != PRVMGR_ERR_NONE) {
        ALOGE("Unable to get privilege display name, err: [" << ret << "]");
        privilegeDisplayName = strdup(privilege.c_str());
    }

    ALOGD("privilege_info_get_privilege_display_name: [" << ret << "]," " <" << privilegeDisplayName << ">");

    UIResponseType response = run_popup(client, user, privilegeDisplayName, m_responseTimeout);
    free(privilegeDisplayName);
    if (UIResponseType::URT_ERROR == response)
        return false;

    responseCallback(requestId, response);
    return true;
}

bool AskUINotificationBackend::setOutdated() {
    // There is no possibility to update window
    return true;
}

bool AskUINotificationBackend::dismiss() {
    // TODO: Implement
    return false;
}

} // namespace Agent

} // namespace AskUser
