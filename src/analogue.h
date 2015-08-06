#pragma once
#include "pebble.h"

  
static const GPathInfo MINUTE_HAND_POINTS = {
  4,
  (GPoint []) {
    {-5, 0 },
    { 0, 10 },
    { 5, 0 },
    {0,-50}  
  }
};

static const GPathInfo HOUR_HAND_POINTS = {
  4, (GPoint []){
    {-5, 0},
    {0, 10},
    {5, 0},
    {0,-40}
  }
};

static const GPathInfo FUEL_HAND_POINTS = {
  4, (GPoint []){
    {-5, 0},
    {0, 5},
    {5, 0},
    {0,-20}
  }
};

static const GPathInfo AM_HAND_POINTS = {
  4, (GPoint []){
    {-4, 0},
    {0, 3},
    {4, 0},
    {0,-18}
  }
};
