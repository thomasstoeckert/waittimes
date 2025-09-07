#pragma once

#include <pebble.h>

#define I_MAX_ATTRACTIONS 100
#define I_MAX_ATTRACTION_NAME_LENGTH 20
#define I_MAX_ATTRACTION_STATUS_LENGTH 20

#define PWT_ATTRS_DEBUG false

typedef struct {
    char * attraction_name;
    char * attraction_status;
} __attribute__((__packed__)) AttractionData;

#define AttractionData(name, status) ((AttractionData){(name), (status)});

int parse_attractions_response(DictionaryIterator *iter, void *context);

void print_attractions();

int get_attractions_count();
int get_attraction_at_index(int index, AttractionData* attraction_data);