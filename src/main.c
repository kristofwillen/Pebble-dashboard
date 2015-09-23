#include <pebble.h>
#include "analogue.h"
#define KEY_INVERT 0 

Window *my_window;
static BitmapLayer *s_dials_layer;
static GBitmap *s_dials_bitmap;
static GPath *s_minute_arrow, *s_hour_arrow, *s_fuel_arrow, *s_am_arrow;
static Layer *s_hands_layer;
#ifdef PBL_COLOR
#else  
  static InverterLayer *s_inv_layer;
#endif
static int batteryLevel = 100;
static TextLayer *s_time_layer, *s_bg_layer;
static GFont s_time_font;
static bool inverted = false;


static void battery_handler(BatteryChargeState new_state) {
  batteryLevel = (int)new_state.charge_percent;
}


static void hands_update_proc(Layer *layer, GContext *ctx) {
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  static char s_date_buffer[10];
    
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a %d", t);
  text_layer_set_text(s_time_layer, s_date_buffer);
  
  //int32_t second_angle = (TRIG_MAX_ANGLE * t->tm_sec / 90 - (TRIG_MAX_ANGLE/3)) % TRIG_MAX_ANGLE;
  //int32_t minute_angle = ((TRIG_MAX_ANGLE * 0 / 90) - (TRIG_MAX_ANGLE/3)) % TRIG_MAX_ANGLE;
  // int32_t hour_angle  = TRIG_MAX_ANGLE * (t->tm_sec%12)/16 - (TRIG_MAX_ANGLE*115/360);
  //int32_t hour_angle   = (TRIG_MAX_ANGLE * 280/360 * (t->tm_hour%12 + t->tm_min/60)/12 - (TRIG_MAX_ANGLE * 110/360)) % TRIG_MAX_ANGLE;
  int32_t minute_angle = ((TRIG_MAX_ANGLE * t->tm_min / 90) - (TRIG_MAX_ANGLE/3)) % TRIG_MAX_ANGLE;
  int32_t hour_angle   = ((TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / 96) - (TRIG_MAX_ANGLE/360*115);
  int32_t am_angle     = ((TRIG_MAX_ANGLE /2 * (t->tm_hour) / 24) - (TRIG_MAX_ANGLE/4)) % TRIG_MAX_ANGLE;
  int32_t fuel_angle   = ((TRIG_MAX_ANGLE * batteryLevel / 100 * 135/360) - (TRIG_MAX_ANGLE*135/360)) % TRIG_MAX_ANGLE;
  
  #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorRed);
  #else 
    graphics_context_set_fill_color(ctx, GColorWhite);
  #endif
  graphics_context_set_stroke_color(ctx, GColorBlack);

  gpath_rotate_to(s_hour_arrow, hour_angle);
  gpath_draw_filled(ctx, s_hour_arrow);
  gpath_draw_outline(ctx, s_hour_arrow);

  gpath_rotate_to(s_minute_arrow, minute_angle);
  gpath_draw_filled(ctx, s_minute_arrow);
  gpath_draw_outline(ctx, s_minute_arrow);
  
  #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorVividCerulean);
  #endif
  gpath_rotate_to(s_am_arrow, am_angle);
  gpath_draw_filled(ctx, s_am_arrow);
  gpath_draw_outline(ctx, s_am_arrow);
  
  // Only checks once per 10 minutes battery state
  if ((t->tm_min%10) == 0) { battery_handler(battery_state_service_peek()); }
  gpath_rotate_to(s_fuel_arrow, fuel_angle);
  gpath_draw_filled(ctx, s_fuel_arrow);
  gpath_draw_outline(ctx, s_fuel_arrow);
  
  // dot in the middle
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(64,140), 1);
  graphics_fill_circle(ctx, GPoint(93,52), 1);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(29,30), 1);
}


static void in_recv_handler(DictionaryIterator *iterator, void *context) {
  //Get data
  Tuple *t = dict_read_first(iterator);
  
  while (t) {
   APP_LOG(APP_LOG_LEVEL_DEBUG, "[DBUG] Found dict key=%i", (int)t->key);
   switch(t->key) {
    case KEY_INVERT:
      
      if (strcmp(t->value->cstring, "0") == 0) { 
        inverted = false; 
        #ifdef PBL_COLOR
        #else  
          layer_set_hidden((Layer *)s_inv_layer,true);
        #endif
        APP_LOG(APP_LOG_LEVEL_DEBUG, "[DBUG]  inverted = 0");
      }
      else { 
        inverted = true; 
        #ifdef PBL_COLOR
        #else  
          layer_set_hidden((Layer *)s_inv_layer,false);
        #endif
        APP_LOG(APP_LOG_LEVEL_DEBUG, "[DBUG] inverted = 1"); 
      }
      break;
   }
    
   t = dict_read_next(iterator);
    
  }
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(my_window));
}


static void inbox_dropped(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "[DBUG] inbox_dropped %d", reason);
}

  

void handle_init(void) {
  
  app_message_register_inbox_received(in_recv_handler);
  app_message_register_inbox_dropped(inbox_dropped);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  // Load user config
  if (persist_exists(KEY_INVERT)) { inverted = persist_read_bool(KEY_INVERT); }
  
  
  my_window = window_create();
  Layer *window_layer = window_get_root_layer(my_window);
  GRect bounds = layer_get_bounds(window_layer);

  s_bg_layer = text_layer_create(GRect(0,0,144,168));
  #ifdef PBL_COLOR
    text_layer_set_background_color(s_bg_layer,GColorBlack);
    layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(s_bg_layer));
  #endif
    
  #ifdef PBL_COLOR
    s_dials_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_TRANSPARANT);
  #else
    s_dials_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DIALS);
  #endif
  s_dials_layer = bitmap_layer_create(GRect(0,0,144,168));
  bitmap_layer_set_bitmap(s_dials_layer, s_dials_bitmap);
  #ifdef PBL_COLOR
    bitmap_layer_set_compositing_mode(s_dials_layer,GCompOpSet);
  #endif
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(s_dials_layer));
  
  s_time_layer = text_layer_create(GRect(44, 148, 36, 15));
   #ifdef PBL_COLOR
    text_layer_set_background_color(s_time_layer, GColorRajah);
  #else 
    text_layer_set_background_color(s_time_layer, GColorWhite);
  #endif
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "Sat 01");
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DS_DIGITAL_12));
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(s_time_layer));
  
  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);
  layer_add_child(window_layer, s_hands_layer);
  
  s_minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
  s_hour_arrow = gpath_create(&HOUR_HAND_POINTS);
  s_fuel_arrow = gpath_create(&FUEL_HAND_POINTS);
  s_am_arrow   = gpath_create(&AM_HAND_POINTS);
  
  gpath_move_to(s_minute_arrow, GPoint(64,140));
  gpath_move_to(s_hour_arrow, GPoint(93,52));
  gpath_move_to(s_am_arrow, GPoint(16,77));
  gpath_move_to(s_fuel_arrow, GPoint(29,30));
  
  #ifdef PBL_COLOR
  #else  
    s_inv_layer = inverter_layer_create(GRect(0,0,144,168));
    layer_add_child(window_get_root_layer(my_window), inverter_layer_get_layer(s_inv_layer));
  
    if (inverted) { layer_set_hidden((Layer *)s_inv_layer,false); }
    else { layer_set_hidden((Layer *)s_inv_layer,true); }
  #endif
    
  window_stack_push(my_window, true);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  // Get the current battery level
  battery_handler(battery_state_service_peek());
  
}


void handle_deinit(void) {
  // Writing persistent storage
  APP_LOG(APP_LOG_LEVEL_DEBUG, "[DBUG] Writing persistent data...");
  persist_write_bool(KEY_INVERT, inverted);
  
  gpath_destroy(s_minute_arrow);
  gpath_destroy(s_hour_arrow);
  gpath_destroy(s_fuel_arrow);
  gpath_destroy(s_am_arrow);
  
  bitmap_layer_destroy(s_dials_layer);
  layer_destroy(s_hands_layer);
  #ifdef PBL_COLOR
  #else
    inverter_layer_destroy(s_inv_layer);
  #endif
  text_layer_destroy(s_bg_layer);  
  window_destroy(my_window);
  tick_timer_service_unsubscribe();
}


int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
