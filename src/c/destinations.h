#pragma once

#include <pebble.h>

#define I_MAX_PARKS 32
#define I_MAX_PARK_NAME_LENGTH 23
#define I_PARK_UUID_LENGTH 37
#define I_MAX_DESTINATION_NAME_LENGTH I_MAX_PARK_NAME_LENGTH
#define I_MAX_DESTINATIONS I_MAX_PARKS

#define DESTINATIONS_ERROR -1

#define PERSIST_VERSION_DESTINATIONS 1
#define PERSIST_KEY_DESTINATIONS_METADATA 4
#define PERSIST_KEY_PARKS_BASE 5
#define PERSIST_KEY_DESTINATIONS_BASE PERSIST_KEY_PARKS_BASE + 8

typedef struct {
    char * park_name;
    char * park_uuid;
    char * destination_name;
} __attribute__((__packed__)) ParkData;

#define ParkData(name, uuid, destname) ((ParkData){(name), (uuid), (destname)});

typedef struct {
    int num_parks;
    int num_destinations;
    int version;
} __attribute__((__packed__)) PersistDestMetadata;

typedef struct {
    char park_uuid_persist[I_PARK_UUID_LENGTH];
    char park_name_persist[I_MAX_PARK_NAME_LENGTH];
    int park_destid_persist;
} __attribute__((__packed__)) PersistParkData;

int parse_destinations_response(DictionaryIterator *iter, void *context);

void print_destinations();

int get_park_count();

int get_data_for_park_index(int park_index, ParkData* park_data);

void persist_save_destinations_data();
void persist_load_destinations_data();
// void persist_upgrade_destinations_data();
// bool persist_has_destinations_data();