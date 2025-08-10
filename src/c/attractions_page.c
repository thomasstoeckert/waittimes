#include "attractions_page.h"

static Window *s_attractions_browse_window;
static MenuLayer *s_attractions_menu_layer;

void refresh_attractions_display()
{
    if(s_attractions_browse_window && s_attractions_menu_layer)
    {
        menu_layer_set_selected_index(s_attractions_menu_layer, MenuIndex(0, 0), MenuRowAlignTop, false);
        menu_layer_reload_data(s_attractions_menu_layer);
    }

    if(get_attractions_count() > 0)
    {
        window_attractions_push();
    }
}

void select_attraction_row_callback(MenuLayer *s_menu_layer, MenuIndex *cell_index, void *callback_context)
{
    // do something
    APP_LOG(APP_LOG_LEVEL_ERROR, "BOoP!");
}

uint16_t get_attractions_row_count(MenuLayer *s_menu_layer, uint16_t section_index, void *callback_context)
{
    return get_attractions_count();
}

void draw_attraction_row_handler(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context)
{
    int index = cell_index -> row;
    int attraction_count = get_attractions_count();

    if(index < 0 || index >= attraction_count)
    {
        APP_LOG(APP_LOG_LEVEL_WARNING,
            "[AP.C] An out of range menu index of %d [0, %d) was asked to be drawn. Drawing nothing.",
            index, attraction_count);
        return;
    }

    AttractionData attraction_data;
    get_attraction_at_index(index, &attraction_data);

    menu_cell_basic_draw(ctx, cell_layer, 
        attraction_data.attraction_name, attraction_data.attraction_status, NULL);
}

void page_attractions_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    s_attractions_menu_layer = menu_layer_create(bounds);
    menu_layer_set_callbacks(s_attractions_menu_layer, NULL, (MenuLayerCallbacks)
        {
           .get_num_rows = get_attractions_row_count,
           .draw_row     = draw_attraction_row_handler,
           .select_click = select_attraction_row_callback
        });
    
    menu_layer_set_click_config_onto_window(s_attractions_menu_layer, window);
    menu_layer_set_highlight_colors(s_attractions_menu_layer, COLOR_AP_ACTIVE, COLOR_AP_INACTIVE);

    layer_add_child(window_layer, menu_layer_get_layer(s_attractions_menu_layer));
}

void page_attractions_unload(Window *window)
{
    menu_layer_destroy(s_attractions_menu_layer);
}

void window_attractions_init(void)
{
    s_attractions_browse_window = window_create();
    window_set_window_handlers(
        s_attractions_browse_window, (WindowHandlers)
        {
            .load = page_attractions_load,
            .unload = page_attractions_unload
        }
    );
}

void window_attractions_deinit(void)
{
    window_destroy(s_attractions_browse_window);
}

void window_attractions_push(void)
{
    if(!s_attractions_browse_window)
    {
        window_attractions_init();
    }

    window_stack_push(s_attractions_browse_window, true);
}

void window_attractions_remove(void)
{
    window_stack_remove(s_attractions_browse_window, true);
}
