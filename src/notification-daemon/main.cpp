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
 * @file        src/notification-daemon/main.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Main askuser notification daemon file
 */

#include <clocale>
#include <csignal>
#include <cstdlib>
#include <string>
#include <systemd/sd-daemon.h>
#include <thread>
#include <unistd.h>

#include <common/Exception.h>
#include <common/log.h>

#include "GuiRunner.h"
#include "AskUserTalker.h"


int main()
{
  using namespace AskUser::Notification;

  AskUser::init_log();

  char *locale = setlocale(LC_ALL, "");
  ALOGD("Current locale is: <" << locale << ">");

  try {
    AskUserTalkerPtr askUserTalker;
    GuiRunner gui;
    askUserTalker = std::move(AskUserTalkerPtr(new AskUserTalker(&gui)));

    int ret = sd_notify(0, "READY=1");
    if (ret == 0) {
        ALOGW("Agent was not configured to notify its status");
    } else if (ret < 0) {
        ALOGE("sd_notify failed: [" << ret << "]");
    }

    askUserTalker->run();

  } catch (std::exception &e) {
    ALOGE("Askuser-notification stopped because of: <" << e.what() << ">.");
  } catch (...) {
    ALOGE("Askuser-notification stopped because of unknown unhandled exception.");
  }

  return 0;
}
