#if 0
void set_level(const char *user, const char *client, const char *privilege, int level /* allow/never */) {
  // on error raise std::expection;
}

void resp_to_cynara(int id, int allow_or_deny, int cache);

void parse_response(Notif_send* notif_response /* aka nr */) {
  void *what; // find from vec/arr
  remove_from_array(what);

  bool determinal = nr->type == ALLOW || nr->type == NEVER;

  if(determinal)
    set_level(what->creds, nr->type);

  add_to_queue_to_send_to_cynara()
      resp_to_cynara(nr->id, nr->type, determinal);
}

bool stop = false;

while(!stop) {
  if(empty)
}

void parse_cynara_request(cynara_req *req) {
  add_to_arr();
  add_to_queue_to_send_to_notif();
}


std::map<user, sockfd> sockets;
void send_to_notif(req) {

  int fd = getFdByUser();
  write(fd, notif_recv, sizeof(notif_recv) - 2*sizeof(char*) );
  write(str_implode(freindly(client), freindly(privilege)));

}
#endif

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include <iostream>
#include <thread>

#include "NotificationTalker.h"
#include <common/notification-ipc.h>


int main()
{
  int ret;

  struct sockaddr_un local, remote;
  int sockfd;

  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

  if (sockfd == -1)
    return -1;

  local.sun_family = AF_UNIX;
  strcpy(local.sun_path, socketpath);
  unlink(socketpath);
  int len = strlen(local.sun_path) + sizeof(local.sun_family);

  ret = bind(sockfd, (struct sockaddr *)&local, len);
  if (ret == -1)
    return -2;

  chmod(socketpath, 0777);

  ret = listen(sockfd, 10);
  if (ret == -1)
    return -3;

  NotificationTalker notif;

  for (;;) {
    socklen_t t;
    int fd = accept(sockfd, (sockaddr*)&remote, &t);
    notif.add(fd);
  }

  return 0;
}
