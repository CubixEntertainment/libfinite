# Changes

## Version 0.8.0

## FiniteRender

- Added `VkResultToString` function for pure C string conversions of VKResult
- Added the `FiniteRenderLinearAllocator` for safer memory handling in larger scale projects
- Removed validation layer requirements from `finite_render_init`

## FiniteDraw

- Added `finite_draw_set_pattern_dithering` to address issues with color banding on some devices.
- Added `finite_draw_cached_png` to improve performance for drawing images. `finite_draw_png` also can now return a cached surface through it's cache param.
- Added `finite_draw_set_wrapped_text`
- Added `finite_draw_hex_to_color_group_alpha` to allow alpha images.
- Fixed `finite_button_create` so it will now always return the focus event for the first button
- Updated `finite_draw_png` to make it so it can use custom error colors
- Fixed an issue where `finite_draw_rouned_rect` didn't fill gradients.
- Fixed an issue in `finite_draw_linear_gradient` that threw out alpha values when they were full transparent.

## FiniteInput

- Added `finite_gamepad_key_get_hold_time` to allow devs to read how long a key has been held
- Added `finite_input_keyboard_rescan` to allow keyboard rescans
- Rewrote the entire controller runtime to address the security issues brought up in [#12](https://github.com/CubixEntertainment/libfinite/issues/12)
- Temporarily removed the controller connect popup for reworks

## FiniteUser

- Added `finite_user` Function Family to allow Infinite developers to get information about who is actively using the console.

## Mailroom

- Added the mailroom middleman client that emulates specific console behaviors on non-console systems. It must run as root. See the the `README.md` for additional notes on usage and installation.

## Examples

- Improved all examples to be up to date with the common libfinite practices.
- Added an example on how to use the auth API

## Extra

- Added the FiniteJSON utilities
- Fixed an issue where the protocol required a dependency that wasn't shipped with libfinite

## Version 0.7.2

### FiniteInput

- Added the `finite_input_textbox` API
- `finite_input_keyboard_init` no longer exits if no keyboard is available.
- Added `finite_input_keyboard_rescan` to allow devs to rescan the wl_seat

### FiniteShell

- Removed `zwp_text_input_manager_v3`

## Version 0.7.1

### FiniteShell

- Added a `wl_seat`(shell.seat) and `zwp_virtual_keyboard_manager_v1`(shell.virtual_manager) struct to FiniteShell. This types are automatically defined by the compositor if they're supported

### FiniteInput

- Added the new `FiniteTextbox` for input handling
- Added `finite_key_is_alpha`
- Downgraded `finite_gamepad_poll_buttons_async`'s log level from `LOG_LEVEL_INFO` to `LOG_LEVEL_DEBUG`
- Fixed an issue in `finite_gamepad_poll_buttons_async` where releasing the Down button or Up button the dpad caused the library to report that up was pressed
- Added `FINITE_KEY_QOUTE` as a secondary reference to `FINITE_KEY_APOSTROPHE` for clarity

### FiniteLog

- Fatal errors now close the File descriptor if its not `stdout` or `stderr`

### FiniteDraw

- Added the `finite_hex_to_color_group` function to support hexadecimals values for colors
- Fixed an OOB issue with some finite-buttons

### FiniteAudio

- Fixed a small type conversion issue in `finite_audio_get_audio_duration`

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
