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
    ALOGD("deny_answer");
    if (data == NULL) {
        ALOGE("data is NULL; return");
        return;
    }

    struct cert_checker_popup_data *pdp = static_cast <struct cert_checker_popup_data *> (data);
    ALOGD("No flag is set");
    pdp->result = UIResponseType::URT_YES_LIFE;
    on_done();
}

void deny_answer(void *data, Evas_Object * /* obj */, void * /* event_info */) {
    ALOGD("deny_answer");
    if (data == NULL) {
        ALOGE("data is NULL; return");
        return;
    }

    struct cert_checker_popup_data *pdp = static_cast <struct cert_checker_popup_data *> (data);
    ALOGD("No flag is set");
    pdp->result = UIResponseType::URT_NO_LIFE;
    on_done();
}

void deny_once_answer(void *data, Evas_Object * /* obj */, void * /* event_info */) {
    ALOGD("deny_answer");
    if (data == NULL) {
        ALOGE("data is NULL; return");
        return;
    }

    struct cert_checker_popup_data *pdp = static_cast <struct cert_checker_popup_data *> (data);
    ALOGD("No flag is set");
    pdp->result = UIResponseType::URT_NO_ONCE;
    on_done();
}

bool show_popup(struct cert_checker_popup_data *pdp) {
    ALOGD("show_popup");

    if (pdp == NULL) {
        ALOGE("pdp is NULL; return");
        return false;
    }

    Evas_Object *win = elm_win_add(NULL,
            dgettext(PROJECT_NAME, "SID_PRIVILEGE_REQUEST_DIALOG_TITLE"),
            ELM_WIN_UTILITY);

    elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
    elm_win_autodel_set(win, EINA_TRUE);
    elm_win_alpha_set(win, EINA_TRUE);

    // FIXME: unfocused event is trigered on top menu slide solution: find better one event if possible
    evas_object_smart_callback_add(win, "unfocused", deny_once_answer, pdp);

    Evas_Object *popup = elm_popup_add(win);
    elm_object_part_text_set(popup, "title,text",
                             dgettext(PROJECT_NAME, "SID_PRIVILEGE_REQUEST_DIALOG_TITLE"));

    Evas_Object *box = elm_box_add(popup);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, 0);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, 0.0);

    Evas_Object *content = elm_label_add(popup);
    elm_object_style_set(content, "elm.swallow.content");
    elm_label_line_wrap_set(content, ELM_WRAP_MIXED);

    // popup text
    int ret;
    char *messageFormat = dgettext(PROJECT_NAME, "SID_PRIVILEGE_REQUEST_DIALOG_MESSAGE_NO_NEWLINE");
    char tmpBuffer[BUFSIZ];
    ret = std::snprintf(tmpBuffer, sizeof(tmpBuffer), messageFormat, pdp->client.c_str(),
                        pdp->user.c_str(), pdp->privilege.c_str());

    if (ret < 0) {
        int erryes = errno;
        ALOGE("sprintf failed with error: <" << strerror(erryes) << ">");
        return false;
    }
    elm_object_text_set(content, tmpBuffer);
    ALOGD("Popup label: " << tmpBuffer);

    evas_object_size_hint_weight_set(content, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(content, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(content);
    elm_box_pack_end(box, content);

    elm_object_part_content_set(popup, "default", box);

    // allow
    Evas_Object *allow_button = elm_button_add(popup);
    elm_object_text_set(allow_button, "Allow");
    elm_object_part_content_set(popup, "button1", allow_button);
    evas_object_smart_callback_add(allow_button, "clicked", allow_answer, pdp);

    // never
    Evas_Object *never_button = elm_button_add(popup);
    elm_object_text_set(never_button, "Never");
    elm_object_part_content_set(popup, "button2", never_button);
    evas_object_smart_callback_add(never_button, "clicked", deny_answer, pdp);

    // deny
    Evas_Object *deny_button = elm_button_add(popup);
    elm_object_text_set(deny_button, "Deny");
    elm_object_part_content_set(popup, "button3", deny_button);
    evas_object_smart_callback_add(deny_button, "clicked", deny_once_answer, pdp);

    evas_object_show(popup);

    // Showing the popup window
    evas_object_show(win);

    // Run the efl mainloop
    elm_run();

    // Shutdown elementary
    ALOGD("elm_shutdown");
    elm_shutdown();

    return true;
}

bool wait_for_parent_info(int pipe_in) {
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
        return false;
    } else if (sel == 0) {
        ALOGE("Timeout reached! Exit popup - ERROR");
        close (pipe_in);
        return false;
    }
    return true;
}

bool deserialize(cert_checker_popup_data *pdp, char *line, size_t line_length) {
    ALOGD("------- Deserialization -------");
    if (line_length == 0) {
        ALOGE("Deserialization failed");
        return false;
    }

    BinaryStream stream;
    stream.Write(line_length, static_cast <void *> (line));

    // Deserialization order:
    // client, user, privilege

    try {
        AskUser::Deserialization::Deserialize(stream, pdp->client);
        ALOGD("client : " << pdp->client.c_str());

        AskUser::Deserialization::Deserialize(stream, pdp->user);
        ALOGD("user : " << pdp->user.c_str());

        AskUser::Deserialization::Deserialize(stream, pdp->privilege);
        ALOGD("privilege : " << pdp->privilege.c_str());
    } catch (const std::runtime_error &e) {
        ALOGE("Cannot serialize data. " << e.what());
        return false;
    }

    return true;
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
    if (sscanf(argv[1], "%d", &pipe_in) == 0) {
        ALOGE("Error while parsing pipe_in; return ERROR");
        return popup_status::EXIT_ERROR;
    }
    if (sscanf(argv[2], "%d", &pipe_out) == 0) {
        ALOGE("Error while parsing pipe_out; return ERROR");
        return popup_status::EXIT_ERROR;
    }
    ALOGD("Parsed pipes: IN: " << pipe_in <<", OUT: " <<  pipe_out);

    if (!wait_for_parent_info(pipe_in)) {
        close(pipe_out);
        return popup_status::EXIT_ERROR;
    }

    int  buff_size = 1024;
    char line[buff_size];
    int tmp;
    ssize_t count = 0;

    do {
        tmp = TEMP_FAILURE_RETRY(read(pipe_in, line + count, buff_size - count));
        if (tmp < 0) {
            close(pipe_in);
            close(pipe_out);
            ALOGE("read returned a negative value (" << count << ")");
            ALOGE("errno: " << strerror(errno));
            ALOGE("Exit popup - ERROR");
            return popup_status::EXIT_ERROR;
        }
        if (tmp > 0) {
            count += tmp;
            ALOGD("Read " << tmp << " bytes. Total: " << count);
        }
    } while (tmp != 0);

    ALOGD("Read bytes in total: " << count);
    close(pipe_in); // cleanup

    if (!deserialize(pdp, line, count))
        return popup_status::EXIT_ERROR;

    pdp->result = UIResponseType::URT_ERROR;

    if (!show_popup(pdp)) // Showing popup
        return popup_status::EXIT_ERROR;

    // sending validation_result to popup-runner
    BinaryStream stream_out;

    ALOGD("pdp->result : " << pdp->result);
    AskUser::Serialization::Serialize(stream_out, pdp->result);

    unsigned int begin = 0;
    while (begin < stream_out.size()) {
        tmp = TEMP_FAILURE_RETRY(write(pipe_out,
                stream_out.char_pointer() + begin,
                stream_out.size() - begin));
        if (tmp < 0) {
            ALOGE("Write to pipe failed!");
            close(pipe_out);
            return popup_status::EXIT_ERROR;
        }
        begin += tmp;
    }

    close(pipe_out);

    ALOGD("Return: " << popup_status::NO_ERROR);
    return popup_status::NO_ERROR;
}
ELM_MAIN()
