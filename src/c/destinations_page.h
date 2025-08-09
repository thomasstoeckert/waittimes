#pragma once

#include <pebble.h>
#include "destinations.h"

#define COLOR_ACTIVE GColorBulgarianRose
#define COLOR_INACTIVE GColorWhite

#define DESTINATIONS_SERIAL_KEY_BASE 2

void destination_selected(int destination_idx);
void refresh_destinations_display();
void select_destination_callback(struct MenuLayer *s_menu_layer, 
    MenuIndex *cell_index, void *callback_context);
uint16_t get_destinations_row_count(struct MenuLayer *s_menu_layer,
    uint16_t section_index, void *callback_context);
void draw_destination_row_handler(GContext *ctx, const Layer *cell_layer,
    MenuIndex *cell_index, void *callback_context);
void page_destinations_load(Window *window);
void page_destinations_unload(Window *window);
void window_destinations_init(void);
void window_destinations_deinit(void);
void window_destinations_push(void);