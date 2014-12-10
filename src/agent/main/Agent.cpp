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
 * @file        Agent.cpp
 * @author      Adam Malinowski <a.malinowsk2@partner.samsung.com>
 * @brief       This file implements main class of ask user agent
 */

#include <memory>
#include <sstream>
#include <string>
#include <unistd.h>
#include <utility>

#include <attributes/attributes.h>
#include <log/log.h>
#include <translator/Translator.h>
#include <types/AgentErrorMsg.h>
#include <types/SupportedTypes.h>

#include <ui/AskUINotificationBackend.h>

#include "Agent.h"

namespace AskUser {

namespace Agent {

Agent::Agent() : m_cynaraTalker([&](const Request &request) -> void { requestHandler(request); }) {
    init();
}

Agent::~Agent() {
    finish();
}

void Agent::init() {
    // TODO: implement if needed

    LOGD("Agent daemon initialized");
}

void Agent::run() {
    m_cynaraTalker.start();

    while (true) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_event.wait(lock);

        Request request;
        if (m_incomingRequests.pop(request)) {
            LOGD("Request popped from queue:"
                 " type [" << request.type() << "],"
                 " id [" << request.id() << "],"
                 " data length [" << request.data().size() << "]");

            if (request.type() == RT_Close) {
                break;
            }

            processCynaraRequest(request);
        } else {
            LOGD("No request available in queue");
        }

        Response response;
        if (m_incomingResponses.pop(response)) {
            LOGD("Response popped from queue:"
                 " type [" << response.type() << "],"
                 " id [" << response.id() << "]");

            auto requestIt = m_requests.find(response.id());
            if (requestIt != m_requests.end()) {
                Cynara::PluginData pluginData;
                if (response.type() == URT_ERROR) {
                    pluginData = Translator::Agent::answerToData(Cynara::PolicyType(),
                                                                 AgentErrorMsg::Error);
                } else if (response.type() == URT_TIMEOUT) {
                    pluginData = Translator::Agent::answerToData(Cynara::PolicyType(),
                                                                 AgentErrorMsg::Timeout);
                } else {
                    pluginData = Translator::Agent::answerToData(
                                                    UIResponseToPolicyType(response.type()), "");
                }
                m_cynaraTalker.sendResponse(RT_Action, requestIt->second.id(), pluginData);
                m_requests.erase(requestIt);
            }

            auto it = m_UIs.find(response.id());
            if (it != m_UIs.end()) {
                if (it->second->dismiss()) {
                    it = m_UIs.erase(it);
                }
            }
        } else {
            LOGD("No responses available in queue");
        }

        cleanupUIThreads();
    }

    //TODO: dismiss all threads if possible

    LOGD("Agent task stopped");
}

void Agent::finish() {
    m_cynaraTalker.stop();

    LOGD("Agent daemon has stopped commonly");
}

void Agent::requestHandler(const Request &request) {
    LOGD("Cynara request received:"
         " type [" << request.type() << "],"
         " id [" << request.id() << "],"
         " data length: [" << request.data().size() << "]");

    m_incomingRequests.push(request);
    m_event.notify_one();
}

void Agent::processCynaraRequest(const Request &request) {
    auto existingRequest = m_requests.find(request.id());
    if (existingRequest != m_requests.end()) {
        if (request.type() == RT_Cancel) {
            m_requests.erase(existingRequest);
            m_cynaraTalker.sendResponse(request.type(), request.id(), Cynara::PluginData());
            auto it = m_UIs.find(request.id());
            if (it != m_UIs.end()) {
                if (it->second->dismiss()) {
                    it = m_UIs.erase(it);
                }
            }
        } else {
            LOGE("Incoming request with ID: [" << request.id() << "] is being already processed");
        }
        return;
    }

    if (request.type() == RT_Cancel) {
        LOGE("Cancel request for unknown request: ID: [" << request.id() << "]");
        return;
    }

    for (const auto &req : m_requests) {
        if (req.second.data() == request.data()) {
            LOGI("Request (id: [" << req.second.id() <<
                 "]) with the same plugin data is already being processed.");
            // For now I don't know what to do so I do nothing.
            return;
        }
    }

    if (!startUIForRequest(request)) {
        // Answer to cynara. Translator will be used in order to create data, for now data is empty
        // Also type of request will be RT_Action
        m_cynaraTalker.sendResponse(RT_Action, request.id(), Cynara::PluginData());
        return;
    }

    m_requests.insert(std::make_pair(request.id(), request));
}

bool Agent::startUIForRequest(const Request &request) {
    auto data = Translator::Agent::dataToRequest(request.data());
    auto ui = std::make_shared<AskUINotificationBackend>();

    auto handler = [&](RequestId requestId, UIResponseType resultType) -> void {
                       UIResponseHandler(requestId, resultType);
                   };
    bool ret = ui->start(data.client, data.user, data.privilege, request.id(), handler);
    if (ret) {
        m_UIs[request.id()] = ui;
    }

    return ret;
}

void Agent::UIResponseHandler(RequestId requestId, UIResponseType responseType) {
    LOGD("UI response received: type [" << responseType << "], id [" << requestId << "]");

    m_incomingResponses.push(Response(requestId, responseType));
    m_event.notify_one();
}

bool Agent::cleanupUIThreads() {
    bool ret = true;
    for (auto it = m_UIs.begin(); it != m_UIs.end();) {
        if (it->second->isDismissing() && it->second->dismiss()) {
            it = m_UIs.erase(it);
        } else {
            ret = false;
            ++it;
        }
    }
    return ret;
}

Cynara::PolicyType Agent::UIResponseToPolicyType(UIResponseType responseType) {
    switch (responseType) {
        case URT_YES:
            return AskUser::SupportedTypes::Client::ALLOW_ONCE;
        case URT_SESSION:
            return AskUser::SupportedTypes::Client::ALLOW_PER_SESSION;
        case URT_NO:
            return Cynara::PredefinedPolicyType::DENY;
        default:
            return Cynara::PredefinedPolicyType::DENY;
    }
}

} // namespace Agent

} // namespace AskUser
