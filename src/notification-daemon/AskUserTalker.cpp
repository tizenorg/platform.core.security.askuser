#include "AskUserTalker.h"

#include <string>

#include <sys/socket.h>
#include <sys/un.h>

#include <iostream>
#include <errno.h>

AskUserTalker::AskUserTalker(GuiRunner *gui) : m_gui(gui) {}

void AskUserTalker::run()
{
  int ret;
  struct sockaddr_un remote;
  int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

  remote.sun_family = AF_UNIX;
  strcpy(remote.sun_path, socketpath);

  int len = strlen(remote.sun_path) + sizeof(remote.sun_family);
  ret = connect(sockfd, (struct sockaddr *)&remote, len);

  if(ret == -1) {
    std::cout << "errno: " << errno << std::endl;
    return;
  }

  Recieve buf;
  GuiResponse response;

  for(int i = 0; true; i++)
  {
    int len = recv(sockfd, &buf, sizeof(buf), 0);
    if(len < 0) {
      std::cout << "len < 0" << std::endl;
      continue;
    }
    if(len == 0) {
      std::cout << "len == 0" << std::endl;
      continue;
    }

    std::string app, perm;
    buf.get(app, perm);

    response = m_gui->popupRun(app, perm);

    send(sockfd, &response, sizeof(response), 0);
  }
}
