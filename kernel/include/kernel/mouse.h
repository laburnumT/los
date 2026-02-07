#ifndef LOS_KERNEL_MOUSE_H
#define LOS_KERNEL_MOUSE_H

enum mouse_code {
        MOUSE_CODE_RECEIVED,
};

enum mouse_code mouse_receive_code(void);

#endif
