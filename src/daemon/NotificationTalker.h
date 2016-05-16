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
 * @file        NotificationTalker.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Declaration of NotificationTalker class
 */

#ifndef __NOTIFICATION_TALKER__
#define __NOTIFICATION_TALKER__

#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <sys/socket.h>

#include <common/Types.h>

typedef std::pair<std::string, int> UserToFdPair;
typedef std::map<std::string, int> UserToFdMap;
typedef std::map<int, std::string> FdToUserMap;
typedef std::map<int, bool> FdStatus;

typedef std::map<std::string, std::deque<CynaraRequestPtr>> RequestsQueue;

typedef std::function<void(Response)> ResponseHandler;

class NotificationTalker
{
public:
  NotificationTalker();

  void parseRequest(CynaraRequestPtr request);
  void run();
  void setResponseHandler(ResponseHandler responseHandler);
  virtual void stop();

protected:
  ResponseHandler m_responseHandler;

  UserToFdMap m_userToFd;
  FdToUserMap m_fdToUser;
  FdStatus m_fdStatus;
  fd_set m_fdSet;
  int m_sockfd;

  RequestsQueue requests;
  std::mutex m_mutex;

  bool m_stopflag = false;

  void parseResponse(Response response, int fd);
  void recvResponses();

  void newConnection();
  void remove(int fd);

  virtual void addRequest(CynaraRequestPtr request);
  virtual void removeRequest(CynaraRequestPtr request);
  virtual void sendRequest(int fd, const CynaraRequestPtr request);
  virtual void sendDismiss(int fd);
};

#endif /* __NOTIFICATION_TALKER__ */

