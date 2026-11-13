/*
** color.c
*/

#include "raycaster.h"

double        night_factor(double distance)
{
  double      factor;

  factor = AMB_LIGHT / distance;
  if (factor > 1.0)
    factor = 1.0;
  return (factor);
}

Uint32        shade_pixel(Uint32 pixel, double factor)
{
  Uint32      r;
  Uint32      g;
  Uint32      b;

  r = (Uint32)(((pixel >> 16) & 0xFF) * factor);
  g = (Uint32)(((pixel >> 8) & 0xFF) * factor);
  b = (Uint32)((pixel & 0xFF) * factor);
  return ((r << 16) | (g << 8) | b);
}
