/** License **/

#pragma once

#include <string>
#include <cstring>


enum class GuiResponse
{
  Allow,
  Deny,
  Never,
  Error
};

struct Recieve
{
  char buffor[4096];

  void get(std::string &app, std::string &perm)
  {
    int app_len = strlen(buffor);
    app = std::string(buffor);
    perm = std::string(buffor + app_len + 1);
  }

  int set(const std::string &app, const std::string &perm)
  {
    strcpy(buffor, app.c_str());
    strcpy(buffor + app.size() + 1, perm.c_str());
    return(app.size() + perm.size() + 2);
  }
};

constexpr char socketpath[] = "/run/askuserd.socket";
