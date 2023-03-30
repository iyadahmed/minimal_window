#include "nanogui.h"


int main() {
    nano_gui_create_fixed_size_window(800, 600);
    nano_gui_run_main_loop();
    return 0;
}