#include "GuiRunner.h"
#include "AskUserTalker.h"

#include <iostream>
#include <csignal>
#include <cstring>
#include <thread>

// Handle kill message from systemd
void kill_handler(int) {
  //
}

int main()
{
  int ret;
  struct sigaction act;

  // Install kill handler - TERM signal will be delivered form systemd to kill this service
  memset(&act, 0, sizeof(act));
  act.sa_handler = &kill_handler;

  sigaction(SIGKILL, &act, NULL);
  if ((ret = sigaction(SIGTERM, &act, NULL)) < 0) {
    return 1;
  }

  GuiRunner gui;
  AskUserTalker talker(&gui);
  talker.run();
}
