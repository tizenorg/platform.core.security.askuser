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
 * @file        src/notification-daemon/GuiRunner.h
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Declaration of GuiRunner class
 */

#ifndef __GUI_RUNNER__
#define __GUI_RUNNER__

#include <Elementary.h>
#include <functional>
#include <string>

#include <types/Response.h>
#include <log/alog.h>

namespace AskUser {

namespace Notification {

typedef std::function<bool()> DropHandler;

struct PopupData {
  GuiResponse type;
  Evas_Object *win;
};

struct drop {
  DropHandler handle;
  PopupData *popup;
};

class GuiRunner
{
public:
  GuiRunner();

  GuiResponse popupRun(const std::string &app, const std::string &perm);

  void setDropHandler(DropHandler dropHandler);
  void stop();

private:
  PopupData *popupData;
  DropHandler m_dropHandler;

  Evas_Object *win;
  Evas_Object *popup;
  Evas_Object *box;
  Evas_Object *content;
  Evas_Object *allowButton;
  Evas_Object *neverButton;
  Evas_Object *denyButton;
  Ecore_Timer *timer;

  bool running = false;
  bool initialized = false;

  void initialize();
};

} /* namespace Notification */

} /* namespace AskUser */

#endif /* __GUI_RUNNER__ */
