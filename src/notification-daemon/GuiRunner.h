#pragma once

#include <string>
#include <Elementary.h>

#include <common/notification-ipc.h>

struct PopupData {
  GuiResponse type;
  Evas_Object *win;
};

class GuiRunner
{
public:
  GuiRunner();

  GuiResponse popupRun(const std::string &app, const std::string &perm);


private:

  PopupData *popupData;

  Evas_Object *win;
  Evas_Object *popup;
  Evas_Object *box;
  Evas_Object *content;
  Evas_Object *allowButton;
  Evas_Object *neverButton;
  Evas_Object *denyButton;

  void initialize();
};
