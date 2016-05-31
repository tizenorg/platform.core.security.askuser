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
 * @file        src/daemon/NotificationTalker.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Definition of NotificationTalker class
 */

#include "NotificationTalker.h"

#include <algorithm>
#include <cstring>
#include <cynara-creds-socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include <exception/ErrnoException.h>
#include <log/alog.h>
#include <translator/Translator.h>
#include <config/Path.h>
#include <types/Protocol.h>

namespace AskUser {

namespace Daemon {

NotificationTalker::NotificationTalker()
{
  int ret, len;
  sockaddr_un local;

  m_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (m_sockfd == -1)
    throw ErrnoException("Creating socket error", errno);

  local.sun_family = AF_UNIX;
  strcpy(local.sun_path, Path::socketpath.c_str());
  unlink(Path::socketpath.c_str());
  len = strlen(local.sun_path) + sizeof(local.sun_family);

  ret = bind(m_sockfd, (struct sockaddr *)&local, len);
  if (ret == -1)
    throw ErrnoException("Binding socket error", errno);

  ret = listen(m_sockfd, 10);
  if (ret == -1)
    throw ErrnoException("Listening socket error", errno);
}

void NotificationTalker::parseRequest(CynaraRequestPtr request)
{
  switch (request->type) {
  case RequestType::RT_Close:
    //ALOGD("Close service");
    stop();
    return;
  case RequestType::RT_Action:
    //ALOGD("Add request: " << request->id);
    addRequest(request);
    return;
  case RequestType::RT_Cancel:
    //ALOGD("Cancel request: " << request->id);
    removeRequest(request);
    return;
  default:
    return;
  }
}

void NotificationTalker::addRequest(CynaraRequestPtr request)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  auto &queue = requests[request->user];
  auto it = std::find_if(
        queue.begin(), queue.end(), [&request](CynaraRequestPtr req){return req->id == request->id;}
      );

  if (it == queue.end()) {
    queue.push_back(request);
  } else {
    //ALOGD("Cynara request already exists");
  }

}

void NotificationTalker::removeRequest(CynaraRequestPtr request)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  for (auto &pair : requests) {
    auto &queue = std::get<1>(pair);
    auto it = std::find_if(
          queue.begin(), queue.end(), [&request](CynaraRequestPtr req){return req->id == request->id;}
        );

    if (it != queue.end()) {
      if (it == queue.begin()) {
        auto user = std::get<0>(pair);
        auto it2 = m_userToFd.find(user);
        if (it2 != m_userToFd.end())
          sendDismiss(std::get<1>(*it2));
      }

      queue.erase(it);
    }
  }
}

void NotificationTalker::setResponseHandler(ResponseHandler responseHandler)
{
  m_responseHandler = responseHandler;
}

void NotificationTalker::stop()
{
  m_stopflag = true;

  for (auto& pair : m_fdStatus) {
    int fd = std::get<0>(pair);
    close(fd);
  }

  m_fdStatus.clear();
  m_fdToUser.clear();
  m_userToFd.clear();

  close(m_sockfd);
  m_sockfd = 0;
}

NotificationTalker::~NotificationTalker()
{
  for (auto& pair : m_fdStatus) {
    int fd = std::get<0>(pair);
    if (fd)
      close(fd);
  }

  if (m_sockfd)
    close(m_sockfd);
}

void NotificationTalker::sendRequest(int fd, const CynaraRequestPtr request)
{
  int size;
  int len;

  m_fdStatus[fd] = false;

  std::string data = notificationRequestToData(request->id, request->app, request->perm);
  size = data.size();

  len = send(fd, &size, sizeof(size), 0);
  if (len <= 0)
    throw ErrnoException("Error sending data to socket", errno);

  len = send(fd, data.c_str(), size, 0);
  if (len <= 0)
    throw ErrnoException("Error sending data to socket", errno);
}

void NotificationTalker::sendDismiss(int fd)
{
  if (!m_fdStatus[fd]) {
    send(fd, &Protocol::dissmisCode, sizeof(Protocol::dissmisCode), 0);
    m_fdStatus[fd] = true;
  }
}

void NotificationTalker::parseResponse(Response response, int fd)
{
  CynaraRequestPtr request = nullptr;

  /* lock_guard */ {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto &queue = requests[m_fdToUser[fd]];
    if (queue.empty()) {
      //ALOGD("Request canceled");
      m_fdStatus[fd] = true;
      return;
    }

    request = queue.front();
    if (request->id != response.id) {
      //ALOGD("Request canceled");
      m_fdStatus[fd] = true;
      return;
    }

    queue.pop_front();
  } /* lock_guard */

  /*ALOGD("For user: <" << request->user << "> client: <" << request->app << "> permision: <" <<
       request->perm << "> recieved: <" << GuiResponseToString(response.response) << ">"); */
  m_responseHandler(response);

  send(fd, &Protocol::ackCode, sizeof(Protocol::ackCode), 0);

  m_fdStatus[fd] = true;
}

void NotificationTalker::recvResponses()
{
  for (auto pair : m_userToFd) {
    int fd = std::get<1>(pair);

    if (FD_ISSET(fd, &m_fdSet)) {
      Response response;

      int len = recv(fd, &response, sizeof(response), 0);
      if (len < 0) {
        throw ErrnoException("Error reciving data from socket", errno);
      } else if (len) {
        parseResponse(response, fd);
      } else {
        remove(fd);
      }
    }
  }
}

void NotificationTalker::newConnection()
{
  if (FD_ISSET(m_sockfd, &m_fdSet)) {
    int fd;
    sockaddr_un remote;
    socklen_t t = sizeof(remote);

    fd = accept(m_sockfd, (sockaddr*)&remote, &t);
    if (fd < 0)
      throw ErrnoException("Accepting socket error", errno);

    char *user_c = nullptr;

    cynara_creds_socket_get_user(fd, USER_METHOD_DEFAULT,&user_c);
    std::string user = user_c ? user_c : "0";

    auto it = m_userToFd.find(user);
    if (it != m_userToFd.end())
      remove(std::get<1>(*it));

    m_userToFd[user] = fd;
    m_fdToUser[fd] = user;
    m_fdStatus[fd] = true;

    free(user_c);

    //ALOGD("Accepted new conection for user: " << user);
  }
}

void NotificationTalker::remove(int fd)
{
  //ALOGE("Close sock " << fd);
  close(fd);
  auto user = m_fdToUser[fd];
  m_fdToUser.erase(fd);
  m_userToFd.erase(user);
  m_fdStatus.erase(fd);
}

void NotificationTalker::run()
{
  //ALOGD("Notification loop started");
  while (!m_stopflag) {

    timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000*100;

    int nfds = m_sockfd;

    FD_ZERO(&m_fdSet);
    FD_SET(m_sockfd, &m_fdSet);

    for (auto pair : m_userToFd) {
      int fd = std::get<1>(pair);
      FD_SET(fd ,&m_fdSet);
      nfds = fd > nfds ? fd : nfds;
    }

    int rv = select(nfds + 1, &m_fdSet, nullptr, nullptr, &timeout);

    if (m_stopflag)
      break;

    if (rv < 0) {
      throw ErrnoException("error on select", errno);
    } else if (rv) {
      recvResponses();
      newConnection();
    } else {
      // timeout
    }

    /* lock_guard */ {
      std::lock_guard<std::mutex> lock(m_mutex);
      for (auto pair : m_fdStatus ) {
        int fd = std::get<0>(pair);
        bool b = std::get<1>(pair);
        auto &queue = requests[m_fdToUser[fd]];
        if(b && !queue.empty()) {
          CynaraRequestPtr request = queue.front();
          sendRequest(fd, request);
        }
      }
    } /* lock_guard */
  }

  //ALOGD("NotificationTalker loop ended");
}

} /* namespace Daemon */

} /* namespace AskUser */
