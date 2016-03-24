#include "GuiRunner.h"

int i = 0;

namespace {

void allow_answer(void *data, Evas_Object *, void *)
{
  if (data == NULL)
    return;

  PopupData *res = static_cast<PopupData*>(data);
  res->type = GuiResponse::Allow;
  evas_object_hide(res->win);
  elm_exit();
}

void deny_answer(void *data, Evas_Object *, void *)
{
  if (data == NULL)
    return;

  PopupData *res = static_cast<PopupData*>(data);
  res->type = GuiResponse::Deny;
  evas_object_hide(res->win);
  elm_exit();
}

void never_answer(void *data, Evas_Object *, void *)
{
  if (data == NULL)
    return;

  PopupData *res = static_cast<PopupData*>(data);
  res->type = GuiResponse::Never;
  evas_object_hide(res->win);
  elm_exit();
}

Eina_Bool timeout_answer(void *data)
{
  PopupData *res = static_cast<PopupData*>(data);
  res->type = GuiResponse::Error;
  evas_object_hide(res->win);
  elm_exit();
  return ECORE_CALLBACK_RENEW;
}

} /* namespace */

GuiRunner::GuiRunner()
{
  popupData = new PopupData({GuiResponse::Deny, nullptr});
  initialize();
}

void GuiRunner::initialize()
{
  elm_init(0, NULL);

  //placeholder
  win = elm_win_add(NULL, "kotek", ELM_WIN_DOCK);
  elm_win_autodel_set(win, EINA_TRUE);
  elm_win_override_set(win, EINA_TRUE);
  elm_win_alpha_set(win, EINA_TRUE);

  // popup
  popup = elm_popup_add(win);
  elm_object_part_text_set(popup, "title,text", "Privilege?");

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

  elm_object_text_set(content, "perm_content");
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
  //evas_object_smart_callback_add(win, "unfocused", deny_answer, popupData);
  evas_object_smart_callback_add(allowButton, "clicked", allow_answer, popupData);
  evas_object_smart_callback_add(neverButton, "clicked", never_answer, popupData);
  evas_object_smart_callback_add(denyButton, "clicked", deny_answer, popupData);
  //Ecore_Timer* timer = ecore_timer_add(5, timeout_answer, popupData);

}

GuiResponse GuiRunner::popupRun(const std::string &app, const std::string &perm)
{
  elm_object_text_set(content, std::string("App: " + app + "\nPermission: " + perm).c_str());

  evas_object_show(popup);
  evas_object_show(win);

  popupData->type = GuiResponse::Error;

  elm_run();

  return popupData->type;
}
