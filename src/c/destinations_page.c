#include "destinations_page.h"

static Window *s_destinations_browse_window;
static MenuLayer *s_destinations_menu_layer;

void destination_selected(int destination_idx) 
{
    // Issue request to the phone to get destination data
}

void refresh_destinations_display()
{
    if(s_destinations_browse_window && s_destinations_menu_layer)
    {
        // Set the index of the parks menu to zero, so we're not selecting an
        // invalid park index
        menu_layer_set_selected_index(s_destinations_menu_layer, MenuIndex(0, 0), MenuRowAlignTop, false);
        // Reload the data in the parks menu
        menu_layer_reload_data(s_destinations_menu_layer);
    }

    if(get_park_count() > 0)
    {
        window_destinations_push();
        window_no_parks_remove();
    }
    else
    {
        window_destinations_remove();
        window_no_parks_push();
    }
}

void select_destination_callback(struct MenuLayer *s_menu_layer, 
    MenuIndex *cell_index, void *callback_context)
{
    int index = cell_index->row;
    int max_parkcount = get_park_count();
    // Validate that our selected index is not out of bounds
    if(index < 0 || index >= max_parkcount)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, 
            "[DP.H] An out of range menu index has been selected (%d)[0, %d)", 
            index, max_parkcount);
        return;
    }
    ParkData park_data;
    get_data_for_park_index(index, &park_data);

    APP_LOG(APP_LOG_LEVEL_INFO, "Destination (%d)%s has been clicked", index, park_data.park_name);

    // Send the request
    destination_selected(index);

    // TODO: Loading window
}

uint16_t get_destinations_row_count(struct MenuLayer *s_menu_layer,
    uint16_t section_index, void *callback_context)
{
    return get_park_count();
}

void draw_destination_row_handler(GContext *ctx, const Layer *cell_layer,
    MenuIndex *cell_index, void *callback_context)
{
    int index = cell_index->row;
    int park_count = get_park_count();

    if(index < 0 || index >= park_count)
    {
        APP_LOG(APP_LOG_LEVEL_WARNING, 
            "[DP.H] An out of range menu index was asked to be drawn. Drawing nothing");
        return;
    }

    ParkData park_data;
    get_data_for_park_index(index, &park_data);

    menu_cell_basic_draw(ctx, cell_layer,
        park_data.park_name, park_data.destination_name, NULL);
}

void page_destinations_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Create our menu layer, assign handlers
    s_destinations_menu_layer = menu_layer_create(bounds);
    menu_layer_set_callbacks(s_destinations_menu_layer, NULL,
        (MenuLayerCallbacks){
            .get_num_rows = get_destinations_row_count,
            .draw_row     = draw_destination_row_handler,
            .select_click = select_destination_callback
        });
    
    // Bind the window's input functionality
    menu_layer_set_click_config_onto_window(s_destinations_menu_layer, window);

    // Style it!
    menu_layer_set_highlight_colors(s_destinations_menu_layer, COLOR_ACTIVE, COLOR_INACTIVE);

    // Something
    layer_add_child(window_layer, menu_layer_get_layer(s_destinations_menu_layer));
}

void page_destinations_unload(Window *window)
{
    menu_layer_destroy(s_destinations_menu_layer);
}

void window_destinations_init(void)
{
    s_destinations_browse_window = window_create();
    window_set_window_handlers(
        s_destinations_browse_window, (WindowHandlers){
            .load   = page_destinations_load,
            .unload = page_destinations_unload
        }
    );
}

void window_destinations_deinit(void)
{
    window_destroy(s_destinations_browse_window);
}

void window_destinations_push(void)
{
    if(!s_destinations_browse_window)
    {
        window_destinations_init();
    }

    if(get_park_count() > 0)
    {
        window_no_parks_remove();
        window_stack_push(s_destinations_browse_window, true);
    }
    else
    {
        window_no_parks_push();
    }
}

void window_destinations_remove(void)
{
    window_stack_remove(s_destinations_browse_window, true);
}
