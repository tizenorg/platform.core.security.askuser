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

#include <common/Exception.h>
#include <common/Translator.h>
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

void inline answer(void *data, GuiResponse response)
{
  ALOGD("User selected: " + GuiResponseToString(response));

  if (data == NULL)
    return;

  PopupData *res = static_cast<PopupData*>(data);
  res->type = response;
  win_close(res->win);
}

void allow_answer(void *data, Evas_Object *, void *)
{
  answer(data, GuiResponse::Allow);
}

void deny_answer(void *data, Evas_Object *, void *)
{
  answer(data, GuiResponse::Deny);
}

void never_answer(void *data, Evas_Object *, void *)
{
  answer(data, GuiResponse::Never);
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
  popupData = new PopupData({GuiResponse::Deny, nullptr});
}

void GuiRunner::initialize()
{
  elm_init(0, NULL);

  //placeholder
  win = elm_win_add(NULL, dgettext(PROJECT_NAME, "SID_PRIVILEGE_REQUEST_DIALOG_TITLE"),
                                   ELM_WIN_DOCK);
  elm_win_autodel_set(win, EINA_TRUE);
  elm_win_override_set(win, EINA_TRUE);
  elm_win_alpha_set(win, EINA_TRUE);

  // popup
  popup = elm_popup_add(win);
  elm_object_part_text_set(popup, "title,text", dgettext(PROJECT_NAME,
                                                         "SID_PRIVILEGE_REQUEST_DIALOG_TITLE"));

  // box
  box = elm_box_add(popup);
  evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, 0);
  evas_object_size_hint_align_set(box, EVAS_HINT_FILL, 0.0);

  // content
  content = elm_label_add(popup);
  elm_object_style_set(content, "elm.swallow.content");
  elm_label_line_wrap_set(content, ELM_WRAP_MIXED);
  evas_object_size_hint_weight_set(content, EVAS_HINT_EXPAND, 0.0);
  evas_object_size_hint_align_set(content, EVAS_HINT_FILL, EVAS_HINT_FILL);

  evas_object_show(content);
  elm_box_pack_end(box, content);
  elm_object_part_content_set(popup, "default", box);

  // buttons
  allowButton = elm_button_add(popup);
  elm_object_part_content_set(popup, "button1", allowButton);
  elm_object_text_set(allowButton, dgettext(PROJECT_NAME,
                                            "SID_PRIVILEGE_REQUEST_DIALOG_BUTTON_ALLOW"));

  neverButton = elm_button_add(popup);
  elm_object_text_set(neverButton, dgettext(PROJECT_NAME,
                                            "SID_PRIVILEGE_REQUEST_DIALOG_BUTTON_NEVER"));
  elm_object_part_content_set(popup, "button2", neverButton);

  denyButton = elm_button_add(popup);
  elm_object_text_set(denyButton, dgettext(PROJECT_NAME,
                                           "SID_PRIVILEGE_REQUEST_DIALOG_BUTTON_DENY"));
  elm_object_part_content_set(popup, "button3", denyButton);

  // callbacks
  evas_object_smart_callback_add(win, "unfocused", unfocused, popupData);
  evas_object_smart_callback_add(allowButton, "clicked", allow_answer, popupData);
  evas_object_smart_callback_add(neverButton, "clicked", never_answer, popupData);
  evas_object_smart_callback_add(denyButton, "clicked", deny_answer, popupData);

  popupData->win = win;
  initialized = true;

}

GuiResponse GuiRunner::popupRun(const std::string &app, const std::string &perm)
{
  if (!m_dropHandler)
    throw Exception("DropHandler was not initialized");

  if (!initialized)
    initialize();

  running = true;
  drop *Drop = new drop({m_dropHandler, popupData});
  timer = ecore_timer_add(.1, timeout_answer, Drop);

  // create message
  char *messageFormat = dgettext(PROJECT_NAME, "SID_PRIVILEGE_REQUEST_DIALOG_MESSAGE");
  char buf[BUFSIZ];
  int ret = std::snprintf(buf, sizeof(buf), messageFormat,
                          app.c_str(),
                          friendlyPrivilegeName(perm).c_str());

  if (ret < 0)
    throw Exception("snprintf failed", errno);

  popupData->type = GuiResponse::None;
  should_raise = true;

  elm_object_text_set(content, buf);

  evas_object_show(popup);
  evas_object_show(win);

  elm_win_raise(win);
  elm_run();

  ecore_timer_del(timer);
  running = false;

  return popupData->type;
}

void GuiRunner::setDropHandler(DropHandler dropHandler)
{
  m_dropHandler = dropHandler;
}

void GuiRunner::stop()
{
  if (running) {
    evas_object_hide(win);
    elm_exit();
  }

  elm_shutdown();
}

} /* namespace Notification */

} /* namespace AskUser */
