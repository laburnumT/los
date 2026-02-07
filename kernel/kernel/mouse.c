#include <kernel/mouse.h>

enum mouse_code mouse_controller_receive_code(void);
enum mouse_code mouse_receive_code(void)
{
        return mouse_controller_receive_code();
}
