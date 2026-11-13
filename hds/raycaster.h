/*
** raycaster.h
*/

#ifndef RAYCASTER_H_
#define RAYCASTER_H_

// System includes
#include <SDL3/SDL.h>
#include <stdio.h>
#include <math.h>

// Map
#define mapWidth 24
#define mapHeight 24

extern int worldMap[mapWidth][mapHeight];

// Initial values
#define WIN_X           1280
#define WIN_Y           720
#define INIT_P_POS_X    22
#define INIT_P_POS_Y    12
#define INIT_P_DIR_X    -1
#define INIT_P_DIR_Y    0
#define INIT_P_PLANE_X  0
#define INIT_P_PLANE_Y  0.66
#define MV_SPEED        0.199
#define ROT_SPEED       0.1745
#define AMB_LIGHT       7
#define MIN_WALL_DIST   0.0001
#define PLAYER_MARGIN   0.2

// Textures (one per nonzero map value, see texture.c)
#define TEX_W           64
#define TEX_H           64
#define NB_WALL_TEX     5
#define PIXEL_FORMAT    SDL_PIXELFORMAT_XRGB8888

// Structures
typedef struct      s_sdl
{
  SDL_Window      *window;
  SDL_Renderer    *renderer;
  SDL_Texture     *screen;
  Uint32          *buffer;
  Uint32          *textures[NB_WALL_TEX + 1];
} t_sdl;

typedef struct      s_raycaster
{
  double          player_pos_x;
  double          player_pos_y;
  double          player_dir_x;
  double          player_dir_y;
  double          player_plane_x;
  double          player_plane_y;
  double          ray_dir_x;
  double          ray_dir_y;
  int             map_x;
  int             map_y;
  double          side_dist_x;
  double          side_dist_y;
  double          delta_dist_x;
  double          delta_dist_y;
  int             step_x;
  int             step_y;
  int             side;
  int             line_height;
  int             draw_start;
  int             draw_end;
  double          perp_wall_dist;
} t_raycaster;

// Proto
double    night_factor(double distance);
Uint32    shade_pixel(Uint32 pixel, double factor);
int       load_textures(t_sdl *sdl);
void      free_textures(t_sdl *sdl);

#endif /* !RAYCASTER_H_ */