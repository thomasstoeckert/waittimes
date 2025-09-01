#pragma once

#include <pebble.h>

#define COLOR_NOATTRACTIONS_BG GColorBulgarianRose

#define NOATTRACTIONS_MARGIN 8

void window_no_attractions_init(void);
void window_no_attractions_deinit(void);
void window_no_attractions_push(void);
void window_no_attractions_remove(void);