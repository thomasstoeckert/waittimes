#include <pebble.h>

static Window *s_parks_browse_window, *s_attraction_list_window;

#pragma region ParksBrowsingWindow
static void parks_browse_load(Window *window) {

}

static void parks_browse_unload(Window *window) {

}
#pragma endregion



#pragma region AttractionsBrowsingWindow
static void attraction_list_load(Window *window) {

}

static void attraction_list_unload(Window *window) {

}
#pragma endregion




static void init(void) {
	// Create the main window (browse all parks)
	s_parks_browse_window = window_create();
	window_set_window_handlers(s_parks_browse_window, (WindowHandlers) {
		.load = parks_browse_load,
		.unload = parks_browse_unload
	});

	// Create the subwindow (browse a parks attractions)
	s_attraction_list_window = window_create();
	window_set_window_handlers(s_attraction_list_window, (WindowHandlers) {
		.load = attraction_list_load,
		.unload = attraction_list_unload
	});
}

static void deinit(void) {
	// Destroy the windows
	window_destroy(s_parks_browse_window);
	window_destroy(s_attraction_list_window);
}

int main(void) {
	// Setup application properties
	init();

	// Perform main application loop
	app_event_loop();

	// Cleanup app
	deinit();
}
