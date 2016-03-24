#include "NotificationTalker.h"

#include <cynara-creds-socket.h>
#include "../common/notification-ipc.h"
#include <iostream>

NotificationTalker::NotificationTalker()
{
}

void NotificationTalker::add(int fd)
{
  char *user_c = nullptr;
  cynara_creds_socket_get_user(fd, USER_METHOD_DEFAULT,&user_c);
  std::string user = user_c ? user_c : "0";
  fdMap[user] = fd;
  free(user_c);
  //FD_SET(fd ,&fdSet);

  for (int i = 0; true; ++i) {

    GuiResponse resp;
    Recieve buf;
    int buf_siz = buf.set("app ka "+ std::to_string(i), "perma: user" + user);
    send(fd, &buf, buf_siz, 0);

    recv(fd, &resp, sizeof(resp), 0);

    switch (resp)
    {
    case GuiResponse::Allow:
      std::cout << "allow" << std::endl;
      break;
    case GuiResponse::Deny:
      std::cout << "deny" << std::endl;
      break;
    case GuiResponse::Never:
      std::cout << "never" << std::endl;
      break;
    default:
      std::cout << "error" << std::endl;
      break;
    }
  }
}

void NotificationTalker::start()
{
  m_thread = std::thread(&NotificationTalker::run, this);
}

void NotificationTalker::addReq(CynaraRequest q)
{

}

void NotificationTalker::run()
{
  while(m_stopflag) {

  }
}
