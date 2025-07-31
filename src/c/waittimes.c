#include <pebble.h>
#include "destinations.h"
#include "destinations_page.h"

static Window *s_attraction_list_window, *s_message_window;
static MenuLayer *s_attractions_menu_layer;
static Layer *s_message_canvas_layer;

static TextLayer *s_message_text_layer;

static GDrawCommandImage *s_pdc_generic, *s_pdc_network, *s_pdc_connection;

typedef struct
{
  char name[32];
  int subtitleID;
  int id;
} Park;

const char *const park_groups[] = {
    "Walt Disney World",
    "Universal Orlando",
    "SeaWorld Orlando",
    "Disneyland Resort",
    "Tokyo Disney Resort",
    "Disneyland Paris Resort",
    "Europa-Park",
    "PortAventura World"};

// This could technically also be offloaded to the API, but I don't want the user
// to wait twice each time they load the app (1x for loading the app, 1x for
// loading a park's times)
const Park park_array[] =
    {
        // Park name is human-readable display text,
        // ID is the index in the javascript array
        {"Disneyland Park", 3, 0},
        {"Disney's California Adventure", 3, 1},
        {"California's Great America", -1, 2},
        {"Knott's Berry Farm", -1, 3},
        {"SeaWorld San Diego", -1, 4},
        {"Universal Studios (Hollywood)", -1, 5},
        {"Magic Kingdom", 0, 6},
        {"Epcot", 0, 7},
        {"Hollywood Studios", 0, 8},
        {"Animal Kingdom", 0, 9},
        {"Typhoon Lagoon", 0, 10},
        {"Blizzard Beach", 0, 11},
        {"Universal Studios (Florida)", 1, 12},
        {"Universal Islands of Adventure", 1, 13},
        {"Volcano Bay", 1, 14},
        {"SeaWorld Orlando", 2, 15},
        {"Aquatica Orlando", 2, 16},
        {"Legoland Florida", -1, 17},
        {"Busch Gardens Tampa Bay", -1, 18},
        {"Busch Gardens Williamsburg", -1, 19},
        {"Canada's Wonderland", -1, 20},
        {"Carowinds", -1, 21},
        {"Cedar Point", -1, 22},
        {"Dollywood", -1, 23},
        {"Dorney Park", -1, 24},
        {"Hersheypark", -1, 25},
        {"Kings Dominion", -1, 26},
        {"Kings Island", -1, 27},
        {"Michigan's Adventure", -1, 28},
        {"SeaWorld San Antonio", -1, 29},
        {"Silver Dollar City", -1, 30},
        {"Valleyfair", -1, 31},
        {"Worlds of Fun", -1, 32},
        {"Alton Towers", -1, 33},
        {"Chessington World of Adventures", -1, 34},
        {"Legoland Windsor", -1, 35},
        {"Thorpe Park", -1, 36},
        {"Europa Park", 6, 37},
        {"Rulantica", 6, 38},
        {"Phantasialand", -1, 39},
        {"Heide Park", -1, 40},
        {"Walibi Holland", -1, 41},
        {"Disneyland Paris", 5, 42},
        {"Walt Disney Studios Park", 5, 43},
        {"Parc Asterix", -1, 44},
        {"Bellewaerde", -1, 45},
        {"Plopsaland De Panne", -1, 46},
        {"Holiday Park", -1, 47},
        {"Liseberg", -1, 48},
        {"Efteling", -1, 49},
        {"Attractiepark Toverland", -1, 50},
        {"Gardaland", -1, 51},
        {"PortAventura Park", 7, 52},
        {"Ferrari Land", 7, 53},
        {"Aquatic Park", 7, 54},
        {"Hong Kong Disneyland", -1, 55},
        {"Shanghai Disneyland", -1, 56},
        {"Tokyo Disneyland", 4, 57},
        {"Tokyo DisneySea", 4, 58}};

// -- User Settings Variables -- //

#define SETTINGS_KEY 2

// This array stores index pointers to the parks in the mother park_array.
// This is what is used to build the display list, with each element pointing at
// the proper park in the park_array.
static int s_selected_parks_array[59];
static int s_num_selected_parks = -1;

typedef struct ClaySettings
{
  int parkVisibility[59];
  bool showEmpty;
  bool showShowTimes;
  bool showValidDataOnly;
} ClaySettings;

static ClaySettings s_settings;

// -- Content Display Variables -- //

static int s_selected_park_index;

static bool s_are_attractions_loading = false;
static bool s_js_ready = false;

static int s_message_code = 0;
static char s_message_text[64] = "";

// -- Fetched Information -- //

static char s_attraction_names[100][128];
static char s_attraction_status[100][20];
static int s_num_attractions;

/*
 * --- App configuration functionaliy ---
 *
 */
static void default_settings()
{
  int count = sizeof(park_array) / sizeof(Park);
  for (int i = 0; i < count; i++)
  {
    s_settings.parkVisibility[i] = 1;
  }
  s_settings.showEmpty = false;
  s_settings.showValidDataOnly = false;
  s_settings.showShowTimes = true;
}

static void update_park_data()
{
  // Recalculate our selected parks
  int total_parks = sizeof(park_array) / sizeof(Park);
  s_num_selected_parks = 0;

  // If the park is visible, add its index to the selected parks array
  for (int i = 0; i < total_parks; i++)
  {
    if (s_settings.parkVisibility[i] == 1)
      s_selected_parks_array[s_num_selected_parks++] = i; // Also increment s_num_selected_parks
  }
}

static void update_display()
{
  update_park_data();

  // Kill existing error page, if exists
  window_stack_remove(s_message_window, true);

  if (s_num_selected_parks == 0)
  {
    // Show the error page
    s_message_code = 0;
    strcpy(s_message_text, "No Parks Selected");
    window_stack_push(s_message_window, true);
  }

  refresh_destinations_display();
}

static void load_settings()
{
  // load the default settings
  default_settings();
  // Read settings from persistent storage, if they exist.
  persist_read_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
}

static void save_settings()
{
  persist_write_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
  // Update the display based on the new settings
  update_display();
}

/*
 * --- App Communication / Messaging Functionality ---
 *
 */
static void inbox_received_callback(DictionaryIterator *iter, void *context)
{
  // A new message has been successfully received.
  APP_LOG(APP_LOG_LEVEL_INFO, "New message received.");

  Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY__ready);
  if (ready_tuple)
  {
    s_js_ready = true;
  }

  Tuple *connection_tuple = dict_find(iter, MESSAGE_KEY_i_connectionError);
  if (connection_tuple)
  {
    // Something wrong happen during communication :(
    // Let the user know.
    s_are_attractions_loading = false;
    s_message_code = 2;
    strcpy(s_message_text, "Connection Error");
    window_stack_push(s_message_window, true);
  }

  
  Tuple *parkcount_tuple = dict_find(iter, MESSAGE_KEY_c_newpark_count);
  if(parkcount_tuple) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Received a PARKCOUNT Tuple");
    int parkparse_result = parse_destinations_response(iter, context);
    APP_LOG(APP_LOG_LEVEL_INFO, "Got a result of %d", parkparse_result);
    if(parkparse_result > 0) {
      print_destinations();
    }
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
    int attrCount = 0;

    // Clear existing attraction data
    memset(s_attraction_names, 0, sizeof(char) * 100 * 128);
    memset(s_attraction_status, 0, sizeof(char) * 100 * 20);

    // Parse our new data
    for (int i = 0; i < s_num_attractions; i++)
    {
      Tuple *name_tuple = dict_find(iter, MESSAGE_KEY_i_attractionNameString + i);
      Tuple *status_tuple = dict_find(iter, MESSAGE_KEY_i_attractionStatus + i);

      bool show_empty_condition = !(s_settings.showEmpty && (strcmp(status_tuple->value->cstring, "") == 0));

      if (!(name_tuple && status_tuple && show_empty_condition))
      {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to find all attraction information at index %d", i);
        continue;
      }
      
      // Get this data into the relevant array
      strcpy(s_attraction_names[attrCount], name_tuple->value->cstring);
      strcpy(s_attraction_status[attrCount], status_tuple->value->cstring);
      attrCount++;
    }

    s_num_attractions = attrCount;

    // We have all of our attraction information
    s_are_attractions_loading = false;
    APP_LOG(APP_LOG_LEVEL_INFO, "Parsed %d attractions sent from phone", s_num_attractions);

    // Trigger the loading of the attractions page
    window_stack_push(s_attraction_list_window, true);
  }

  Tuple *settings_tuple = dict_find(iter, MESSAGE_KEY_c_showParks);
  if (settings_tuple)
  {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "We've receieved a SHOW PARKS messsage");
    // If we have one key, we have them all. Dig through the whole showPark array
    for (int i = 0; i < 59; i++)
    {
      Tuple *show_tuple = dict_find(iter, MESSAGE_KEY_c_showParks + i);

      if (!show_tuple)
      {
        // We can't find this.
        APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to find visibility setting for index %d", i);
        continue;
      }

      // Update the setting value
      s_settings.parkVisibility[i] = show_tuple->value->int16;
    }

    // Save our settings
    save_settings();

    // Update our display
    update_display();
  }

  Tuple *showEmpty_tuple = dict_find(iter, MESSAGE_KEY_c_showEmpty);
  if (showEmpty_tuple)
  {
    s_settings.showEmpty = (showEmpty_tuple->value->int8 == 0);
    save_settings();
  }

  Tuple *showShowTimes_tuple = dict_find(iter, MESSAGE_KEY_c_showShowTimes);
  if (showShowTimes_tuple)
  {
    s_settings.showShowTimes = (showShowTimes_tuple->value->int8 == 0);
    save_settings();
  }

  Tuple *showValidDataOnly_tuple = dict_find(iter, MESSAGE_KEY_c_showValidDataOnly);
  if (showValidDataOnly_tuple)
  {
    s_settings.showValidDataOnly = (showValidDataOnly_tuple->value->int8 == 0);
    save_settings();
  }
}

static void send_park_request(int parkIndex)
{
  // If javascript is not ready, do not send :)
  if (!s_js_ready)
    return;

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
    int showShowTimes = s_settings.showShowTimes ? 1 : 0;
    int showValidDataOnly = s_settings.showValidDataOnly ? 1 : 0;
    dict_write_int(out_iter, MESSAGE_KEY_o_parkID, &parkID, sizeof(int), true);
    dict_write_int(out_iter, MESSAGE_KEY_c_showShowTimes, &showShowTimes, sizeof(int), true);
    dict_write_int(out_iter, MESSAGE_KEY_c_showValidDataOnly, &showValidDataOnly, sizeof(int), true);

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
  menu_layer_set_callbacks(s_attractions_menu_layer, NULL, (MenuLayerCallbacks){.get_num_rows = get_attractions_row_count, .draw_row = draw_attractions_row_handler, .get_header_height = get_attractions_header_height, .draw_header = draw_attractions_header});
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
  load_settings();
  update_park_data();

  // Configure app message settings
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  app_message_register_inbox_received(inbox_received_callback);

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

  window_destinations_push();
}

static void deinit(void)
{
  // Destroy the windows
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
