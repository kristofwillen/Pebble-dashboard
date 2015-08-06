#include <pebble.h>
#include "analogue.h"

Window *my_window;
static BitmapLayer *s_dials_layer;
static GBitmap *s_dials_bitmap;
static GPath *s_minute_arrow, *s_hour_arrow, *s_fuel_arrow, *s_am_arrow;
static Layer *s_hands_layer;
static int batteryLevel = 100;


static void battery_handler(BatteryChargeState new_state) {
  
  batteryLevel = (int)new_state.charge_percent;
}


static void hands_update_proc(Layer *layer, GContext *ctx) {
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  //int32_t second_angle = (TRIG_MAX_ANGLE * t->tm_sec / 90 - (TRIG_MAX_ANGLE/3)) % TRIG_MAX_ANGLE;
  int32_t minute_angle = ((TRIG_MAX_ANGLE * t->tm_min / 90) - (TRIG_MAX_ANGLE/3)) % TRIG_MAX_ANGLE;
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "[DBUG] tm_min=%i", tm_min);
  //int32_t hour_angle   = ((TRIG_MAX_ANGLE * (t->tm_hour %12 + t->tm_min/60) / 12 * 290/360) - (TRIG_MAX_ANGLE/3)) % TRIG_MAX_ANGLE;
  int32_t hour_angle   = (TRIG_MAX_ANGLE * 280/360 * (t->tm_hour%12 + t->tm_min/60)/12 - (TRIG_MAX_ANGLE * 110/360)) % TRIG_MAX_ANGLE;
  int32_t am_angle   = ((TRIG_MAX_ANGLE /2 * (t->tm_hour) / 24) - (TRIG_MAX_ANGLE/4)) % TRIG_MAX_ANGLE;
  int32_t fuel_angle   = ((TRIG_MAX_ANGLE * batteryLevel / 300) - (TRIG_MAX_ANGLE/3)) % TRIG_MAX_ANGLE;
  
  #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorDarkCandyAppleRed);
  #else 
    graphics_context_set_fill_color(ctx, GColorWhite);
  #endif
  graphics_context_set_stroke_color(ctx, GColorBlack);

  //gpath_rotate_to(s_hour_arrow, (225 * ((((t->tm_hour % 12) * 6) + (t->tm_min / 10))-90) / (12 * 6)));
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
  
  gpath_rotate_to(s_fuel_arrow, fuel_angle);
  gpath_draw_filled(ctx, s_fuel_arrow);
  gpath_draw_outline(ctx, s_fuel_arrow);
  
  // dot in the middle
   #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorDarkCandyAppleRed);
  #else 
    graphics_context_set_fill_color(ctx, GColorWhite);
  #endif
  graphics_fill_circle(ctx, GPoint(64,140), 6);
  graphics_fill_circle(ctx, GPoint(93,52), 6);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(29,30), 1);
}



static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(my_window));
}


void handle_init(void) {
  my_window = window_create();
  Layer *window_layer = window_get_root_layer(my_window);
  GRect bounds = layer_get_bounds(window_layer);

  #ifdef PBL_COLOR
    s_dials_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_COLOR);
  #else
    s_dials_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DIALS);
  #endif
  s_dials_layer = bitmap_layer_create(GRect(0,0,144,168));
  bitmap_layer_set_bitmap(s_dials_layer, s_dials_bitmap);
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(s_dials_layer));
  
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

  window_stack_push(my_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Get the current battery level
  battery_handler(battery_state_service_peek());
  
}


void handle_deinit(void) {
  bitmap_layer_destroy(s_dials_layer);
  layer_destroy(s_hands_layer);
  window_destroy(my_window);
  tick_timer_service_unsubscribe();
}


int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
