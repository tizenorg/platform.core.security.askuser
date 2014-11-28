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
 * @file        CynaraTalker.cpp
 * @author      Adam Malinowski <a.malinowsk2@partner.samsung.com>
 * @brief       This file implements class of cynara talker
 */

#include <cassert>

#include <attributes/attributes.h>
#include <log/log.h>
#include <types/SupportedTypes.h>

#include "CynaraTalker.h"

namespace {
    AskUser::Agent::RequestType cynaraType2AgentType(cynara_agent_msg_type type) {
        switch (type) {
            case CYNARA_MSG_TYPE_ACTION:
                return AskUser::Agent::RT_Action;
            case CYNARA_MSG_TYPE_CANCEL:
                return AskUser::Agent::RT_Cancel;
        }

        return AskUser::Agent::RT_Cancel; // Just for compiler
    }

    cynara_agent_msg_type agentType2CynaraType(AskUser::Agent::RequestType type) {
        assert(type != AskUser::Agent::RT_Close); // Rather should be changed to exception

        switch (type) {
            case AskUser::Agent::RT_Action:
                return CYNARA_MSG_TYPE_ACTION;
            case AskUser::Agent::RT_Cancel:
                return CYNARA_MSG_TYPE_CANCEL;
            case AskUser::Agent::RT_Close:
                // This should never happen - it is just for compiler
                return CYNARA_MSG_TYPE_CANCEL;
        }

        return CYNARA_MSG_TYPE_CANCEL; // Just for compiler
    }
}

namespace AskUser {

namespace Agent {

CynaraTalker::CynaraTalker(RequestHandler requestHandler) : m_requestHandler(requestHandler),
                                                            m_cynara(nullptr) {
    m_future = m_threadFinished.get_future();
}

bool CynaraTalker::start() {
    m_thread = std::thread(&CynaraTalker::run, this);
    return true;
}

bool CynaraTalker::stop() {
    // There is no possibility to stop this thread nicely when it waits for requests from cynara
    // We can only try to get rid of thread
    auto status = m_future.wait_for(std::chrono::milliseconds(10));
    if (status == std::future_status::ready) {
        LOGD("Cynara thread finished and ready to join.");
        m_thread.join();
        return true;
    }

    LOGD("Cynara thread not finished.");
    return false;
}

void CynaraTalker::run() {
    if (!m_requestHandler) {
        LOGE("Empty request handler!");
        return;
    }

    int ret = cynara_agent_initialize(&m_cynara, SupportedTypes::Agent::AgentType);
    if (ret != CYNARA_API_SUCCESS) {
        LOGE("Initialization of cynara structure failed with error: [" << ret << "]");
        m_requestHandler(Request(RT_Close, 0, nullptr, 0)); // Notify agent he should die
        return;
    }

    try {
        while (true) {
            cynara_agent_msg_type req_type;
            cynara_agent_req_id req_id;
            void *data = nullptr;
            size_t data_size = 0;

            ret = cynara_agent_get_request(m_cynara, &req_type, &req_id, &data, &data_size);
            if (ret != CYNARA_API_SUCCESS) {
                LOGE("Receiving request from cynara failed with error: [" << ret << "]");
                m_requestHandler(Request(RT_Close, 0, nullptr, 0));
                free(data);
                break;
            }

            m_requestHandler(Request(cynaraType2AgentType(req_type), req_id, data, data_size));
            free(data);
        }

        std::unique_lock<std::mutex> mlock(m_mutex);

        ret = cynara_agent_finish(m_cynara);
        m_cynara = nullptr;
        if (ret != CYNARA_API_SUCCESS) {
            LOGE("Finishing cynara connection failed with error: [" << ret << "]");
        }
    } catch (const std::exception &e) {
        LOGC("Unexpected exception: <" << e.what() << ">");
    } catch (...) {
        LOGE("Unexpected unknown exception caught!");
    }
}

bool CynaraTalker::sendResponse(RequestType requestType, RequestId requestId,
                                const Cynara::PluginData &data) {

    std::unique_lock<std::mutex> mlock(m_mutex);

    if (!m_cynara) {
        LOGE("Trying to send response using uninitialized cynara connection!");
        return false;
    }

    int ret = cynara_agent_put_response(m_cynara, agentType2CynaraType(requestType), requestId,
                                        data.size() ? data.data() : nullptr, data.size());
    if (ret != CYNARA_API_SUCCESS) {
        LOGE("Sending response to cynara failed with error: [" << ret << "]");
        return false;
    }

    return true;
}

} // namespace Agent

} // namespace AskUser
