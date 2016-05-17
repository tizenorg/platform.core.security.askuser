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
 * @file        GuiRunner.h
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Declaration of AskUserTalker class
 */

#ifndef __ASKUSER_TALKER__
#define __ASKUSER_TALKER__

#include <functional>
#include <queue>
#include <memory>
#include <mutex>

#include <common/Types.h>

#include "GuiRunner.h"

namespace AskUser {

namespace Notification {

class AskUserTalker
{
public:
  AskUserTalker(GuiRunner *gui);
  ~AskUserTalker();

  void run();
  void stop();

  bool shouldDismiss();

private:
  GuiRunner *m_gui;
  int sockfd;
  bool stopFlag = false;
};

typedef std::unique_ptr<AskUserTalker> AskUserTalkerPtr;

} /* namespace Notification */

} /* namespace AskUser */

#endif /* __ASKUSER_TALKER__ */
