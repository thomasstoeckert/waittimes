#pragma once

#include <pebble.h>

#define I_MAX_PARKS 32
#define I_MAX_PARK_NAME_LENGTH 33
#define I_PARK_UUID_LENGTH 37
#define I_MAX_DESTINATION_NAME_LENGTH I_MAX_PARK_NAME_LENGTH
#define I_MAX_DESTINATIONS I_MAX_PARKS

#define DESTINATIONS_ERROR -1

typedef struct {
    char * park_name;
    char * park_uuid;
    char * destination_name;
} __attribute__((__packed__)) ParkData;

#define ParkData(name, uuid, destname) ((ParkData){(name), (uuid), (destname)});

int parse_destinations_response(DictionaryIterator *iter, void *context);

void print_destinations();

int get_park_count();

int get_data_for_park_index(int park_index, ParkData* park_data);

