/*
** main.c
*/

#include "raycaster.h"

int           init(t_sdl *sdl, t_raycaster *rc)
{
  sdl->window = NULL;
  sdl->renderer = NULL;
  rc->player_pos_x = INIT_P_POS_X;
  rc->player_pos_y = INIT_P_POS_Y;
  rc->player_dir_x = INIT_P_DIR_X;
  rc->player_dir_y = INIT_P_DIR_Y;
  rc->player_plane_x = INIT_P_PLANE_X;
  rc->player_plane_y = INIT_P_PLANE_Y;
  if (!SDL_Init(SDL_INIT_VIDEO))
  {
    fprintf(stderr,"SDL initialization failed (%s)\n",SDL_GetError());
    return (-1);
  }
  if (!SDL_CreateWindowAndRenderer("Raycaster", WIN_X, WIN_Y, 0, &sdl->window, &sdl->renderer))
  {
    fprintf(stderr,"Window creation failed (%s)\n",SDL_GetError());
    return (-1);
  }
  if (!SDL_SetRenderVSync(sdl->renderer, 1))
    fprintf(stderr,"VSync unavailable (%s)\n",SDL_GetError());
  return (0);
}

void          cleanup(t_sdl *sdl)
{
  if (sdl->renderer)
    SDL_DestroyRenderer(sdl->renderer);
  if (sdl->window)
    SDL_DestroyWindow(sdl->window);
  SDL_Quit();
}

void          initial_calc(t_raycaster *rc, int x)
{
  double      camera_x;

  camera_x = 2 * x / (double)(WIN_X) - 1;
  rc->ray_dir_x = rc->player_dir_x + rc->player_plane_x * camera_x;
  rc->ray_dir_y = rc->player_dir_y + rc->player_plane_y * camera_x;
  rc->map_x = (int)(rc->player_pos_x);
  rc->map_y = (int)(rc->player_pos_y);
  rc->delta_dist_x = (rc->ray_dir_x == 0) ? 1e30 : fabs(1 / rc->ray_dir_x);
  rc->delta_dist_y = (rc->ray_dir_y == 0) ? 1e30 : fabs(1 / rc->ray_dir_y);
  if (rc->ray_dir_x < 0)
  {
    rc->step_x = -1;
    rc->side_dist_x = (rc->player_pos_x - rc->map_x) * rc->delta_dist_x;
  }
  else
  {
    rc->step_x = 1;
    rc->side_dist_x = (rc->map_x + 1.0 - rc->player_pos_x) * rc->delta_dist_x;
  }
  if (rc->ray_dir_y < 0)
  {
    rc->step_y = -1;
    rc->side_dist_y = (rc->player_pos_y - rc->map_y) * rc->delta_dist_y;
  }
  else
  {
    rc->step_y = 1;
    rc->side_dist_y = (rc->map_y + 1.0 - rc->player_pos_y) * rc->delta_dist_y;
  }
}

void          perform_dda(t_raycaster *rc)
{
  int         hit;

  hit = 0;
  while (hit == 0)
  {
    if (rc->side_dist_x < rc->side_dist_y)
    {
      rc->side_dist_x += rc->delta_dist_x;
      rc->map_x += rc->step_x;
      rc->side = 0;
    }
    else
    {
      rc->side_dist_y += rc->delta_dist_y;
      rc->map_y += rc->step_y;
      rc->side = 1;
    }
    if (worldMap[rc->map_x][rc->map_y] > 0)
      hit = 1;
  }
}

void          calc_wall_height(t_raycaster *rc)
{
  int         line_height;

  if (rc->side == 0)
    rc->perp_wall_dist = rc->side_dist_x - rc->delta_dist_x;
  else
    rc->perp_wall_dist = rc->side_dist_y - rc->delta_dist_y;
  if (rc->perp_wall_dist < MIN_WALL_DIST)
    rc->perp_wall_dist = MIN_WALL_DIST;
  line_height = (int)(WIN_Y / rc->perp_wall_dist);
  rc->draw_start = -line_height / 2 + WIN_Y / 2;
  if (rc->draw_start < 0)
    rc->draw_start = 0;
  rc->draw_end = line_height / 2 + WIN_Y / 2;
  if (rc->draw_end >= WIN_Y)
    rc->draw_end = WIN_Y - 1;
}

void          draw_vert_line(t_sdl *sdl, t_raycaster *rc, int x)
{
  SDL_Color   color;

  color = apply_night_effect(select_wall_color(rc->map_x, rc->map_y), rc->perp_wall_dist);
  if (rc->side == 1)
  {
    color.r /= 2;
    color.g /= 2;
    color.b /= 2;
  }
  SDL_SetRenderDrawColor(sdl->renderer, color.r, color.g, color.b, color.a);
  SDL_RenderLine(sdl->renderer, (float)x, (float)rc->draw_start, (float)x, (float)rc->draw_end);
}

void          render_frame(t_sdl *sdl)
{
  SDL_RenderPresent(sdl->renderer);
  SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(sdl->renderer);
}

void          try_move(t_raycaster *rc, double dx, double dy)
{
  double      margin_x;
  double      margin_y;

  margin_x = (dx > 0) ? PLAYER_MARGIN : -PLAYER_MARGIN;
  margin_y = (dy > 0) ? PLAYER_MARGIN : -PLAYER_MARGIN;
  if (worldMap[(int)(rc->player_pos_x + dx + margin_x)][(int)(rc->player_pos_y)] == 0)
    rc->player_pos_x += dx;
  if (worldMap[(int)(rc->player_pos_x)][(int)(rc->player_pos_y + dy + margin_y)] == 0)
    rc->player_pos_y += dy;
}

void          rotate(t_raycaster *rc, double angle)
{
  double      c;
  double      s;
  double      old_dir_x;
  double      old_plane_x;

  c = cos(angle);
  s = sin(angle);
  old_dir_x = rc->player_dir_x;
  rc->player_dir_x = rc->player_dir_x * c - rc->player_dir_y * s;
  rc->player_dir_y = old_dir_x * s + rc->player_dir_y * c;
  old_plane_x = rc->player_plane_x;
  rc->player_plane_x = rc->player_plane_x * c - rc->player_plane_y * s;
  rc->player_plane_y = old_plane_x * s + rc->player_plane_y * c;
}

int           handle_events(t_raycaster *rc)
{
  SDL_Event   event;

  while (SDL_PollEvent(&event))
  {
    if (event.type == SDL_EVENT_QUIT)
      return (-1);
    if (event.type == SDL_EVENT_KEY_DOWN)
    {
      if (event.key.key == SDLK_UP)
        try_move(rc, rc->player_dir_x * MV_SPEED, rc->player_dir_y * MV_SPEED);
      else if (event.key.key == SDLK_DOWN)
        try_move(rc, -rc->player_dir_x * MV_SPEED, -rc->player_dir_y * MV_SPEED);
      else if (event.key.key == SDLK_RIGHT)
        rotate(rc, -ROT_SPEED);
      else if (event.key.key == SDLK_LEFT)
        rotate(rc, ROT_SPEED);
    }
  }
  return (0);
}

void          raycaster(t_sdl *sdl, t_raycaster *rc)
{
  bool        done;

  done = false;
  while(!done)
  {
    for(int x = 0; x < WIN_X; x++)
    {
      initial_calc(rc, x);
      perform_dda(rc);
      calc_wall_height(rc);
      draw_vert_line(sdl, rc, x);
    }
    render_frame(sdl);
    if (handle_events(rc) != 0)
      done = true;
  }
}

int           main(void)
{
  t_sdl       sdl;
  t_raycaster rc;

  if (init(&sdl, &rc) != 0)
  {
    cleanup(&sdl);
    return (1);
  }
  raycaster(&sdl, &rc);
  cleanup(&sdl);
  return (0);
}
