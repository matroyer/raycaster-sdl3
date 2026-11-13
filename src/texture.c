/*
** texture.c
*/

#include "raycaster.h"

/*
** Texture used for each map value: index N is drawn for walls equal to N.
** Index 0 is empty space and has no texture.
** FLOOR_TEX and CEILING_TEX hold the floor and the ceiling.
*/
static const char *g_texture_files[NB_TEX] =
{
  [0] = NULL,
  [1] = "pics/eagle.png",
  [2] = "pics/redbrick.png",
  [3] = "pics/purplestone.png",
  [4] = "pics/colorstone.png",
  [5] = "pics/bluestone.png",
  [FLOOR_TEX] = "pics/greystone.png",
  [CEILING_TEX] = "pics/wood.png"
};

static Uint32 *load_texture(const char *base_path, const char *file)
{
  char        *path;
  SDL_Surface *png;
  SDL_Surface *surface;
  Uint32      *pixels;

  pixels = NULL;
  if (SDL_asprintf(&path, "%s%s", base_path, file) < 0)
    return (NULL);
  png = SDL_LoadPNG(path);
  if (!png)
  {
    fprintf(stderr, "Cannot load %s (%s)\n", path, SDL_GetError());
    SDL_free(path);
    return (NULL);
  }
  surface = SDL_ConvertSurface(png, PIXEL_FORMAT);
  SDL_DestroySurface(png);
  if (!surface)
    fprintf(stderr, "Cannot convert %s (%s)\n", path, SDL_GetError());
  else if (surface->w != TEX_W || surface->h != TEX_H)
    fprintf(stderr, "%s must be %dx%d\n", path, TEX_W, TEX_H);
  else if ((pixels = SDL_malloc(TEX_W * TEX_H * sizeof(Uint32))) != NULL)
    for (int y = 0; y < TEX_H; y++)
      SDL_memcpy(pixels + y * TEX_W,
                 (Uint8 *)surface->pixels + y * surface->pitch,
                 TEX_W * sizeof(Uint32));
  SDL_DestroySurface(surface);
  SDL_free(path);
  return (pixels);
}

static int    check_map(void)
{
  for (int x = 0; x < mapWidth; x++)
    for (int y = 0; y < mapHeight; y++)
      if (worldMap[x][y] < 0 || worldMap[x][y] > NB_WALL_TEX)
      {
        fprintf(stderr, "Map value %d at [%d][%d] has no texture\n",
                worldMap[x][y], x, y);
        return (-1);
      }
  return (0);
}

int           load_textures(t_sdl *sdl)
{
  const char  *base_path;

  if (check_map() != 0)
    return (-1);
  base_path = SDL_GetBasePath();
  if (!base_path)
    base_path = "";
  for (int i = 1; i < NB_TEX; i++)
  {
    if (!g_texture_files[i])
    {
      fprintf(stderr, "Texture %d has no file\n", i);
      return (-1);
    }
    sdl->textures[i] = load_texture(base_path, g_texture_files[i]);
    if (!sdl->textures[i])
      return (-1);
  }
  return (0);
}

void          free_textures(t_sdl *sdl)
{
  for (int i = 0; i < NB_TEX; i++)
  {
    SDL_free(sdl->textures[i]);
    sdl->textures[i] = NULL;
  }
}
