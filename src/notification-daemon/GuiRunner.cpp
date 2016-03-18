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
 * @file        GuiRunner.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Definition of GuiRunner class
 */

#include "GuiRunner.h"

#include <common/Exception.h>
#include <common/Translator.h>
#include <privilegemgr/privilege_info.h>

namespace {

void inline answer(void *data, GuiResponse response)
{
  LOGD("User selected: " + GuiResponseToString(response));

  if (data == NULL)
    return;

  PopupData *res = static_cast<PopupData*>(data);
  res->type = response;
  evas_object_hide(res->win);
  elm_exit();
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
    evas_object_hide(d->popup->win);
    elm_exit();
  }

  return ECORE_CALLBACK_RENEW;
}

std::string friendlyPrivilegeName(const std::string &privilege)
{
  char *name = nullptr;
  int res = privilege_info_get_privilege_display_name(privilege.c_str(), &name);
  if (res != PRVMGR_ERR_NONE || !name) {
    LOGE("Unable to get privilege display name for: <" << privilege << ">, err: <" << res << ">");
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
  win = elm_win_add(NULL, "Privilege", ELM_WIN_DOCK);
  elm_win_autodel_set(win, EINA_TRUE);
  elm_win_override_set(win, EINA_TRUE);
  elm_win_alpha_set(win, EINA_TRUE);

  // popup
  popup = elm_popup_add(win);
  elm_object_part_text_set(popup, "title,text", "Privilege request");

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

  //elm_object_text_set(content, "perm_content");
  evas_object_show(content);
  elm_box_pack_end(box, content);
  elm_object_part_content_set(popup, "default", box);

  // buttons
  allowButton = elm_button_add(popup);
  elm_object_part_content_set(popup, "button1", allowButton);
  elm_object_text_set(allowButton, "Allow");

  neverButton = elm_button_add(popup);
  elm_object_text_set(neverButton, "Never");
  elm_object_part_content_set(popup, "button2", neverButton);

  denyButton = elm_button_add(popup);
  elm_object_text_set(denyButton, "Deny");
  elm_object_part_content_set(popup, "button3", denyButton);

  // callbacks
  evas_object_smart_callback_add(win, "unfocused", deny_answer, popupData);
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

  elm_object_text_set(content, std::string("Application <b>" + app + "</b> requested privilege <b>"
                                           + friendlyPrivilegeName(perm) + "</b>").c_str());

  evas_object_show(popup);
  evas_object_show(win);

  popupData->type = GuiResponse::None;

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
