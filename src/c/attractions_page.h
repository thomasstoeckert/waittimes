#pragma once

#include <pebble.h>
#include "attractions_data.h"

#define COLOR_AP_ACTIVE GColorBulgarianRose
#define COLOR_AP_INACTIVE GColorWhite

void refresh_attractions_display();
void select_attraction_row_callback(struct MenuLayer *s_menu_layer, MenuIndex *cell_index, void * callback_context);
uint16_t get_attractions_row_count(struct MenuLayer *s_menu_layer, uint16_t section_index, void * callback_context);
void draw_attraction_row_handler(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);

void page_attractions_load(Window *window);
void page_attractions_unload(Window *window);

void window_attractions_init(void);
void window_attractions_deinit(void);
void window_attractions_push(void);
void window_attractions_remove(void);