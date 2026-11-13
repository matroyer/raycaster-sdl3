# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A small C implementation of Lode's raycasting tutorial (https://lodev.org/cgtutor/raycasting.html): textured walls, floor and ceiling, rendered in software into a pixel buffer.

The code targets **SDL3**: it includes `<SDL3/SDL.h>` and links `-lSDL3`. It was ported from SDL2, so be careful with SDL2-era examples. In SDL3, functions return `bool` (true means success, not `0`), events are named `SDL_EVENT_*`, and the key is read from `event.key.key`. PNGs are loaded with SDL3's built-in `SDL_LoadPNG`, so SDL3_image is not a dependency.

## Build & run

```sh
make          # builds ./raycaster (gcc, -O2 -W -Wall -Wextra -Werror)
./raycaster
make re       # clean rebuild; also: make clean / make fclean
make debug    # rebuild with AddressSanitizer
```

New `.c` files must be added by hand to `SRC` in the Makefile. `-Werror` is on, so any warning breaks the build. There are no tests and no linter. The `raycaster` binary is gitignored.

Textures are loaded from `pics/` relative to the executable's directory (`SDL_GetBasePath`), not the current directory, so the binary must stay in the repo root next to `pics/`.

## Architecture

- `hds/raycaster.h` is the single shared header. It holds all compile-time configuration as `#define`s: window size, player start position/direction/camera plane, movement and rotation speed, `AMB_LIGHT`, texture size (`TEX_W`/`TEX_H`, 64×64), texture slots (`NB_WALL_TEX`, `FLOOR_TEX`, `CEILING_TEX`, `NB_TEX`) and the pixel format (`PIXEL_FORMAT`, XRGB8888). It also defines the two state structs:
  - `t_sdl`: the window and renderer, the streaming screen texture, the `WIN_X * WIN_Y` pixel buffer, and the loaded textures (`textures[NB_TEX]`, each a `TEX_W * TEX_H` array of `Uint32`; slot 0 is unused).
  - `t_raycaster`: the player state (position, direction, camera plane) plus per-ray scratch values (ray direction, DDA step and side distances, hit side, wall distance, line height, draw range). These ray fields are overwritten for every column.
- `src/main.c` runs the whole loop. Each frame starts with `draw_floor_ceiling`, which casts the floor one horizontal scanline at a time and mirrors each row onto the ceiling, filling every pixel of the buffer. Then, for each screen column `x`, it calls these in order: `initial_calc` (ray setup) → `perform_dda` (grid traversal until a wall hit) → `calc_wall_height` → `draw_vert_line` (samples the wall texture, using `calc_tex_x` for the column). After all columns it calls `render_frame` (upload the buffer to the screen texture and present; the buffer is not cleared, because `draw_floor_ceiling` overwrites every pixel) and `handle_events`. Input is handled on discrete `KEYDOWN` events, not held-key polling, so movement is one step per key repeat. Collision (`try_move`) checks `worldMap` on each axis separately and keeps the player `PLAYER_MARGIN` away from walls. `calc_wall_height` clamps the wall distance to at least `MIN_WALL_DIST` so it never divides by zero.
- `src/map.c` holds `worldMap`, a 24×24 grid indexed `[x][y]`. `0` is empty, and values `1` to `NB_WALL_TEX` are walls. The outer border must stay solid because DDA has no bounds check.
- `src/texture.c` maps each slot to a PNG in `g_texture_files`, using designated indices: wall values `1` to `NB_WALL_TEX`, then `FLOOR_TEX` and `CEILING_TEX`. `load_textures` refuses to start if the map contains a value outside `0..NB_WALL_TEX`, if a slot has no file, or if a PNG is not `TEX_W`×`TEX_H`. To add a wall type, add its entry and raise `NB_WALL_TEX`.
- `src/color.c` handles shading. `night_factor` gives the distance-based "night" darkening, `min(1, AMB_LIGHT / distance)`, and `shade_pixel` scales a pixel's channels by a factor. Walls use the wall distance; the floor and ceiling use the row distance. Walls hit on a y-side (`side == 1`) are drawn at half brightness in `draw_vert_line`.

## Code style

The code uses Epitech-style C: `t_`-prefixed typedefs for `s_`-named structs, `return (value);` with parentheses, a `/* ** file */` header comment, and 2-space indentation with aligned declarations.
