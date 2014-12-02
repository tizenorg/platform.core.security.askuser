/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/**
 * @file        Agent.h
 * @author      Adam Malinowski <a.malinowsk2@partner.samsung.com>
 * @brief       This file defines main class of ask user agent
 */

#pragma once

#include <condition_variable>
#include <map>
#include <mutex>
#include <types/PolicyType.h>

#include <containers/ConcurrentQueue.h>

#include <main/CynaraTalker.h>
#include <main/Request.h>
#include <main/Response.h>

#include <ui/AskUIInterface.h>

namespace AskUser {

namespace Agent {

class Agent {
public:
    Agent();
    ~Agent();

    void run();

private:
    CynaraTalker m_cynaraTalker;
    std::map<RequestId, Request> m_requests;
    AskUser::ConcurrentQueue<Request> m_incomingRequests;
    AskUser::ConcurrentQueue<Response> m_incomingResponses;
    std::condition_variable m_event;
    std::mutex m_mutex;
    std::map<RequestId, AskUIInterfacePtr> m_UIs;

    void init();
    void finish();

    void requestHandler(const Request &request);
    void UIResponseHandler(RequestId requestId, UIResponseType responseType);

    void processCynaraRequest(const Request &request);
    bool startUIForRequest(const Request &request);
    bool cleanupUIThreads();

    static Cynara::PolicyType UIResponseToPolicyType(UIResponseType responseType);
};

} // namespace Agent

} // namespace AskUser
