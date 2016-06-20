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

#pragma once

#include <Elementary.h>
#include <functional>
#include <string>

#include <types/NotificationResponse.h>
#include <log/alog.h>

namespace AskUser {

namespace Notification {

typedef std::function<bool()> DropHandler;

struct PopupData {
    NResponseType type;
    Evas_Object *win;
};

struct drop {
    DropHandler handle;
    PopupData *popup;
};

class GuiRunner {
public:
    GuiRunner();
    ~GuiRunner();

    NResponseType popupRun(const std::string &app, const std::string &perm);

    void setDropHandler(DropHandler dropHandler);
    void stop();

    std::string getErrorMsg() { return m_errorMsg; }

private:
    PopupData *m_popupData;
    DropHandler m_dropHandler;

    Evas_Object *m_win;
    Evas_Object *m_popup;
    Evas_Object *m_box;
    Evas_Object *m_content;
    Evas_Object *m_allowButton;
    Evas_Object *m_neverButton;
    Evas_Object *m_denyButton;
    Ecore_Timer *m_timer;

    bool m_running = false;
    bool m_initialized = false;

    std::string m_errorMsg;

    void initialize();
};

} /* namespace Notification */

} /* namespace AskUser */
