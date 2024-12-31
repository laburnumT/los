#include <kernel/ps2.h>

#include <kernel/keyboard.h>
#include <stdint.h>

enum : uint8_t {
        PS2_KEYBOARD_COMAND_SCAN_CODE = 0xF0,
};

enum ps2_keyboard_code {
        PS2_A_PRESS            = 0x1c,
        PS2_B_PRESS            = 0x32,
        PS2_C_PRESS            = 0x21,
        PS2_D_PRESS            = 0x23,
        PS2_E_PRESS            = 0x24,
        PS2_F_PRESS            = 0x2B,
        PS2_G_PRESS            = 0x34,
        PS2_H_PRESS            = 0x33,
        PS2_I_PRESS            = 0x43,
        PS2_J_PRESS            = 0x3B,
        PS2_K_PRESS            = 0x42,
        PS2_L_PRESS            = 0x4B,
        PS2_M_PRESS            = 0x3A,
        PS2_N_PRESS            = 0x31,
        PS2_O_PRESS            = 0x44,
        PS2_P_PRESS            = 0x4D,
        PS2_Q_PRESS            = 0x15,
        PS2_R_PRESS            = 0x2D,
        PS2_S_PRESS            = 0x1B,
        PS2_T_PRESS            = 0x2C,
        PS2_U_PRESS            = 0x3C,
        PS2_V_PRESS            = 0x2A,
        PS2_W_PRESS            = 0x1D,
        PS2_X_PRESS            = 0x22,
        PS2_Y_PRESS            = 0x35,
        PS2_Z_PRESS            = 0x1A,
        PS2_0_PRESS            = 0x45,
        PS2_1_PRESS            = 0x16,
        PS2_2_PRESS            = 0x1E,
        PS2_3_PRESS            = 0x26,
        PS2_4_PRESS            = 0x25,
        PS2_5_PRESS            = 0x2E,
        PS2_6_PRESS            = 0x36,
        PS2_7_PRESS            = 0x3D,
        PS2_8_PRESS            = 0x3E,
        PS2_9_PRESS            = 0x46,
        PS2_BACKTICK_PRESS     = 0x0E,
        PS2_MIN_PRESS          = 0x4E,
        PS2_EQUAL_PRESS        = 0x55,
        PS2_SQUARE_L_PRESS     = 0x54,
        PS2_SQUARE_R_PRESS     = 0x5B,
        PS2_BACKSLASH_PRESS    = 0x5D,
        PS2_SLASH_PRESS        = 0x4A,
        PS2_COMMA_PRESS        = 0x41,
        PS2_DOT_PRESS          = 0x49,
        PS2_SEMICOLON_PRESS    = 0x4C,
        PS2_APOSTRAPHE_PRESS   = 0x52,
        PS2_SPACE_PRESS        = 0x29,
        PS2_TAB_PRESS          = 0x0D,
        PS2_ENTER_PRESS        = 0x5A,
        PS2_BKSP_PRESS         = 0x66,
        PS2_SHIFT_L_PRESS      = 0x12,
        PS2_SHIFT_R_PRESS      = 0x59,
        PS2_CAPS_PRESS         = 0x58,
};

static const enum keyboard_code scan2_key_map[] = {
    [PS2_A_PRESS]            = KEYBOARD_A_PRESS,
    [PS2_B_PRESS]            = KEYBOARD_B_PRESS,
    [PS2_C_PRESS]            = KEYBOARD_C_PRESS,
    [PS2_D_PRESS]            = KEYBOARD_D_PRESS,
    [PS2_E_PRESS]            = KEYBOARD_E_PRESS,
    [PS2_F_PRESS]            = KEYBOARD_F_PRESS,
    [PS2_G_PRESS]            = KEYBOARD_G_PRESS,
    [PS2_H_PRESS]            = KEYBOARD_H_PRESS,
    [PS2_I_PRESS]            = KEYBOARD_I_PRESS,
    [PS2_J_PRESS]            = KEYBOARD_J_PRESS,
    [PS2_K_PRESS]            = KEYBOARD_K_PRESS,
    [PS2_L_PRESS]            = KEYBOARD_L_PRESS,
    [PS2_M_PRESS]            = KEYBOARD_M_PRESS,
    [PS2_N_PRESS]            = KEYBOARD_N_PRESS,
    [PS2_O_PRESS]            = KEYBOARD_O_PRESS,
    [PS2_P_PRESS]            = KEYBOARD_P_PRESS,
    [PS2_Q_PRESS]            = KEYBOARD_Q_PRESS,
    [PS2_R_PRESS]            = KEYBOARD_R_PRESS,
    [PS2_S_PRESS]            = KEYBOARD_S_PRESS,
    [PS2_T_PRESS]            = KEYBOARD_T_PRESS,
    [PS2_U_PRESS]            = KEYBOARD_U_PRESS,
    [PS2_V_PRESS]            = KEYBOARD_V_PRESS,
    [PS2_W_PRESS]            = KEYBOARD_W_PRESS,
    [PS2_X_PRESS]            = KEYBOARD_X_PRESS,
    [PS2_Y_PRESS]            = KEYBOARD_Y_PRESS,
    [PS2_Z_PRESS]            = KEYBOARD_Z_PRESS,
    [PS2_0_PRESS]            = KEYBOARD_0_PRESS,
    [PS2_1_PRESS]            = KEYBOARD_1_PRESS,
    [PS2_2_PRESS]            = KEYBOARD_2_PRESS,
    [PS2_3_PRESS]            = KEYBOARD_3_PRESS,
    [PS2_4_PRESS]            = KEYBOARD_4_PRESS,
    [PS2_5_PRESS]            = KEYBOARD_5_PRESS,
    [PS2_6_PRESS]            = KEYBOARD_6_PRESS,
    [PS2_7_PRESS]            = KEYBOARD_7_PRESS,
    [PS2_8_PRESS]            = KEYBOARD_8_PRESS,
    [PS2_9_PRESS]            = KEYBOARD_9_PRESS,
    [PS2_BACKTICK_PRESS]     = KEYBOARD_BACKTICK_PRESS,
    [PS2_MIN_PRESS]          = KEYBOARD_MIN_PRESS,
    [PS2_EQUAL_PRESS]        = KEYBOARD_EQUAL_PRESS,
    [PS2_SQUARE_L_PRESS]     = KEYBOARD_SQUARE_L_PRESS,
    [PS2_SQUARE_R_PRESS]     = KEYBOARD_SQUARE_R_PRESS,
    [PS2_BACKSLASH_PRESS]    = KEYBOARD_BACKSLASH_PRESS,
    [PS2_SLASH_PRESS]        = KEYBOARD_SLASH_PRESS,
    [PS2_COMMA_PRESS]        = KEYBOARD_COMMA_PRESS,
    [PS2_DOT_PRESS]          = KEYBOARD_DOT_PRESS,
    [PS2_SEMICOLON_PRESS]    = KEYBOARD_SEMICOLON_PRESS,
    [PS2_APOSTRAPHE_PRESS]   = KEYBOARD_APOSTRAPHE_PRESS,
    [PS2_SPACE_PRESS]        = KEYBOARD_SPACE_PRESS,
    [PS2_TAB_PRESS]          = KEYBOARD_TAB_PRESS,
    [PS2_ENTER_PRESS]        = KEYBOARD_ENTER_PRESS,
    [PS2_BKSP_PRESS]         = KEYBOARD_BKSP_PRESS,
    [PS2_SHIFT_L_PRESS]      = KEYBOARD_SHIFT_L_PRESS,
    [PS2_SHIFT_R_PRESS]      = KEYBOARD_SHIFT_R_PRESS,
    [PS2_CAPS_PRESS]         = KEYBOARD_CAPS_PRESS,
};

enum keyboard_code ps2_receive_key(void)
{
        for (;;) {
                uint8_t code = ps2_receive_byte();
                if (code == 0xF0)
                        return scan2_key_map[ps2_receive_byte()] +
                               KEYBOARD_A_RELEASE;

                return scan2_key_map[code];
        }
}

uint8_t ps2_scan_code_get(void)
{
        do {
                ps2_send_byte(PS2_KEYBOARD_COMAND_SCAN_CODE, 1);
                ps2_send_byte(0x00, 1);
        } while (ps2_receive_byte() != 0xFA);
        return ps2_receive_byte();
}

void ps2_scan_code_set(uint8_t set)
{
        if (set < 1 || set > 3)
                return;
        do {
                ps2_send_byte(PS2_KEYBOARD_COMAND_SCAN_CODE, 1);
                ps2_send_byte(set, 1);
        } while (ps2_receive_byte() != 0xFA);
}
