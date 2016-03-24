#pragma once

#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <sys/socket.h>
#include <thread>

#include <cynara-agent.h>
#include <cynara-plugin.h>

typedef enum {
    RT_Action,
    RT_Cancel,
    RT_Close
} RequestType;

typedef cynara_agent_req_id RequestId;

class CynaraRequest
{
public:
  RequestId m_id;
  RequestType m_type;
  Cynara::PluginData m_data;
};

class NotificationTalker
{
public:
  NotificationTalker();
  void add(int fd);
  void start();

  void addReq(CynaraRequest q);

private:
  std::map<std::string, int> fdMap;
  std::map<int, std::queue<CynaraRequest>> queues;
  std::map<RequestId, CynaraRequest> responseQueue;

  std::mutex mutex;

  std::thread m_thread;
  bool m_stopflag = false;
  fd_set fdSet;

  void run();

};

