# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A small C implementation of Lode's raycasting tutorial (https://lodev.org/cgtutor/raycasting.html): untextured, flat-colored walls drawn one vertical line per screen column. The README calls it "a quick port, not a full project". Error handling and optimizations are intentionally minimal.

The code targets **SDL3**: it includes `<SDL3/SDL.h>` and links `-lSDL3`. It was ported from SDL2, so be careful with SDL2-era examples. In SDL3, functions return `bool` (true means success, not `0`), events are named `SDL_EVENT_*`, the key is read from `event.key.key`, and `SDL_RenderLine` takes float coordinates.

## Build & run

```sh
make          # builds ./raycaster (gcc, -O2 -W -Wall -Wextra -Werror)
./raycaster
make re       # clean rebuild; also: make clean / make fclean
```

New `.c` files must be added by hand to `SRC` in the Makefile. `-Werror` is on, so any warning breaks the build. There are no tests and no linter.

## Architecture

- `hds/raycaster.h` is the single shared header. It holds all compile-time configuration as `#define`s: window size, player start position/direction/camera plane, movement and rotation speed, `AMB_LIGHT`, and colors. It also defines the two state structs:
  - `t_sdl`: the window and renderer.
  - `t_raycaster`: the player state (position, direction, camera plane) plus per-ray scratch values (ray direction, DDA step and side distances, hit side, wall distance, draw range). These ray fields are overwritten for every column.
- `src/main.c` runs the whole loop. For each frame and each screen column `x`, it calls these in order: `initial_calc` (ray setup) → `perform_dda` (grid traversal until a wall hit) → `calc_wall_height` → `draw_vert_line`. After all columns it calls `render_frame` (present, then clear) and `handle_events`. Input is handled on discrete `KEYDOWN` events, not held-key polling, so movement is one step per key repeat. Collision (`try_move`) checks `worldMap` on each axis separately and keeps the player `PLAYER_MARGIN` away from walls. `calc_wall_height` clamps the wall distance to at least `MIN_WALL_DIST` so it never divides by zero.
- `src/map.c` holds `worldMap`, a 24×24 grid indexed `[x][y]`. `0` is empty, and nonzero values are walls. The outer border must stay solid because DDA has no bounds check.
- `src/color.c` maps wall IDs to colors: 1 red, 2 green, 3 blue, 4 white, anything else black (so the `5` in the map renders black). It also applies the distance-based "night" darkening (`apply_night_effect`, scaled by `AMB_LIGHT`). Walls hit on a y-side (`side == 1`) are drawn at half brightness in `draw_vert_line`.

## Code style

The code uses Epitech-style C: `t_`-prefixed typedefs for `s_`-named structs, `return (value);` with parentheses, a `/* ** file */` header comment, and 2-space indentation with aligned declarations.
