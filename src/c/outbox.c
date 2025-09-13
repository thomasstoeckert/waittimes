#include "outbox.h"

static bool s_js_ready = false;

void outbox_set_ready(bool is_ready)
{
    s_js_ready = is_ready;
}

int outbox_send_attractions_request(char *park_uuid)
{
    if(!s_js_ready)
    {
        APP_LOG(APP_LOG_LEVEL_WARNING, "Outbox was asked to send a message before receiving a JS ready note");
        return -1;
    }

    DictionaryIterator *out_iter;
    AppMessageResult result = app_message_outbox_begin(&out_iter);

    if(result != APP_MSG_OK)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox is not ready to send a message");
        return -1 * (int) result;
    }

    dict_write_cstring(out_iter, MESSAGE_KEY_o_parkID, park_uuid);

    result = app_message_outbox_send();

    if(result != APP_MSG_OK)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox had an issue after sending the message");
        return -1 * (int) result;
    }

    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox successfully requested data for %s", park_uuid);
    return 0;
}
