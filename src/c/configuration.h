#pragma once

#include <pebble.h>

#define SETTINGS_KEY 3

typedef struct ClaySettings
{
    bool show_empty;
    bool show_show_times;
    bool show_valid_data_only;
} ClaySettings;

ClaySettings clay_settings;

void persist_clay_default_settings();
void persist_clay_load_settings();
void persist_clay_save_settings();