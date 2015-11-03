/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/*
 * @file        popup.h
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 */

#include <Elementary.h>

#include <ui/AskUIInterface.h>

struct cert_checker_popup_data {
    std::string client;
    std::string user;
    std::string privilege;
    AskUser::Agent::UIResponseType result;

    Evas_Object *popup       = NULL;
    Evas_Object *win         = NULL;
    Evas_Object *box         = NULL;
    Evas_Object *title       = NULL;
    Evas_Object *content     = NULL;
    Evas_Object *keep_button = NULL;
    Evas_Object *uninstall_button = NULL;
    Evas_Object *check_session = NULL;
    Evas_Object *check_life    = NULL;
};
