/*
    Window drawing with Libfinite SDK example
    Written by Gabriel Thompson <gabriel.thomp@cubixdev.org>
*/

#include "finite/draw.h"
#include "finite/log.h"

int main() {
    finite_log_init(stdout, LOG_LEVEL_DEBUG, false);
    FINITE_LOG("Starting.");
    // create a new shell
    FiniteShell *myShell = finite_shell_init("wayland-0");
    
    if (!myShell) {
        FINITE_LOG("Unable to init shell");
        return 1;
    }

    // to draw windows make a window
    finite_window_init(myShell);

    if (!myShell) {
        FINITE_LOG("Unable to make shell");
        return 1;
    }

    /* 
        to get custom window sizing we can call finite_window_size_set by default the size is set to the full screen. 
        The example below sets it to 60% of the screen's width and 50% of its hieght while 
        centering it. Note that you can and SHOULD use the initial values of shell.details. We also recommend you store
        them as once you call finite_window_set_size those values are overwritten.
    */

    FiniteWindowInfo *det = myShell->details;
    int32_t true_width = det->width;
    int32_t true_height = det->height;

    finite_window_size_set(myShell, ((true_width * 20) / 100), ((true_height *25) / 100), ((true_width * 60) / 100), ((true_height *50) / 100));
    int32_t width = det->width;
    int32_t height = det->height;
    // allocate the shared memory buffer
    // second param is false unless using alpha
    finite_shm_alloc(myShell, false);

    // now draw

    // lets create a red box (#d33f49)
    // colors are RGBA values where RGB is value/255 and A is a double between 1 and 0 where 1 is transparent
    // ? You can pass decimals but for clearity, the math to get aforementioned decimals is shown
    FiniteColorGroup myColor = {
        .r = 211.0/255.0,
        .g = 63.0/255.0,
        .b = 73.0/255.0
    };

    // !! VERY IMPORTANT
    // finite_draw_rect supports both solid colors and gradients but you CANNOT have both values set. One of the last two params MUST be NULL.
    // in this example we use a solid color so the gradient value is NULL
    finite_draw_rect(myShell, 0,0, width, height, &myColor, NULL);

    // now lets create some text
    finite_draw_set_font(myShell, "KunbhSans", false, false, 24);
    FiniteColorGroup white = {
        .r = 0,
        .g = 0,
        .b = 0
    }; 
    finite_draw_set_draw_position(myShell, 30, 20);
    finite_draw_set_text(myShell, "Hello there!", &white);

    // when done rendering clean it up and commit with finite_draw_finish()
    // the last param should only be true if you enabled alpha earlier
    bool success = finite_draw_finish(myShell, width, height, myShell->stride, false);
    if (!success) {
        FINITE_LOG_ERROR("Something went wrong.\n");
        free(myShell);
    }

    // now just keep the window alive
    while (wl_display_dispatch(myShell->display) != -1) {}
    finite_draw_cleanup(myShell);    
    wl_display_disconnect(myShell->display);
}
