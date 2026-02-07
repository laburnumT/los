#include <ps2.h>

#include <kernel/mouse.h>

enum mouse_code mouse_controller_receive_code(void)
{
        ps2_receive_byte(2);
        return MOUSE_CODE_RECEIVED;
}
