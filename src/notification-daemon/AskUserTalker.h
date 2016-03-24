#pragma once

#include <functional>
#include <queue>
#include <mutex>

#include <common/notification-ipc.h>

#include "GuiRunner.h"

class AskUserTalker
{
public:
  AskUserTalker(GuiRunner *gui);

  void run();

private:
  GuiRunner *m_gui;
};
