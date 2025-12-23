# Changes

## Version 0.7.0

### FiniteInput

- Added the `FiniteGamepad` input family.
- Added `FiniteDevice` and `FiniteDevices` for udev-less device polling.
- Moved `finite_button_handle_poll` to FiniteDraw instead of being FiniteInput.

### FiniteDraw

- `finite_draw_finish` now calls wl_display_flush after committing to garentee redraw.
- Fixed `finite_draw_finish` not using the stride and withAlpha arguments.
- `finite_draw_cleanup` now reports the program closing on `LOG_LEVEL_INFO` instead of `LOG_LEVEL_DEBUG`
- Added `finite_overlay_set_margin`
- Updated `finite_draw_png` to support dyanmic scaling and image error handling
- Fixed `finite_draw_cleanup` throwing segfaults when using a shared memory buffer from `finite_shm_aloc`

### FiniteRender

- `finite_render_get_memory_format_debug` now returns 0 on failure to silence errors.

### FiniteAudio

- `finite_audio_stop` now reports dev.isPlaying is true.
- `finite_audio_play` now checks dev.isPlayer is true before continuing.

### Examples

New Controller Demo

### Notes

Developers should **never** use shell.gamepads[i] at any time as shell.gamepads is not reliably static.
