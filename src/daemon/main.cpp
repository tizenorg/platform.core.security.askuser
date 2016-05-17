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
 * @file        main.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Main askuser daemon file
 */

#include <csignal>
#include <functional>
#include <systemd/sd-daemon.h>
#include <type_traits>
#include <unistd.h>

#include <common/Exception.h>
#include <common/log.h>

#include "CynaraTalker.h"
#include "NotificationTalker.h"

AskUser::Daemon::CynaraTalkerPtr cynaraTalker = nullptr;

void kill_handler(int) {
  LOGE("Killme!");
  if (cynaraTalker)
    cynaraTalker->stop();
  LOGE("Closed!");
  exit(EXIT_SUCCESS);
  abort();
}

int main()
{
  using namespace std::placeholders;
  using namespace AskUser::Daemon;
  init_log();

  try {
    int ret;
    struct sigaction act;

    memset(&act, 0, sizeof(act));
    act.sa_handler = &kill_handler;
    if ((ret = sigaction(SIGTERM, &act, NULL)) < 0)
      throw AskUser::Exception("Sigaction failed", errno);

    cynaraTalker = std::move(CynaraTalkerPtr(new CynaraTalker));
    NotificationTalker notificationTalker;

    RequestHandler requestHandler = std::bind(&NotificationTalker::parseRequest, &notificationTalker, _1);
    ResponseHandler responseHandler = std::bind(&CynaraTalker::addResponse, cynaraTalker.get(), _1);

    notificationTalker.setResponseHandler(responseHandler);
    cynaraTalker->setRequestHandler(requestHandler);

    cynaraTalker->start();

    ret = sd_notify(0, "READY=1");
    if (ret == 0) {
        LOGW("Agent was not configured to notify its status");
    } else if (ret < 0) {
        LOGE("sd_notify failed: [" << ret << "]");
    }

    notificationTalker.run();
  } catch (std::exception &e) {
    LOGE("Askuserd stopped because of: <" << e.what() << ">.");
  } catch (...) {
    LOGE("Askuserd stopped because of unknown unhandled exception.");
  }

  LOGI("exiting");

  cynaraTalker->stop();

  return EXIT_SUCCESS;
}
