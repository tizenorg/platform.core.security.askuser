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
 * @file        src/notification-daemon/GuiRunner.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Definition of GuiRunner class
 */

#include "GuiRunner.h"

#include <exception/ErrnoException.h>
#include <exception/Exception.h>
#include <translator/Translator.h>
#include <libintl.h>
#include <privilegemgr/privilege_info.h>

namespace AskUser {

namespace Notification {

namespace {

bool should_raise = false;

void unfocused(void *data, Evas_Object *, void *)
{
    if (data == NULL)
        return;

    PopupData *res = static_cast<PopupData*>(data);

    if (should_raise)
        elm_win_raise(res->win);
    else
        elm_exit();
}

void inline win_close(Evas_Object *win) {
    should_raise = false;
    elm_win_lower(win);
}

void inline answer(void *data, NResponseType response)
{
    ALOGD("User selected: " + Translator::Gui::responseToString(response));

    if (data == NULL)
        return;

    PopupData *res = static_cast<PopupData*>(data);
    res->type = response;
    win_close(res->win);
}

void allow_answer(void *data, Evas_Object *, void *)
{
    answer(data, NResponseType::Allow);
}

void deny_answer(void *data, Evas_Object *, void *)
{
    answer(data, NResponseType::Deny);
}

void never_answer(void *data, Evas_Object *, void *)
{
    answer(data, NResponseType::Never);
}

Eina_Bool timeout_answer(void *data) {
    if (!data)
        return ECORE_CALLBACK_RENEW;

    drop *d = static_cast<drop*>(data);

    if (d->handle()) {
        win_close(d->popup->win);
    }

    return ECORE_CALLBACK_RENEW;
}

std::string friendlyPrivilegeName(const std::string &privilege)
{
    char *name = nullptr;
    int res = privilege_info_get_privilege_display_name(privilege.c_str(), &name);
    if (res != PRVMGR_ERR_NONE || !name) {
        ALOGE("Unable to get privilege display name for: <" << privilege << ">, err: <" << res << ">");
        return privilege;
    }
    std::string ret(name);
    free(name);
    return ret;
}

} /* namespace */

GuiRunner::GuiRunner()
{
    m_popupData = new PopupData({NResponseType::Deny, nullptr});
}

GuiRunner::~GuiRunner()
{
    delete m_popupData;
}

void GuiRunner::initialize()
{
    elm_init(0, NULL);

    //placeholder
    m_win = elm_win_add(NULL, dgettext(PROJECT_NAME, "SID_PRIVILEGE_REQUEST_DIALOG_TITLE"),
                                   ELM_WIN_DOCK);
    elm_win_autodel_set(m_win, EINA_TRUE);
    elm_win_override_set(m_win, EINA_TRUE);
    elm_win_alpha_set(m_win, EINA_TRUE);

    // popup
    m_popup = elm_popup_add(m_win);
    elm_object_part_text_set(m_popup, "title,text", dgettext(PROJECT_NAME,
                                                         "SID_PRIVILEGE_REQUEST_DIALOG_TITLE"));

    // box
    m_box = elm_box_add(m_popup);
    evas_object_size_hint_weight_set(m_box, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(m_box, EVAS_HINT_FILL, 0.0);

    // content
    m_content = elm_label_add(m_popup);
    elm_object_style_set(m_content, "elm.swallow.content");
    elm_label_line_wrap_set(m_content, ELM_WRAP_MIXED);
    evas_object_size_hint_weight_set(m_content, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(m_content, EVAS_HINT_FILL, EVAS_HINT_FILL);

    evas_object_show(m_content);
    elm_box_pack_end(m_box, m_content);
    elm_object_part_content_set(m_popup, "default", m_box);

    // buttons
    m_allowButton = elm_button_add(m_popup);
    elm_object_part_content_set(m_popup, "button1", m_allowButton);
    elm_object_text_set(m_allowButton, dgettext(PROJECT_NAME,
                                            "SID_PRIVILEGE_REQUEST_DIALOG_BUTTON_ALLOW"));

    m_neverButton = elm_button_add(m_popup);
    elm_object_text_set(m_neverButton, dgettext(PROJECT_NAME,
                                            "SID_PRIVILEGE_REQUEST_DIALOG_BUTTON_NEVER"));
    elm_object_part_content_set(m_popup, "button2", m_neverButton);

    m_denyButton = elm_button_add(m_popup);
    elm_object_text_set(m_denyButton, dgettext(PROJECT_NAME,
                                           "SID_PRIVILEGE_REQUEST_DIALOG_BUTTON_DENY"));
    elm_object_part_content_set(m_popup, "button3", m_denyButton);

    // callbacks
    evas_object_smart_callback_add(m_win, "unfocused", unfocused, m_popupData);
    evas_object_smart_callback_add(m_allowButton, "clicked", allow_answer, m_popupData);
    evas_object_smart_callback_add(m_neverButton, "clicked", never_answer, m_popupData);
    evas_object_smart_callback_add(m_denyButton, "clicked", deny_answer, m_popupData);

    m_popupData->win = m_win;
    m_initialized = true;

}

NResponseType GuiRunner::popupRun(const std::string &app, const std::string &perm)
{

    try {
        if (!m_dropHandler)
            throw Exception("DropHandler was not initialized");

        if (!m_initialized)
            initialize();

        m_running = true;
        drop *Drop = new drop({m_dropHandler, m_popupData});
        m_timer = ecore_timer_add(0.1, timeout_answer, Drop);

        // create message
        char *messageFormat = dgettext(PROJECT_NAME, "SID_PRIVILEGE_REQUEST_DIALOG_MESSAGE");
        char buf[BUFSIZ];
        int ret = std::snprintf(buf, sizeof(buf), messageFormat,
                                app.c_str(),
                                friendlyPrivilegeName(perm).c_str());

        if (ret < 0)
            throw ErrnoException("snprintf failed", errno);

        m_popupData->type = NResponseType::None;

        should_raise = true;

        elm_object_text_set(m_content, buf);

        evas_object_show(m_popup);
        evas_object_show(m_win);

        elm_win_raise(m_win);
        elm_run();

        ecore_timer_del(m_timer);
        m_running = false;
        should_raise = false;
    } catch (const std::exception &e) {
        m_popupData->type = NResponseType::Error;
        m_errorMsg = std::move(e.what());
    }

    return m_popupData->type;
}

void GuiRunner::setDropHandler(DropHandler dropHandler)
{
    m_dropHandler = dropHandler;
}

void GuiRunner::stop()
{
    if (m_running) {
        evas_object_hide(m_win);
        elm_exit();
    }

    elm_shutdown();
}

} /* namespace Notification */

} /* namespace AskUser */
