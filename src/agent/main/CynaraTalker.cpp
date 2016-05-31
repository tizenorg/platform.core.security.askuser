/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file        CynaraTalker.cpp
 * @author      Adam Malinowski <a.malinowsk2@partner.samsung.com>
 * @brief       This file implements class of cynara talker
 */

#include <csignal>
#include <string>

#include <attributes/attributes.h>
#include <types/SupportedTypes.h>

#include <log/alog.h>

#include "CynaraTalker.h"

namespace {

class TypeException : std::exception {
public:
    TypeException(const std::string &msg) : m_what(msg) {};
    virtual const char* what() const noexcept {
        return m_what.c_str();
    }

private:
    std::string m_what;
};

AskUser::Agent::RequestType cynaraType2AgentType(cynara_agent_msg_type type) {
    switch (type) {
        case CYNARA_MSG_TYPE_ACTION:
            return AskUser::Agent::RT_Action;
        case CYNARA_MSG_TYPE_CANCEL:
            return AskUser::Agent::RT_Cancel;
    }

    throw TypeException("Unsupported request type: " + std::to_string(type) +
                        " received from cynara.");
}

cynara_agent_msg_type agentType2CynaraType(AskUser::Agent::RequestType type) {
    switch (type) {
        case AskUser::Agent::RT_Action:
            return CYNARA_MSG_TYPE_ACTION;
        case AskUser::Agent::RT_Cancel:
            return CYNARA_MSG_TYPE_CANCEL;
        default: // let's make compiler happy
            break;
    }

    throw TypeException("Invalid response type: " + std::to_string(type) + " to send to cynara.");
}

Cynara::PolicyType GuiResponseToPolicyType(AskUser::GuiResponse responseType) {
    switch (responseType) {
    case GuiResponse::Allow:
        return AskUser::SupportedTypes::Client::ALLOW_PER_LIFE;
    case GuiResponse::Never:
        return AskUser::SupportedTypes::Client::DENY_PER_LIFE;
    default:
        return AskUser::SupportedTypes::Client::DENY_ONCE;
  }
}

}

namespace AskUser {

namespace Agent {

CynaraTalker::CynaraTalker(RequestHandler requestHandler) : m_requestHandler(requestHandler),
                                                            m_cynara(nullptr), m_run(true) {
    m_future = m_threadFinished.get_future();
}

bool CynaraTalker::start() {
    if (!m_requestHandler) {
        ALOGE("Empty request handler!");
        return false;
    }

    m_thread = std::thread(&CynaraTalker::run, this);
    return true;
}

bool CynaraTalker::stop() {
    // There is no possibility to stop this thread nicely when it waits for requests from cynara
    // We can only try to get rid of thread
    m_run = false;
    auto status = m_future.wait_for(std::chrono::milliseconds(10));
    if (status == std::future_status::ready) {
        ALOGD("Cynara thread finished and ready to join.");
        m_thread.join();
        return true;
    }

    ALOGD("Cynara thread not finished.");
    return false;
}

void CynaraTalker::run() {
    int ret;
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    if ((ret = sigprocmask(SIG_BLOCK, &mask, nullptr)) < 0) {
        ALOGE("sigprocmask failed [<<" << ret << "]");
    }

    ret = cynara_agent_initialize(&m_cynara, SupportedTypes::Agent::AgentType);
    if (ret != CYNARA_API_SUCCESS) {
        ALOGE("Initialization of cynara structure failed with error: [" << ret << "]");
        m_requestHandler(CynaraRequest(RT_Close)); // Notify agent he should die
        return;
    }

    void *data = nullptr;

    try {
        while (m_run) {
            cynara_agent_msg_type req_type;
            cynara_agent_req_id req_id;
            size_t data_size = 0;

            ret = cynara_agent_get_request(m_cynara, &req_type, &req_id, &data, &data_size);
            if (!m_run) {
                ALOGD("Got info to stop working.");
                break;
            }
            if (ret != CYNARA_API_SUCCESS) {
                ALOGE("Receiving request from cynara failed with error: [" << ret << "]");
                m_requestHandler(CynaraRequest(RT_Close));
                break;
            }

            try {
                std::string user, client, privilege;

                if (data_size > 0)
                    dataToRequest(static_cast<char *>(data), client, user, privilege);
                m_requestHandler(CynaraRequest(cynaraType2AgentType(req_type), req_id,
                                 user, client, privilege));
            } catch (const TypeException &e) {
                ALOGE("TypeException: <" << e.what() << "> Request dropped!");
            }
            free(data);
            data = nullptr;
        }
    } catch (const std::exception &e) {
        ALOGC("Unexpected exception: <" << e.what() << ">");
    } catch (...) {
        ALOGE("Unexpected unknown exception caught!");
    }

    free(data);

    std::unique_lock<std::mutex> mlock(m_mutex);
    ret = cynara_agent_finish(m_cynara);
    m_cynara = nullptr;
    if (ret != CYNARA_API_SUCCESS) {
        ALOGE("Finishing cynara connection failed with error: [" << ret << "]");
    }

    m_threadFinished.set_value(true);
}

bool CynaraTalker::sendResponse(Response response) {

    std::unique_lock<std::mutex> mlock(m_mutex);
    
    if (!m_cynara) {
        ALOGE("Trying to send response using uninitialized cynara connection!");
        return false;
    }

    std::string data = answerToData(GuiResponseToPolicyType(response.response));
    int ret;
    try {
        ret = cynara_agent_put_response(m_cynara, CYNARA_MSG_TYPE_ACTION, response.id,
                                        static_cast<void*>(data.c_str()), data.size());
    } catch (const TypeException &e) {
        ALOGE("TypeException: <" << e.what() << "> Response dropped!");
        ret = CYNARA_API_INVALID_PARAM;
    }

    if (ret != CYNARA_API_SUCCESS) {
        ALOGE("Sending response to cynara failed with error: [" << ret << "]");
        return false;
    }

    return true;
}

} // namespace Agent

} // namespace AskUser
