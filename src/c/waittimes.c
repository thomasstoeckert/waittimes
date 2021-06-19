#include <pebble.h>

static Window *s_parks_browse_window, *s_attraction_list_window;
static MenuLayer *s_parks_menu_layer, *s_attractions_menu_layer;

typedef struct
{
  char name[32];
  int subtitleID;
  int id;
} Park;

const char* const park_groups[] = {
  "Walt Disney World",
  "Universal Orlando",
  "Disneyland Resort",
  "Tokyo Disney Resort",
  "Disneyland Paris Resort"
};

const Park park_array[] =
    {
        // Park name is human-readable display text,
        // ID is the index in the javascript array
        /* --- Walt Disney World --- */
        {"Magic Kingdom", 0, 0},
        {"EPCOT", 0, 1},
        {"Hollywood Studios", 0, 2},
        {"Animal Kingdom", 0, 3},
        /* --- Universal Orlando --- */
        {"Universal Florida", 1, 16},
        {"Islands of Adventure", 1, 14},
        {"Volcano Bay", 1, 17},
        /* --- Disneyland --- */
        {"Disneyland", 2, 4},
        {"Disney's California Adventure", 2, 5},
        /* --- Disney (Abroad) --- */
        {"Tokyo Disneyland", 3, 11},
        {"Tokyo DisneySea", 3, 12},
        {"Disneyland Paris", 4, 8},
        {"Walt Disney Studios Paris", 4, 7},
        {"Hong Kong Disneyland", -1, 9},
        {"Shanghai Disneyland", -1, 10},
        /* --- Other --- */
        {"Universal Hollywood", -1, 13},
        {"Europa Park", -1, 15},
        {"Efteling", -1, 6},
};

static int s_selected_park_index;

static bool s_are_attractions_loading = false;

static char s_attraction_names[100][128];
static char s_attraction_status[100][16];
static int s_num_attractions;

/*
 * --- App Communication / Messaging Functionality ---
 *
 */
static void inbox_received_callback(DictionaryIterator *iter, void *context)
{
  // A new message has been successfully received.
  APP_LOG(APP_LOG_LEVEL_INFO, "New message received.");

  Tuple *count_tuple = dict_find(iter, MESSAGE_KEY_i_attractionCount);
  if (count_tuple)
  {
    // If we have any number of attractions, we have an attraction response

    // We do want to double-check that we're not loading something by accident.
    if (!s_are_attractions_loading)
      return;

    // We need to parse ALL of the attractions. That's a lot, but we should be able to handle it.
    s_num_attractions = count_tuple->value->int32;
    for (int i = 0; i < s_num_attractions; i++)
    {
      Tuple *name_tuple = dict_find(iter, MESSAGE_KEY_i_attractionNameString + i);
      Tuple *status_tuple = dict_find(iter, MESSAGE_KEY_i_attractionStatus + i);

      if (!(name_tuple && status_tuple))
      {
        // We can't find all the information for this attraction. Something
        //  went wrong in the communication process.
        APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to find all attraction information at index %d", i);
        continue;
      }

      // Get this data into the relevant array
      strcpy(s_attraction_names[i], name_tuple->value->cstring);
      strcpy(s_attraction_status[i], status_tuple->value->cstring);
    }

    // We have all of our attraction information
    s_are_attractions_loading = false;
    APP_LOG(APP_LOG_LEVEL_INFO, "Parsed %d attractions sent from phone", s_num_attractions);

    // Trigger the loading of the attractions page
    window_stack_push(s_attraction_list_window, true);
  }
}

static void
select_park_callback(struct MenuLayer *s_menu_layer, MenuIndex *cell_index,
                     void *callback_context)
{

  APP_LOG(APP_LOG_LEVEL_INFO, "Park %s has been clicked", park_array[cell_index->row].name);

  // Issue request to javascript
  DictionaryIterator *out_iter;
  s_are_attractions_loading = true;
  AppMessageResult result = app_message_outbox_begin(&out_iter);

  if (result == APP_MSG_OK)
  {
    // Construct the message
    int parkID = park_array[cell_index->row].id;
    s_selected_park_index = cell_index->row;
    dict_write_int(out_iter, MESSAGE_KEY_o_parkID, &parkID, sizeof(int), true);

    result = app_message_outbox_send();

    if (result != APP_MSG_OK)
    {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    }
    else
    {
      APP_LOG(APP_LOG_LEVEL_INFO, "Message Sent");
    }
  }
  else
  {
    // Outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }

  // Set the state of the park window to "loading"

  // Push the park window
}

static uint16_t get_parks_row_count(struct MenuLayer *menulayer, uint16_t section_index, void *callback_context)
{
  int count = sizeof(park_array) / sizeof(Park);
  return count;
}

static void draw_parks_row_handler(GContext *ctx, const Layer *cell_layer,
                                   MenuIndex *cell_index, void *callback_context)
{
  const char *name = park_array[cell_index->row].name;
  int subtitleID = park_array[cell_index->row].subtitleID;
  const char *subtitle = NULL;
  if(subtitleID != -1) {
    subtitle = park_groups[subtitleID];
  }
  menu_cell_basic_draw(ctx, cell_layer, name, subtitle, NULL);
}

static void parks_browse_load(Window *window)
{
  // Get the root layer / bounds for this window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the menu layer, assign handlers,
  s_parks_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_parks_menu_layer, NULL,
                           (MenuLayerCallbacks){.get_num_rows = get_parks_row_count,
                                                .draw_row = draw_parks_row_handler,
                                                .select_click = select_park_callback});
  // Bind the window's input functionality onto the menu
  menu_layer_set_click_config_onto_window(s_parks_menu_layer, window);

  menu_layer_set_highlight_colors(s_parks_menu_layer, GColorBulgarianRose, GColorWhite);

  // Add the menu layer as a child of the main window
  layer_add_child(window_layer, menu_layer_get_layer(s_parks_menu_layer));
}

static void parks_browse_unload(Window *window)
{
  menu_layer_destroy(s_parks_menu_layer);
}

static uint16_t get_attractions_row_count(struct MenuLayer *menulayer, uint16_t section_index, void *callback_context)
{
  return s_num_attractions;
}

static void draw_attractions_row_handler(GContext *ctx, const Layer *cell_layer,
                                         MenuIndex *cell_index, void *callback_context)
{
  int i = cell_index->row;
  menu_cell_basic_draw(ctx, cell_layer, s_attraction_names[i], s_attraction_status[i], NULL);
}

static int16_t get_attractions_header_height(struct MenuLayer *menulayer, uint16_t section_index, void *callback_context)
{
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void draw_attractions_header(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context)
{
  menu_cell_basic_header_draw(ctx, cell_layer, park_array[s_selected_park_index].name);
}

static void attraction_list_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_attractions_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_attractions_menu_layer, NULL, (MenuLayerCallbacks){
                                                               .get_num_rows = get_attractions_row_count,
                                                               .draw_row = draw_attractions_row_handler,
                                                               .get_header_height = get_attractions_header_height,
                                                               .draw_header = draw_attractions_header
                                                           });
  menu_layer_set_click_config_onto_window(s_attractions_menu_layer, window);

  menu_layer_set_highlight_colors(s_attractions_menu_layer, GColorBulgarianRose, GColorWhite);

  layer_add_child(window_layer, menu_layer_get_layer(s_attractions_menu_layer));
}

static void attraction_list_unload(Window *window)
{
  menu_layer_destroy(s_attractions_menu_layer);
}

static void init(void)
{
  // Configure app message settings
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  app_message_register_inbox_received(inbox_received_callback);

  // Create the main window (browse all parks)
  s_parks_browse_window = window_create();
  window_set_window_handlers(
      s_parks_browse_window, (WindowHandlers){.load = parks_browse_load,
                                              .unload = parks_browse_unload});

  // Create the subwindow (browse a parks attractions)
  s_attraction_list_window = window_create();
  window_set_window_handlers(
      s_attraction_list_window, (WindowHandlers){.load = attraction_list_load,
                                                 .unload = attraction_list_unload});

  window_stack_push(s_parks_browse_window, false);
}

static void deinit(void)
{
  // Destroy the windows
  window_destroy(s_parks_browse_window);
  window_destroy(s_attraction_list_window);
}

int main(void)
{
  // Setup application properties
  init();

  // Perform main application loop
  app_event_loop();

  // Cleanup app
  deinit();
}
