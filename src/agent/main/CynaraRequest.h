
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
 * @file        src/common/types/CynaraRequest.h
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @author      Adam Malinowski <a.malinowsk2@partner.samsung.com>
 * @brief       This file declares class representing request from cynara service
 */

#pragma once

#include <string>

#include <cynara-agent.h>

namespace AskUser {

namespace Agent {

typedef enum {
    RT_Action,
    RT_Cancel,
    RT_Close,
    RT_Ignore
} RequestType;

typedef cynara_agent_req_id RequestId;

class CynaraRequest {
public:
    CynaraRequest() = default;
    CynaraRequest(RequestType type, RequestId id = RequestId(0),
                  std::string user = "",
                  std::string client = "",
                  std::string privilege = "")
        : m_type(type),
          m_id(id),
          m_perm(perm),
          m_client(client),
          m_user(user) 
    {}

    RequestType type() const {
        return m_type;
    }

    RequestId id() const {
        return m_id;
    }

    std::string privilege() {
        return m_privilege;
    }

    std::string client() {
        return m_client;
    }

    std::string user() {
        return m_user;
    }

private:
    RequestType m_type;
    RequestId m_id;
    std::string m_perm;
    std::string m_client;
    std::string m_user;
};

} //namespace AskUser
