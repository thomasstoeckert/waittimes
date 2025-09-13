#include "configuration.h"

ClaySettings clay_settings;

void persist_clay_default_settings()
{
    clay_settings.show_empty = false;
    clay_settings.show_valid_data_only = false;
    clay_settings.show_show_times = true;
}

void persist_clay_load_settings()
{
    // Set default clay_settings. This allows for "upgrading" setting information
    // if things change.
    persist_clay_default_settings();
    // Override default clay_settings with our saved data
    persist_read_data(SETTINGS_KEY, &clay_settings, sizeof(clay_settings));
}

void persist_clay_save_settings()
{
    persist_write_data(SETTINGS_KEY, &clay_settings, sizeof(clay_settings));
}
