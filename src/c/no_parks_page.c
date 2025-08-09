#include "no_parks_page.h"

static Window *s_no_parks_window;
static TextLayer *s_no_parks_header;
static TextLayer *s_no_parks_body;

static PreferredContentSize s_content_size;

static GFont header_font;

void page_noparks_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    // Background color
    window_set_background_color(s_no_parks_window, COLOR_NOPARKS_BG);

    // General layout things:

    // We want the header to be in the top center of the thing, and the 
    // body to be beneath that.
    // They'll both be the size of the window, with a couple px offset from
    // the corners

    // I'm thinking the header takes up the top third of the window,
    // with the body taking up a second third. Use the remaining third for
    // headroom/footroom.

    int screen_width = bounds.size.w;
    int screen_height = bounds.size.h;

    int margined_width = screen_width - (NOPARKS_MARGIN * 2);

    int third_height = screen_height / 3;
    int sixth_height = third_height / 2;

    GRect header_space = GRect(NOPARKS_MARGIN, NOPARKS_MARGIN + (sixth_height / 2), margined_width, 32);
    GRect body_space   = GRect(NOPARKS_MARGIN, header_space.origin.y + 40, margined_width, third_height + sixth_height);

    // Header
    s_no_parks_header = text_layer_create(header_space);
    text_layer_set_text_color(s_no_parks_header, GColorWhite);
    text_layer_set_background_color(s_no_parks_header, GColorClear);
    text_layer_set_font(s_no_parks_header, header_font);
    text_layer_set_text(s_no_parks_header, "Welcome!");
    text_layer_set_text_alignment(s_no_parks_header, GTextAlignmentCenter);
    
    // Body
    s_no_parks_body = text_layer_create(body_space);
    text_layer_set_text_color(s_no_parks_body, GColorWhite);
    text_layer_set_background_color(s_no_parks_body, GColorClear);
    text_layer_set_font(s_no_parks_body, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text(s_no_parks_body, "Open the 'Settings' page on your phone to get started");
    text_layer_set_text_alignment(s_no_parks_body, GTextAlignmentCenter);

    layer_add_child(window_layer, text_layer_get_layer(s_no_parks_header));
    layer_add_child(window_layer, text_layer_get_layer(s_no_parks_body));
}

void page_noparks_unload(Window *window)
{
    // Destroy text layers
    text_layer_destroy(s_no_parks_header);
    text_layer_destroy(s_no_parks_body);
}

void window_no_parks_init(void)
{
    s_content_size = preferred_content_size();
    header_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_CUSTOM_FONT_DROID_SERIF_24_BOLD));

    s_no_parks_window = window_create();
    window_set_window_handlers(
        s_no_parks_window, (WindowHandlers)
        {
            .load = page_noparks_load,
            .unload = page_noparks_unload
        }
    );
}

void window_no_parks_deinit(void)
{
    window_destroy(s_no_parks_window);
}

void window_no_parks_push(void)
{
    if(!s_no_parks_window)
    {
        window_no_parks_init();
    }

    window_stack_push(s_no_parks_window, true);
}

void window_no_parks_pop(void)
{
    window_stack_remove(s_no_parks_window, true);
}
