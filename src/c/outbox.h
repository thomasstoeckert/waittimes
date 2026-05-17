#pragma once

#include <pebble.h>

void outbox_set_ready(bool is_ready);
int outbox_send_attractions_request(char * park_uuid);
void outbox_send_forgot();