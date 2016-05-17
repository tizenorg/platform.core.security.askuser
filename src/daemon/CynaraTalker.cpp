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
 * @file        CynaraTalker.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Definition of CynaraTalker class
 */

#include "CynaraTalker.h"

#include <csignal>
#include <cstring>

#include <common/Exception.h>
#include <common/log.h>
#include <common/SupportedTypes.h>
#include <common/Translator.h>

namespace AskUser {

namespace Daemon {

namespace {

RequestType toRequestType(cynara_agent_msg_type type)
{
  switch (type) {
  case CYNARA_MSG_TYPE_ACTION:
    return RequestType::RT_Action;
  case CYNARA_MSG_TYPE_CANCEL:
    return RequestType::RT_Cancel;
  default:
    break;
  }
  return RequestType::RT_Ignore;
}

Cynara::PolicyType GuiResponseToPolicyType(GuiResponse responseType) {
  switch (responseType) {
  case GuiResponse::Allow:
    return AskUser::SupportedTypes::Client::ALLOW_PER_LIFE;
  case GuiResponse::Never:
    return AskUser::SupportedTypes::Client::DENY_PER_LIFE;
  default:
    return AskUser::SupportedTypes::Client::DENY_ONCE;
  }
}

} /* namespace */

void CynaraTalker::start()
{
  m_thread = std::thread(&CynaraTalker::run, this);
}

void CynaraTalker::setRequestHandler(RequestHandler requestHandler)
{
  m_requestHandler = requestHandler;
}

CynaraTalker::~CynaraTalker()
{
  m_stop_thread = true;
  cynara_agent_finish(m_cynara);
  m_thread.join();
}

void CynaraTalker::addResponse(Response response)
{
  std::string sResponse = answerToData(GuiResponseToPolicyType(response.response));

  void *data = strdup(sResponse.c_str());

  int ret = cynara_agent_put_response(m_cynara, CYNARA_MSG_TYPE_ACTION, response.id, static_cast<void*>(data), sResponse.size());
  if (ret != CYNARA_API_SUCCESS) {
    LOGE("putting response to cynara failed: " << ret);
  }

  free(data);
}

void CynaraTalker::stop()
{
  if (!m_stop_thread) {
    if (m_requestHandler)
      m_requestHandler(std::make_shared<CynaraRequest>(RequestType::RT_Close));

    m_stop_thread = true;
  }
}

void CynaraTalker::run()
{
  try {
    int ret;

    LOGD("CynaraTalker starting...");

    if (!m_requestHandler)
      throw Exception("Missing request handler");

    ret = cynara_agent_initialize(&m_cynara, "AskUser");
    if (ret != CYNARA_API_SUCCESS)
      throw CynaraException("cynara_agent_initialize", ret);

    void *data = nullptr;

    LOGD("CynaraTalker running");

    while (!m_stop_thread) {
      CynaraRequestType req_type;
      RequestId req_id;
      size_t data_size = 0;

      ret = cynara_agent_get_request(m_cynara, &req_type, &req_id, &data, &data_size);

      if (m_stop_thread)
        break;

      if (ret != CYNARA_API_SUCCESS) {
        m_requestHandler(std::make_shared<CynaraRequest>(RequestType::RT_Close));
        throw CynaraException("cynara_agent_get_request", ret);
      }

      std::string user, client, privilege;

      if (data_size)
        dataToRequest(static_cast<char *>(data), client, user, privilege);

      auto req = std::make_shared<CynaraRequest>(toRequestType(req_type), req_id, user, client,
                                                 privilege);
      m_requestHandler(req);

      free(data);
      data = nullptr;
    }

  } catch (std::exception &e) {
    LOGE("CynaraTalker stopped because of: <" << e.what() << ">.");
  } catch (...) {
    LOGE("CynaraTalker stopped because of unknown unhandled exception.");
  }
}

} /* namespace Daemon */

} /* namespace AskUser */
