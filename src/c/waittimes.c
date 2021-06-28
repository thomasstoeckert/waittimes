#include <pebble.h>

static Window *s_parks_browse_window, *s_attraction_list_window, *s_message_window;
static MenuLayer *s_parks_menu_layer, *s_attractions_menu_layer;
static Layer *s_message_canvas_layer;

static TextLayer *s_message_text_layer;

static GDrawCommandImage *s_pdc_generic, *s_pdc_network, *s_pdc_connection;

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

// This could technically also be offloaded to the API, but I don't want the user
// to wait twice each time they load the app (1x for loading the app, 1x for 
// loading a park's times)
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
static bool s_js_ready = false;

static int s_message_code = 0;
static char s_message_text[64] = "";

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

  Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY__ready);
  if(ready_tuple) 
  {
    s_js_ready = true;
  }

  Tuple *connection_tuple = dict_find(iter, MESSAGE_KEY_i_connectionError);
  if(connection_tuple)
  {
    // Something wrong happen during communication :(
    // Let the user know.
    s_are_attractions_loading = false;
    s_message_code = 2;
    strcpy(s_message_text, "Connection Error");
    window_stack_push(s_message_window, true);
  }

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

static void send_park_request(int parkIndex)
{
  // If javascript is not ready, do not send :)
  if(!s_js_ready) return;

  // We want to send a message to the javascript component.
  // To do so, we do the following:
  // 1. Create the payload (out_iter)
  // 2. Initialize the outbox for our sending
  DictionaryIterator *out_iter;
  AppMessageResult result = app_message_outbox_begin(&out_iter);

  if (result == APP_MSG_OK)
  {

    // 3. If the outbox is ready for us to send, we then need to prepare the payload
    int parkID = park_array[parkIndex].id;
    dict_write_int(out_iter, MESSAGE_KEY_o_parkID, &parkID, sizeof(int), true);


    // 4. Send the message via the outbox.
    result = app_message_outbox_send();

    if (result != APP_MSG_OK)
    {
      // If there was an error in sending the message, oops.
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox for %d: %d", parkID, (int)result);
      s_are_attractions_loading = false;
      s_message_code = 2;
      strcpy(s_message_text, "BT Error");
      window_stack_push(s_message_window, true);
    }
    else
    {
      // There was no error sending the message, so we can set our current state
      // to "loading"
      APP_LOG(APP_LOG_LEVEL_INFO, "Park loading request sent for %d", parkID);
      s_are_attractions_loading = true;
      // And update our selected park index (for the header on the details page)
      s_selected_park_index = parkIndex;
    }
  }
  else
  {
    // Outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox for %d: %d", parkIndex, (int)result);
    s_are_attractions_loading = false;
    s_message_code = 2;
    strcpy(s_message_text, "BT Error");
    window_stack_push(s_message_window, true);
  }
}

/*
 * --- Main Page / Park Select Interface ---
 *
 */
/// The "onclick" functionality of the main menu layer. Essentially just
/// issues the server request and opens the loading page.
static void
select_park_callback(struct MenuLayer *s_menu_layer, MenuIndex *cell_index,
                     void *callback_context)
{

  APP_LOG(APP_LOG_LEVEL_INFO, "Park %s has been clicked", park_array[cell_index->row].name);

  // Send the park load request
  send_park_request((int)cell_index->row);

  // Push the loading window
  // TODO: Loading window
}

/// A handler function that just returns the number of parks (fixed, but taken
/// from the parks array)
static uint16_t get_parks_row_count(struct MenuLayer *menulayer, uint16_t section_index, void *callback_context)
{
  int count = sizeof(park_array) / sizeof(Park);
  return count;
}

/// This function is responsible for drawing each row of the parks page - namely
/// the various titles and subtitles.
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

/// This function establishes the main menu layer's properties and functionality
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

  // Stylize the main menu layer
  menu_layer_set_highlight_colors(s_parks_menu_layer, GColorBulgarianRose, GColorWhite);

  // Add the menu layer as a child of the main window
  layer_add_child(window_layer, menu_layer_get_layer(s_parks_menu_layer));
}

static void parks_browse_unload(Window *window)
{
  menu_layer_destroy(s_parks_menu_layer);
}

/*
 * --- Secondary Page / Attraction Browse Interface ---
 *
 */

/// This is a simple handler function that returns the number of attractions in the attractions list.
static uint16_t get_attractions_row_count(struct MenuLayer *menulayer, uint16_t section_index, void *callback_context)
{
  return s_num_attractions;
}

/// This function renders a row for a given attraction (cell_index) in the attractions lists. 
static void draw_attractions_row_handler(GContext *ctx, const Layer *cell_layer,
                                         MenuIndex *cell_index, void *callback_context)
{
  int i = cell_index->row;
  menu_cell_basic_draw(ctx, cell_layer, s_attraction_names[i], s_attraction_status[i], NULL);
}

/// This function returns the default header height so the section headers render
static int16_t get_attractions_header_height(struct MenuLayer *menulayer, uint16_t section_index, void *callback_context)
{
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

/// This function draws a simple header, with the title being the name of the park.
static void draw_attractions_header(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context)
{
  menu_cell_basic_header_draw(ctx, cell_layer, park_array[s_selected_park_index].name);
}

/// This function prepares the attraction list page menu, setting callbacks, and
/// preparing the window for display.
static void attraction_list_load(Window *window)
{
  // Get the root properties for our window (layer, bounds)
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the attractions menu layer, assign callbacks for functionality
  s_attractions_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_attractions_menu_layer, NULL, (MenuLayerCallbacks){
                                                               .get_num_rows = get_attractions_row_count,
                                                               .draw_row = draw_attractions_row_handler,
                                                               .get_header_height = get_attractions_header_height,
                                                               .draw_header = draw_attractions_header
                                                           });
  // Bind user input (up/down/select) to the menu layer
  menu_layer_set_click_config_onto_window(s_attractions_menu_layer, window);

  // Stylize the menu layer
  menu_layer_set_highlight_colors(s_attractions_menu_layer, GColorBulgarianRose, GColorWhite);

  // Establish the menu layer as a child of our root layer
  layer_add_child(window_layer, menu_layer_get_layer(s_attractions_menu_layer));
}

/// A little helper function that just destroys the created window
static void attraction_list_unload(Window *window)
{
  menu_layer_destroy(s_attractions_menu_layer);
}

/*
 * --- Message Window functionality ---
 *
 */
static void message_canvas_update(Layer *layer, GContext *ctx)
{
  GPoint origin = GPoint(0, 0);

  // Draw the GDrawCommandImgae to the GContext
  // Switch based upon which message ID we have
  switch (s_message_code)
  {
  case 1:
    // No app connection
    gdraw_command_image_draw(ctx, s_pdc_connection, origin);
  case 2:
    // No internet :D
    gdraw_command_image_draw(ctx, s_pdc_network, origin);
    break;
  default:
    // Generic Response
    gdraw_command_image_draw(ctx, s_pdc_generic, origin);
    break;
  }
}

static void message_window_load(Window *window)
{
  // Get our root properties for the window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create our message page
  window_set_background_color(window, GColorBulgarianRose);
  
  // Create our canvas layer (for drawing graphics)
#if defined(PBL_ROUND)
  GRect icon_bounds = GRect(50, 50, bounds.size.w, bounds.size.h);
#else
  GRect icon_bounds = GRect(30, 30, bounds.size.w, bounds.size.h);
#endif

  s_message_canvas_layer = layer_create(icon_bounds);

  // Set the layer update proc
  layer_set_update_proc(s_message_canvas_layer, message_canvas_update);

  layer_add_child(window_layer, s_message_canvas_layer);

  // Add our text over top
#if defined(PBL_ROUND)
  GRect text_bounds = GRect(0, 125, bounds.size.w, 20);
#else
  GRect text_bounds = GRect(0, 128, bounds.size.w, 20);
#endif

  s_message_text_layer = text_layer_create(text_bounds);
  text_layer_set_text(s_message_text_layer, s_message_text);
  text_layer_set_text_alignment(s_message_text_layer, GTextAlignmentCenter);
  text_layer_set_background_color(s_message_text_layer, GColorClear);
  text_layer_set_text_color(s_message_text_layer, GColorWhite);

  layer_add_child(window_layer, text_layer_get_layer(s_message_text_layer));
}

static void message_window_unload(Window *window)
{
  text_layer_destroy(s_message_text_layer);
  layer_destroy(s_message_canvas_layer);
}

/*
 * --- Main app management functions ---
 *
 */

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
  
  // An intermediate "message" window
  s_message_window = window_create();
  window_set_window_handlers(
    s_message_window, (WindowHandlers){.load = message_window_load, .unload = message_window_unload});

  // Load our PDC assets
  s_pdc_connection = gdraw_command_image_create_with_resource(RESOURCE_ID_WATCH_DISCONNECTED);
  s_pdc_network = gdraw_command_image_create_with_resource(RESOURCE_ID_CHECK_INTERNET);
  s_pdc_generic = gdraw_command_image_create_with_resource(RESOURCE_ID_GENERIC_QUESTION);

  // Push our main menu window onto the stack.
  window_stack_push(s_parks_browse_window, false);
}

static void deinit(void)
{
  // Destroy the windows
  window_destroy(s_parks_browse_window);
  window_destroy(s_attraction_list_window);
  window_destroy(s_message_window);
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
