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

#include <chrono>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <string>
#include <unistd.h>
#include <utility>

#include <attributes/attributes.h>
#include <translator/Translator.h>
#include <types/AgentErrorMsg.h>
#include <types/SupportedTypes.h>

#include <log/alog.h>
#include <ui/AskUINotificationBackend.h>

#include "Agent.h"

namespace AskUser {

namespace Agent {

volatile sig_atomic_t Agent::m_stopFlag = 0;

Agent::Agent() : m_cynaraTalker([&](RequestPtr request) -> void { requestHandler(request); }) {
    init();
}

Agent::~Agent() {
    finish();
}

void Agent::init() {
    // TODO: implement if needed

    ALOGD("Agent daemon initialized");
}

void Agent::run() {
    m_cynaraTalker.start();

    while (!m_stopFlag) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_event.wait_for(lock, std::chrono::milliseconds(1000));

        if (m_stopFlag) {
            break;
        }

        while (!m_incomingRequests.empty() || !m_incomingResponses.empty()) {

            if (!m_incomingRequests.empty()) {
                RequestPtr request = m_incomingRequests.front();
                m_incomingRequests.pop();
                lock.unlock();

                ALOGD("Request popped from queue:"
                     " type [" << request->type() << "],"
                     " id [" << request->id() << "],"
                     " data length [" << request->data().size() << "]");

                if (request->type() == RT_Close) {
                    m_stopFlag = 1;
                    break;
                }

                processCynaraRequest(request);

                lock.lock();
            }

            if (!m_incomingResponses.empty()) {
                Response response = m_incomingResponses.front();
                m_incomingResponses.pop();
                lock.unlock();

                ALOGD("Response popped from queue:"
                     " type [" << response.type() << "],"
                     " id [" << response.id() << "]");

                processUIResponse(response);

                lock.lock();
            }

            cleanupUIThreads();
        }

    }

    ALOGD("Agent task stopped");
}

void Agent::finish() {
    bool success = cleanupUIThreads() && m_cynaraTalker.stop();
    if (!success) {
        ALOGE("At least one of threads could not be stopped. Calling quick_exit()");
        quick_exit(EXIT_SUCCESS);
    } else {
        ALOGD("Agent daemon has stopped commonly");
    }
}

void Agent::requestHandler(RequestPtr request) {
    ALOGD("Cynara request received:"
         " type [" << request->type() << "],"
         " id [" << request->id() << "],"
         " data length: [" << request->data().size() << "]");

    std::unique_lock<std::mutex> lock(m_mutex);
    m_incomingRequests.push(request);
    m_event.notify_one();
}

void Agent::processCynaraRequest(RequestPtr request) {
    auto existingRequest = m_requests.find(request->id());
    if (existingRequest != m_requests.end()) {
        if (request->type() == RT_Cancel) {
            delete existingRequest->second;
            m_requests.erase(existingRequest);
            m_cynaraTalker.sendResponse(request->type(), request->id(), Cynara::PluginData());
            dismissUI(request->id());
        } else {
            ALOGE("Incoming request with ID: [" << request->id() << "] is being already processed");
        }
        delete request;
        return;
    }

    if (request->type() == RT_Cancel) {
        ALOGE("Cancel request for unknown request: ID: [" << request->id() << "]");
        delete request;
        return;
    }

    if (!startUIForRequest(request)) {
        auto data = Translator::Agent::answerToData(Cynara::PolicyType(), AgentErrorMsg::Error);
        m_cynaraTalker.sendResponse(RT_Action, request->id(), data);
        delete request;
        return;
    }

    m_requests.insert(std::make_pair(request->id(), request));
}

void Agent::processUIResponse(const Response &response) {
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
                                            UIResponseToPolicyType(response.type()),
                                                                   AgentErrorMsg::NoError);
        }
        m_cynaraTalker.sendResponse(RT_Action, requestIt->second->id(), pluginData);
        delete requestIt->second;
        m_requests.erase(requestIt);
    }

    dismissUI(response.id());
}

bool Agent::startUIForRequest(RequestPtr request) {
    auto data = Translator::Agent::dataToRequest(request->data());
    auto ui = std::make_shared<AskUINotificationBackend>();

    auto handler = [&](RequestId requestId, UIResponseType resultType) -> void {
                       UIResponseHandler(requestId, resultType);
                   };
    bool ret = ui->start(data.client, data.user, data.privilege, request->id(), handler);
    if (ret) {
        m_UIs[request->id()] = ui;
    }

    return ret;
}

void Agent::UIResponseHandler(RequestId requestId, UIResponseType responseType) {
    ALOGD("UI response received: type [" << responseType << "], id [" << requestId << "]");

    std::unique_lock<std::mutex> lock(m_mutex);
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

void Agent::dismissUI(RequestId requestId) {
    auto it = m_UIs.find(requestId);
    if (it != m_UIs.end()) {
        if (it->second->dismiss()) {
            it = m_UIs.erase(it);
        }
    }
}

Cynara::PolicyType Agent::UIResponseToPolicyType(UIResponseType responseType) {
    switch (responseType) {
        case URT_YES_ONCE:
            return AskUser::SupportedTypes::Client::ALLOW_ONCE;
        case URT_YES_SESSION:
            return AskUser::SupportedTypes::Client::ALLOW_PER_SESSION;
        case URT_YES_LIFE:
            return AskUser::SupportedTypes::Client::ALLOW_PER_LIFE;
        case URT_NO_ONCE:
            return AskUser::SupportedTypes::Client::DENY_ONCE;
        case URT_NO_SESSION:
            return AskUser::SupportedTypes::Client::DENY_PER_SESSION;
        case URT_NO_LIFE:
            return AskUser::SupportedTypes::Client::DENY_PER_LIFE;
        default:
            return AskUser::SupportedTypes::Client::DENY_ONCE;
    }
}

} // namespace Agent

} // namespace AskUser
