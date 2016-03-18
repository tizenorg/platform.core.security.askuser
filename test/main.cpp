/*
 *  Copyright (c) 2014-2015 Samsung Electronics Co.
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
 * @file        main.cpp
 * @author      Oskar Świtalski <o.switalski@samsung.com>
 * @brief       Main test file
 */

#include <iostream>
#include <cstring>

#include <cynara-admin.h>
#include <cynara-client.h>
#include <cynara-error.h>

#include <common/Exception.h>
#include <common/SupportedTypes.h>

#include <unistd.h>

cynara *cyn;
cynara_admin *admin;

void check_cynara_return(std::string func, int ret) {
  if (ret < 0) {
    static const char *time2Die = "cynara_strerror error ;)";
    static char strerror[BUFSIZ];

    std::string err = cynara_strerror(ret, strerror, sizeof(strerror)) == CYNARA_API_SUCCESS ?
          strerror : time2Die;
    throw Exception(func + ": " + err);
  }
}

int main(int n, char **argc)
{
  int ret;

  char *client = new char[100];
  char *user = new char[100];
  char *privilege = new char[100];
  char *bucket = new char[100];

  strcpy(client, "User::App::org.tizen.task-mgr");
  strcpy(user, "5001");

  if (n > 1)
    strcpy(privilege, argc[1]);
  else
    strcpy(privilege, "http://tizen.org/privilege/appmanager.kill");
  strcpy(bucket, "");

  try {

    ret = cynara_admin_initialize(&admin);
    check_cynara_return("cynara_admin_initialize", ret);

    cynara_admin_policy **policies = new cynara_admin_policy*[2];
    cynara_admin_policy policy = {bucket, client, user, privilege, AskUser::SupportedTypes::Service::ASK_USER , nullptr};
    policies[0] = &policy;
    policies[1] = nullptr;

    ret = cynara_admin_set_policies(admin, policies);
    check_cynara_return("cynara_admin_set_policies", ret);

    ret = cynara_admin_finish(admin);
    check_cynara_return("cynara_admin_finish", ret);

    ret = cynara_initialize(&cyn, nullptr);
    check_cynara_return("cynara_initialize", ret);

    for (int i = 0; i < 10; ++i) {

      ret = cynara_check(cyn, client, "session", user, privilege);
      check_cynara_return("cynara_check", ret);

      switch (ret) {
      case CYNARA_API_ACCESS_ALLOWED:
        std::cout << "Allowed" << std::endl;
        break;
      case CYNARA_API_ACCESS_DENIED:
        std::cout << "Denied" << std::endl;
        break;
      }

    }

    ret = cynara_finish(cyn);
    check_cynara_return("cynara_finish", ret);

  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  } catch (...) {
    std::cerr << "Unknown error" << std::endl;
  }

  return 0;
}
