#include "waittimes.h"

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
    outbox_set_ready(true);
  }
  
  Tuple *parkcount_tuple = dict_find(iter, MESSAGE_KEY_c_newpark_count);
  if(parkcount_tuple) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Received a PARKCOUNT Tuple");
    int parkparse_result = parse_destinations_response(iter, context);
    APP_LOG(APP_LOG_LEVEL_INFO, "Got a result of %d", parkparse_result);
    if(parkparse_result > 0 && PWT_DEBUG) {
      print_destinations();
    }
    refresh_destinations_display();
    persist_save_destinations_data();
  }

  Tuple *attractioncount_tuple = dict_find(iter, MESSAGE_KEY_i_attractionCount);
  if(attractioncount_tuple)
  {
    int attractionparse_result = parse_attractions_response(iter, context);
    if(attractionparse_result >= 0 && PWT_DEBUG) {
      print_attractions();
    }
    refresh_attractions_display();
  }
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

  init_fonts();

  persist_clay_load_settings();
  persist_load_destinations_data();

  window_destinations_push();
}

static void deinit(void)
{
  window_attractions_deinit();
  window_destinations_deinit();
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
