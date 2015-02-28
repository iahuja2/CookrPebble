#include <pebble.h>
enum {
    SELECT_KEY = 0x0,
    UP_KEY = 0x01,
    DOWN_KEY = 0x02,
    TITLE = 0x03,
    TIME = 0x04,
    STEP = 0x05,
    KEY_BUTTON = 0x06,
};
char buf[6];

Window* window;
TextLayer *title_layer, *step_layer, *time_layer; 

static TextLayer* init_text_layer(GRect location, GColor colour, GColor background, const char *res_id, GTextAlignment alignment)
{
  TextLayer *layer = text_layer_create(location);
  text_layer_set_text_color(layer, colour);
  text_layer_set_background_color(layer, background);
  text_layer_set_font(layer, fonts_get_system_font(res_id));
  text_layer_set_text_alignment(layer, alignment);
 
  return layer;
}


void window_load(Window *window)
{
 title_layer = init_text_layer(GRect(5, 0, 144, 30), GColorBlack, GColorClear, "RESOURCE_ID_GOTHIC_24", GTextAlignmentCenter);
  text_layer_set_text(title_layer, "Cookr");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(title_layer));
 step_layer = init_text_layer(GRect(5, 60, 144, 30), GColorBlack, GColorClear, "RESOURCE_ID_GOTHIC_18", GTextAlignmentCenter);
  text_layer_set_text(step_layer, "Step");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(step_layer));
 time_layer = init_text_layer(GRect(5, 120, 144, 30), GColorBlack, GColorClear, "RESOURCE_ID_GOTHIC_18", GTextAlignmentCenter);
  text_layer_set_text(time_layer, "Time");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
}

static void send(int key, int message) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_int(iter, key, &message, sizeof(int), true);
  app_message_outbox_send();
}

static void inbox_received_handler(DictionaryIterator *iterator, void *context) {
  // Get the first pair
  Tuple *t = dict_read_first(iterator);
  // Process all pairs present
  while(t != NULL) {
  // Process this pair's key
  switch(t->key) {
    case TITLE:
      text_layer_set_text(title_layer, t->value->cstring);
    case TIME:
      snprintf(buf, sizeof (buf), "%d", (int)t->value->int32);
      text_layer_set_text(step_layer, buf);
    case STEP:
      snprintf(buf, sizeof (buf), "%d", (int)t->value->int32);
      text_layer_set_text(time_layer, buf);
    // Trigger vibration
    //vibes_short_pulse();
  break;
  default:
  APP_LOG(APP_LOG_LEVEL_INFO, "Unknown key: %d", (int)t->key);
break;
}
// Get next pair, if any
t = dict_read_next(iterator);
}
}
static void inbox_dropped_handler(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}
static void outbox_failed_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}
static void outbox_sent_handler(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

/********************************* Buttons ************************************/
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  send(KEY_BUTTON, SELECT_KEY);
}
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  send(KEY_BUTTON, UP_KEY);
}
static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  send(KEY_BUTTON, DOWN_KEY);
}
static void click_config_provider(void *context) {
// Assign button handlers
window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

void window_unload(Window *window)
{
 text_layer_destroy(title_layer);
 text_layer_destroy(step_layer);
 text_layer_destroy(time_layer);
}
 
void init()
{
  // Register callbacks
  app_message_register_inbox_received(inbox_received_handler);
  app_message_register_inbox_dropped(inbox_dropped_handler);
  app_message_register_outbox_failed(outbox_failed_handler);
  app_message_register_outbox_sent(outbox_sent_handler);
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  window = window_create();
  WindowHandlers handlers = {
    .load = window_load,
    .unload = window_unload
  };
  window_set_window_handlers(window, handlers);
 
  window_stack_push(window, true);
}
 
void deinit()
{
  window_destroy(window);
}
 
int main(void)
{
  init();
  app_event_loop();
  deinit();
}
