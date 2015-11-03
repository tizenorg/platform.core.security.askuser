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
 * @file        popup.cpp
 * @author      Janusz Kozerski (j.kozerski@samsung.com)
 * @version     1.0
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <libintl.h>
#include <sys/select.h>
#include <time.h>

#include <Elementary.h>

#include <log/alog.h>
#include "popup.h"
#include "popup-runner.h"
#include "serialization.h"

using namespace AskUser::Agent;

namespace { // anonymous

void on_done(void) {
    // Quit the efl-mainloop
    ALOGD("elm_exit()");
    elm_exit();
}

void allow_answer(void *data, Evas_Object * /* obj */, void * /* event_info */) {

    ALOGD("allow_answer");
    if(NULL == data){
        ALOGE("data is NULL; return");
        return;
    }
    struct cert_checker_popup_data *pdp = static_cast <struct cert_checker_popup_data *> (data);
    if (elm_check_state_get(pdp->check_life)) {
        ALOGD("remember_life is set");
        pdp->result = UIResponseType::URT_YES_LIFE;
    }
    else if (elm_check_state_get(pdp->check_session)) {
        ALOGD("remember_session is set");
        pdp->result = UIResponseType::URT_YES_SESSION;
    }
    else {
        ALOGD("No flag is set");
        pdp->result = UIResponseType::URT_YES_ONCE;
    }

    on_done();
}

void deny_answer(void *data, Evas_Object * /* obj */, void * /* event_info */) {

    ALOGD("deny_answer");
    if(NULL == data){
        ALOGE("data is NULL; return");
        return;
    }
    struct cert_checker_popup_data *pdp = static_cast <struct cert_checker_popup_data *> (data);
    if (elm_check_state_get(pdp->check_life)) {
        ALOGD("remember_life is set");
        pdp->result = UIResponseType::URT_NO_LIFE;
    }
    else if (elm_check_state_get(pdp->check_session)) {
        ALOGD("remember_session is set");
        pdp->result = UIResponseType::URT_NO_SESSION;
    }
    else {
        ALOGD("No flag is set");
        pdp->result = UIResponseType::URT_NO_ONCE;
    }

    on_done();
}

int show_popup(struct cert_checker_popup_data *pdp) {
    ALOGD("show_popup()");

    if(NULL == pdp){
        ALOGE("pdp is NULL; return");
        return 1;
    }

    pdp->win = elm_win_add(NULL,
            dgettext(PROJECT_NAME, "SID_PRIVILEGE_REQUEST_DIALOG_TITLE"),
            ELM_WIN_NOTIFICATION);

    elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
    elm_win_autodel_set(pdp->win, EINA_TRUE);
    evas_object_show(pdp->win);
    elm_win_indicator_opacity_set(pdp->win, ELM_WIN_INDICATOR_TRANSLUCENT);

    pdp->popup = elm_popup_add(pdp->win);

    pdp->box = elm_box_add(pdp->popup);
    evas_object_size_hint_weight_set(pdp->box, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(pdp->box, EVAS_HINT_FILL, 0.0);

    pdp->title = elm_label_add(pdp->popup);
    elm_object_style_set(pdp->title, "elm.text.title");
    elm_object_text_set(pdp->title, dgettext(PROJECT_NAME, "SID_PRIVILEGE_REQUEST_DIALOG_TITLE"));
    evas_object_show(pdp->title);
    elm_box_pack_end(pdp->box, pdp->title);

    pdp->content = elm_label_add(pdp->popup);
    elm_object_style_set(pdp->content, "elm.swallow.content");
    elm_label_line_wrap_set(pdp->content, ELM_WRAP_MIXED);

    int ret;
    char *messageFormat = dgettext(PROJECT_NAME, "SID_PRIVILEGE_REQUEST_DIALOG_MESSAGE");
    char tmpBuffer[BUFSIZ];
    ret = std::snprintf(tmpBuffer, sizeof(tmpBuffer), messageFormat,
            pdp->client.c_str(),
            pdp->user.c_str(),
            pdp->privilege.c_str());

    if (ret < 0) {
        int erryes = errno;
        ALOGE("sprintf failed with error: <" << strerror(erryes) << ">");
        return 1;
    }

    elm_object_text_set(pdp->content, tmpBuffer);
    ALOGD("Popup label: " << tmpBuffer);
    evas_object_size_hint_weight_set(pdp->content, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(pdp->content, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(pdp->content);
    elm_box_pack_end(pdp->box, pdp->content);

    elm_object_part_content_set(pdp->popup, "default", pdp->box);

    pdp->keep_button = elm_button_add(pdp->popup);
    elm_object_style_set(pdp->keep_button, "elm.swallow.content.button1");
    elm_object_text_set(pdp->keep_button, dgettext(PROJECT_NAME, "SID_PRIVILEGE_REQUEST_DIALOG_BUTTON_YES_ONCE"));
    elm_object_part_content_set(pdp->popup, "button1", pdp->keep_button);
    evas_object_smart_callback_add(pdp->keep_button, "clicked", allow_answer, pdp);

    pdp->uninstall_button = elm_button_add(pdp->popup);
    elm_object_style_set(pdp->uninstall_button, "elm.swallow.content.button2");
    elm_object_text_set(pdp->uninstall_button, dgettext(PROJECT_NAME, "SID_PRIVILEGE_REQUEST_DIALOG_BUTTON_NO_ONCE"));
    elm_object_part_content_set(pdp->popup, "button2", pdp->uninstall_button);
    evas_object_smart_callback_add(pdp->uninstall_button, "clicked", deny_answer, pdp);

    pdp->check_session = elm_check_add(pdp->box);
    elm_object_style_set(pdp->check_session, "check");
    elm_object_text_set(pdp->check_session, dgettext(PROJECT_NAME, "SID_PRIVILEGE_REQUEST_DIALOG_CHECKBOX_REMEMBER_SESSION"));
    evas_object_show(pdp->check_session);
    elm_box_pack_end(pdp->box, pdp->check_session);

    pdp->check_life = elm_check_add(pdp->box);
    elm_object_style_set(pdp->check_life, "check");
    elm_object_text_set(pdp->check_life, dgettext(PROJECT_NAME, "SID_PRIVILEGE_REQUEST_DIALOG_CHECKBOX_REMEMBER_LIFE"));
    evas_object_show(pdp->check_life);
    elm_box_pack_end(pdp->box, pdp->check_life);

    evas_object_show(pdp->popup);

    // Showing the popup window
    evas_object_show(pdp->win);

    // Run the efl mainloop
    elm_run();

    // Shutdown elementary
    ALOGD("elm_shutdown()");
    elm_shutdown();

    return 0;
}

int wait_for_parent_info (int pipe_in)
{
    // wait for parameters from pipe_in
    // timeout is set for 10 seconds
    struct timeval timeout = {10L, 0L};
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(pipe_in, &readfds);

    int sel = select(pipe_in + 1, &readfds, NULL, NULL, &timeout);
    if (sel == -1) {
        ALOGE("Cannot get info from parent. Exit popup - ERROR (" << errno << ")");
        close (pipe_in);
        return -1;
    }
    else if (sel == 0) {
        ALOGE("Timeout reached! Exit popup - ERROR");
        close (pipe_in);
        return -1;
    }
    return 0;
}

void deserialize (cert_checker_popup_data *pdp, char *line, ssize_t line_length)
{
    ALOGD("------- Deserialization -------");
    BinaryStream stream;
    stream.Write(line_length, static_cast <void *> (line));

    // Deserialization order:
    // client, user, privilege

    AskUser::Deserialization::Deserialize(stream, pdp->client);
    ALOGD("client : " << pdp->client.c_str());

    AskUser::Deserialization::Deserialize(stream, pdp->user);
    ALOGD("user : " << pdp->user.c_str());

    AskUser::Deserialization::Deserialize(stream, pdp->privilege);
    ALOGD("privilege : " << pdp->privilege.c_str());
}

} // anonymous

EAPI_MAIN int
elm_main(int argc, char **argv)
{
    // int pipe_in and int pipe_out should be passed to Popup via args.

    // These parameters should be passed to Popup via pipe_in:
    // std::string    client
    // std::string    user
    // std::string    privilege

    struct cert_checker_popup_data pd;
    struct cert_checker_popup_data *pdp = &pd;

    ALOGD("############################ popup binary ################################");

    setlocale(LC_ALL, "");

    if (argc < 3) {
        ALOGE("To few args passed in exec to popup-bin - should be at least 3:");
        ALOGE("(binary-name, pipe_in, pipe_out)");
        ALOGE("return ERROR");
        return popup_status::EXIT_ERROR;
    }

    ALOGD("Passed args: " << argv[0] <<", " << argv[1] << ", " << argv[2]);

    int pipe_in;
    int pipe_out;

    // Parsing args (pipe_in, pipe_out)
    if ( 0 == sscanf(argv[1], "%d", &pipe_in) ){
        ALOGE("Error while parsing pipe_in; return ERROR");
        return popup_status::EXIT_ERROR;
    }
    if ( 0 == sscanf(argv[2], "%d", &pipe_out) ){
        ALOGE("Error while parsing pipe_out; return ERROR");
        return popup_status::EXIT_ERROR;
    }
    ALOGD("Parsed pipes: IN: " << pipe_in <<", OUT: " <<  pipe_out);

    if (wait_for_parent_info(pipe_in) == -1) {
        close(pipe_out);
        return popup_status::EXIT_ERROR;
    }

    int  buff_size = 1024;
    char line[buff_size];

    ssize_t count = 0;

    do {
        count = TEMP_FAILURE_RETRY(read(pipe_in, line, buff_size));
    } while (0 == count);
    if(count < 0){
        close(pipe_in);
        close(pipe_out);
        ALOGE("read returned a negative value (" << count <<")");
        ALOGE("errno: " << strerror( errno ) );
        ALOGE("Exit popup - ERROR");
        return popup_status::EXIT_ERROR;
    }
    ALOGD("Read bytes : " << count);
    close(pipe_in); // cleanup

    deserialize(pdp, line, count);

    pdp->result = UIResponseType::URT_ERROR;

    if (show_popup(pdp))
        return popup_status::EXIT_ERROR; // Showing popup

    // sending validation_result to popup-runner
    BinaryStream stream_out;

    ALOGD("pdp->result : " << pdp->result);
    AskUser::Serialization::Serialize(stream_out, pdp->result);
    if(-1 == TEMP_FAILURE_RETRY(write(pipe_out, stream_out.char_pointer(), stream_out.size()))){
        ALOGE("Write to pipe failed!");
        close(pipe_out);
        return popup_status::EXIT_ERROR;
    }

    close(pipe_out);

    ALOGD("############################ /popup binary ################################");
    ALOGD("Return: " << popup_status::NO_ERROR);
    return popup_status::NO_ERROR;
}
ELM_MAIN()
