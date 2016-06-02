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
 * @file        NotificationBackend.h
 * @author      Zofia Abramowska <z.abramowska@samsung.com>
 * @brief       This file contains notification backend declaration.
 */

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>

#include <main/Request.h>
#include <main/NotificationTalker.h>
#include <ui/AskUIInterface.h>

#include <types/NotificationResponse.h>

namespace AskUser {

namespace Agent {

class NotificationBackend : public AskUIInterface {
public:
    NotificationBackend();
    virtual ~NotificationBackend();

    virtual bool start(const std::string &client, const std::string &user,
                       const std::string &privilege, RequestId requestId, UIResponseCallback);
    virtual bool setOutdated();
    virtual bool dismiss();
    virtual bool isDismissing() const;

private:
    static void responseCb(NotificationResponse response);
    static NotificationTalker m_notiTalker;
    static std::map<RequestId, NotificationBackend *> m_idToInstance;
    static std::mutex m_instanceGuard;

    RequestId m_id;
    UIResponseCallback m_cb;
    bool m_isDismissing;
};

    typedef std::unique_ptr<NotificationBackend> NotificationBackendPtr;
} // namespace Agent

} // namespace AskUser
