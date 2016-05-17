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
 * @file        ???
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       ???
 */

#ifndef SRC_COMMON_IPC
#define SRC_COMMON_IPC

#include <cstring>
#include <memory>
#include <string>

#include <cynara-agent.h>
#include <cynara-plugin.h>

namespace AskUser {

typedef cynara_agent_req_id RequestId;
typedef cynara_agent_msg_type CynaraRequestType;

enum class GuiResponse
{
    Allow,
    Deny,
    Never,
    Error,
    None
};

struct Response {
    RequestId id;
    GuiResponse response;
};

typedef enum {
    RT_Action,
    RT_Cancel,
    RT_Close,
    RT_Ignore
} RequestType;

struct CynaraRequest
{
    CynaraRequest(RequestType type, RequestId id = RequestId(0), std::string user = "",
                  std::string app = "", std::string perm = "") : id(id), type(type), perm(perm),
                  app(app), user(user) {}

    CynaraRequest() = default;

    RequestId id;
    RequestType type;
    std::string perm;
    std::string app;
    std::string user;
};

typedef std::shared_ptr<CynaraRequest> CynaraRequestPtr;

struct NotificationRequest
{
    RequestId id;
    std::string app;
    std::string privilege;
};

constexpr char socketpath[] = "/run/askuserd.socket";
constexpr uint8_t dissmisCode = 0xDE;
constexpr uint8_t ackCode = 0xAC;

} /* namespace AskUser */

#endif // SRC_COMMON_IPC
