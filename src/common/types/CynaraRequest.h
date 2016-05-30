
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
 * @brief       Definition of common types and consts
 */

#pragma once

#include <memory>
#include <string>

#include <cynara-agent.h>

namespace AskUser {

typedef enum {
    RT_Action,
    RT_Cancel,
    RT_Close,
    RT_Ignore
} RequestType;

struct CynaraRequest
{
    CynaraRequest(RequestType type,
                  cynara_agent_req_id id = cynara_agent_req_id(0),
                  std::string user = "",
                  std::string app = "",
                  std::string perm = "")
        : id(id),
          type(type),
          perm(perm),
          app(app),
          user(user) 
    {}

    CynaraRequest() = default;

    cynara_agent_req_id id;
    RequestType type;
    std::string perm;
    std::string app;
    std::string user;
};

typedef std::shared_ptr<CynaraRequest> CynaraRequestPtr;

} //namespace AskUser
