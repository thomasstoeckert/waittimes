#pragma once

#include <pebble.h>

#define COLOR_NOPARKS_BG GColorBulgarianRose

#define NOPARKS_MARGIN 8

void window_no_parks_init(void);
void window_no_parks_deinit(void);
void window_no_parks_push(void);
void window_no_parks_remove(void);