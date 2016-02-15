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
 * @file        AskUIPopupBackend.cpp
 * @author      Adam Malinowski <a.malinowsk2@partner.samsung.com>
 * @author      Janusz Kozerski <j.kozerski@samsung.com>
 * @brief       This file implements class for ask user window
 */

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libintl.h>
#include <privilegemgr/privilege_info.h>

#include <attributes/attributes.h>

#include <log/alog.h>
#include <ui/AskUIPopupBackend.h>

namespace AskUser {

namespace Agent {

AskUIPopupBackend::AskUIPopupBackend() :
    m_dismissing(false)
{}

AskUIPopupBackend::~AskUIPopupBackend(){
    if (!m_dismissing)
        dismiss();
}

bool AskUIPopupBackend::start(const std::string &client, const std::string &user,
                                     const std::string &privilege, RequestId requestId,
                                     UIResponseCallback responseCallback) {
    if (!responseCallback) {
        ALOGE("Empty response callback is not allowed");
        return false;
    }
    m_responseCallback = responseCallback;
    m_requestId = requestId;

    char *privilegeDisplayName = NULL;
    int ret = privilege_info_get_privilege_display_name(privilege.c_str(), &privilegeDisplayName);
    if (ret != PRVMGR_ERR_NONE) {
        ALOGE("Unable to get privilege display name, err: [" << ret << "]");
        privilegeDisplayName = strdup(privilege.c_str());
    }
    ALOGD("privilege_info_get_privilege_display_name: [" << ret << "]," " <" << privilegeDisplayName << ">");

    if (!m_popup.run_popup(client, user, std::string(privilegeDisplayName), m_responseTimeout)) {
        free(privilegeDisplayName);
        return false;
    }
    free(privilegeDisplayName);
    m_thread = std::thread(&AskUIPopupBackend::run, this);
    return true;
}

void AskUIPopupBackend::run() {
    UIResponseType response = m_popup.wait_for_response();
    m_responseCallback(m_requestId, response);
}

bool AskUIPopupBackend::setOutdated() {
    // There is no possibility to update window
    return true;
}

bool AskUIPopupBackend::dismiss() {
    m_dismissing = true;
    m_popup.dissmiss();
    m_thread.join();
    return true;
}

} // namespace Agent

} // namespace AskUser
