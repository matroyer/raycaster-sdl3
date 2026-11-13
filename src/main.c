/*
** main.c
*/

#include "raycaster.h"

int           init(t_sdl *sdl, t_raycaster *rc)
{
  sdl->window = NULL;
  sdl->renderer = NULL;
  sdl->screen = NULL;
  sdl->buffer = NULL;
  for (int i = 0; i < NB_TEX; i++)
    sdl->textures[i] = NULL;
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
  sdl->screen = SDL_CreateTexture(sdl->renderer, PIXEL_FORMAT, SDL_TEXTUREACCESS_STREAMING, WIN_X, WIN_Y);
  if (!sdl->screen)
  {
    fprintf(stderr,"Screen texture creation failed (%s)\n",SDL_GetError());
    return (-1);
  }
  sdl->buffer = SDL_calloc(WIN_X * WIN_Y, sizeof(Uint32));
  if (!sdl->buffer)
  {
    fprintf(stderr,"Screen buffer allocation failed\n");
    return (-1);
  }
  if (load_textures(sdl) != 0)
    return (-1);
  return (0);
}

void          cleanup(t_sdl *sdl)
{
  free_textures(sdl);
  SDL_free(sdl->buffer);
  if (sdl->screen)
    SDL_DestroyTexture(sdl->screen);
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
  if (rc->side == 0)
    rc->perp_wall_dist = rc->side_dist_x - rc->delta_dist_x;
  else
    rc->perp_wall_dist = rc->side_dist_y - rc->delta_dist_y;
  if (rc->perp_wall_dist < MIN_WALL_DIST)
    rc->perp_wall_dist = MIN_WALL_DIST;
  rc->line_height = (int)(WIN_Y / rc->perp_wall_dist);
  rc->draw_start = -rc->line_height / 2 + WIN_Y / 2;
  if (rc->draw_start < 0)
    rc->draw_start = 0;
  rc->draw_end = rc->line_height / 2 + WIN_Y / 2;
  if (rc->draw_end >= WIN_Y)
    rc->draw_end = WIN_Y - 1;
}

int           calc_tex_x(t_raycaster *rc)
{
  double      wall_x;
  int         tex_x;

  if (rc->side == 0)
    wall_x = rc->player_pos_y + rc->perp_wall_dist * rc->ray_dir_y;
  else
    wall_x = rc->player_pos_x + rc->perp_wall_dist * rc->ray_dir_x;
  wall_x -= floor(wall_x);
  tex_x = (int)(wall_x * TEX_W);
  if (tex_x >= TEX_W)
    tex_x = TEX_W - 1;
  if ((rc->side == 0 && rc->ray_dir_x > 0) || (rc->side == 1 && rc->ray_dir_y < 0))
    tex_x = TEX_W - tex_x - 1;
  return (tex_x);
}

void          draw_vert_line(t_sdl *sdl, t_raycaster *rc, int x)
{
  Uint32      *texture;
  int         tex_x;
  int         tex_y;
  double      step;
  double      tex_pos;
  double      shade;

  texture = sdl->textures[worldMap[rc->map_x][rc->map_y]];
  tex_x = calc_tex_x(rc);
  step = (double)TEX_H / rc->line_height;
  tex_pos = (rc->draw_start - WIN_Y / 2 + rc->line_height / 2) * step;
  shade = night_factor(rc->perp_wall_dist);
  if (rc->side == 1)
    shade /= 2;
  for (int y = rc->draw_start; y <= rc->draw_end; y++)
  {
    tex_y = (int)tex_pos & (TEX_H - 1);
    tex_pos += step;
    sdl->buffer[y * WIN_X + x] = shade_pixel(texture[tex_y * TEX_W + tex_x], shade);
  }
}

/*
** Floor casting, one horizontal scanline at a time. Each floor row y is
** mirrored by the ceiling row WIN_Y - y - 1, which is at the same distance.
** Rows are sampled at the pixel center (+ 0.5) so the horizon never divides
** by zero and every row of the screen gets filled.
*/
void          draw_floor_ceiling(t_sdl *sdl, t_raycaster *rc)
{
  double      ray_x0;
  double      ray_y0;
  double      row_dist;
  double      floor_x;
  double      floor_y;
  double      step_x;
  double      step_y;
  double      shade;
  int         tex_off;

  ray_x0 = rc->player_dir_x - rc->player_plane_x;
  ray_y0 = rc->player_dir_y - rc->player_plane_y;
  for (int y = WIN_Y / 2; y < WIN_Y; y++)
  {
    row_dist = (0.5 * WIN_Y) / (y - WIN_Y / 2 + 0.5);
    step_x = row_dist * 2 * rc->player_plane_x / WIN_X;
    step_y = row_dist * 2 * rc->player_plane_y / WIN_X;
    floor_x = rc->player_pos_x + row_dist * ray_x0;
    floor_y = rc->player_pos_y + row_dist * ray_y0;
    shade = night_factor(row_dist);
    for (int x = 0; x < WIN_X; x++)
    {
      tex_off = ((int)(TEX_H * (floor_y - floor(floor_y))) & (TEX_H - 1)) * TEX_W + ((int)(TEX_W * (floor_x - floor(floor_x))) & (TEX_W - 1));
      floor_x += step_x;
      floor_y += step_y;
      sdl->buffer[y * WIN_X + x] = shade_pixel(sdl->textures[FLOOR_TEX][tex_off], shade);
      sdl->buffer[(WIN_Y - y - 1) * WIN_X + x] = shade_pixel(sdl->textures[CEILING_TEX][tex_off], shade);
    }
  }
}

void          render_frame(t_sdl *sdl)
{
  SDL_UpdateTexture(sdl->screen, NULL, sdl->buffer, WIN_X * sizeof(Uint32));
  SDL_RenderTexture(sdl->renderer, sdl->screen, NULL, NULL);
  SDL_RenderPresent(sdl->renderer);
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
    draw_floor_ceiling(sdl, rc);
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
