#include <pebble.h>
#include "destinations.h"
#include "destinations_page.h"

static bool s_js_ready = false;

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
  
  Tuple *parkcount_tuple = dict_find(iter, MESSAGE_KEY_c_newpark_count);
  if(parkcount_tuple) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Received a PARKCOUNT Tuple");
    int parkparse_result = parse_destinations_response(iter, context);
    APP_LOG(APP_LOG_LEVEL_INFO, "Got a result of %d", parkparse_result);
    if(parkparse_result > 0) {
      print_destinations();
    }
    refresh_destinations_display();
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

  window_destinations_push();
}

static void deinit(void)
{
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
