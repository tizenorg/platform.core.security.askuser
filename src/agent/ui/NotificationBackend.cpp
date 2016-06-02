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
 * @file        NotificationBackend.cpp
 * @author      Zofia Abramowska <z.abramowska@samsung.com>
 * @brief       This file contains notification backend definition.
 */

#include <stdexcept>
#include <log/alog.h>

#include "NotificationBackend.h"

namespace AskUser {

namespace Agent {

NotificationTalker NotificationBackend::m_notiTalker;
std::map<RequestId, NotificationBackend *> NotificationBackend::m_idToInstance;
std::mutex NotificationBackend::m_instanceGuard;

NotificationBackend::NotificationBackend() : m_id(-1), m_isDismissing(false)
{
    if (m_notiTalker.isFailed()) {
        ALOGE("NotificationTalker failed beacause: " << m_notiTalker.getErrorMsg());
        throw std::runtime_error("Backend failed");
    }
}

bool NotificationBackend::start(const std::string &client, const std::string &user,
                                const std::string &privilege, RequestId requestId,
                                UIResponseCallback callback)
{
    m_idToInstance[requestId] = this;
    m_id = requestId;
    m_cb = callback;

    if (m_notiTalker.isFailed()) {
        ALOGE("NotificationTalker failed beacause: " << m_notiTalker.getErrorMsg());
        throw std::runtime_error("Backend failed");
    }

    m_notiTalker.setResponseHandler(&NotificationBackend::responseCb);
    m_notiTalker.parseRequest(RequestType::RT_Action,
                              NotificationRequest(requestId, client, user, privilege));
    return true;
}

void NotificationBackend::responseCb(NotificationResponse response)
{
    std::lock_guard<std::mutex> lock(m_instanceGuard);
    auto it = m_idToInstance.find(response.id);
    if (it == m_idToInstance.end()) {
        ALOGW("Instance for this request does not exist anymore");
        return;
    }
    UIResponseType type;
    switch(response.response) {
    case NResponseType::Allow:
        type = UIResponseType::URT_YES_LIFE;
        break;
    case NResponseType::Deny:
        type = UIResponseType::URT_NO_ONCE;
        break;
    case NResponseType::Never:
        type = UIResponseType::URT_NO_LIFE;
        break;
    case NResponseType::Error:
        type = UIResponseType::URT_ERROR;
        break;
    case NResponseType::None:
        type = UIResponseType::URT_TIMEOUT;
        break;
    }
    it->second->m_cb(response.id, type);
}

bool NotificationBackend::setOutdated()
{
    return true;
}

bool NotificationBackend::dismiss()
{
    m_notiTalker.parseRequest(RequestType::RT_Cancel, NotificationRequest(m_id));
    return true;
}

bool NotificationBackend::isDismissing() const
{
    return m_isDismissing;
}

NotificationBackend::~NotificationBackend()
{
    std::lock_guard<std::mutex> lock(m_instanceGuard);
    m_idToInstance.erase(m_id);
}

} //namespace Agent

} //namespace AskUser
