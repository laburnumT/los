#include <stdint.h>

#include <kernel/keyboard.h>

static uint8_t shift_l_pressed = 0;
static uint8_t shift_r_pressed = 0;
static uint8_t caps_lock       = 0;

static char shift_promotion[] = {
    ['0'] = ')',  ['1'] = '!', ['2'] = '@',  ['3'] = '#', ['4'] = '$',
    ['5'] = '%',  ['6'] = '^', ['7'] = '&',  ['8'] = '*', ['9'] = '(',
    ['`'] = '~',  ['-'] = '_', ['='] = '+',  ['['] = '{', [']'] = '}',
    ['\\'] = '|', [';'] = ':', ['\''] = '"', [','] = '<', ['.'] = '>',
    ['/'] = '?',
};

static uint8_t is_printable(enum keyboard_code c)
{
        return (c >= KEYBOARD_A_PRESS && c < KEYBOARD_SHIFT_L_PRESS) ||
               (c >= KEYBOARD_A_RELEASE && c < KEYBOARD_SHIFT_L_RELEASE);
}

enum keyboard_code keyboard_controller_receive_key(void);
enum keyboard_code keyboard_receive_key(void)
{
        enum keyboard_code code = keyboard_controller_receive_key();
        if (code == KEYBOARD_SHIFT_L_PRESS)
                shift_l_pressed = 1;
        if (code == KEYBOARD_SHIFT_L_RELEASE)
                shift_l_pressed = 0;
        if (code == KEYBOARD_SHIFT_R_PRESS)
                shift_r_pressed = 1;
        if (code == KEYBOARD_SHIFT_R_RELEASE)
                shift_r_pressed = 0;
        if (code == KEYBOARD_CAPS_PRESS)
                caps_lock = !caps_lock;
        return code;
}

int keyboard_getchar(void)
{
        enum keyboard_code code;
        do {
                code = keyboard_receive_key();
        } while (!is_printable(code));

        // Assumes a-z is contiguous
        if (code >= KEYBOARD_A_PRESS && code <= KEYBOARD_Z_PRESS) {
                const char start =
                    caps_lock != (shift_l_pressed || shift_r_pressed) ? 'A'
                                                                      : 'a';
                return (int)(start + (code - KEYBOARD_A_PRESS));
        }
        if (code >= KEYBOARD_0_PRESS && code <= KEYBOARD_9_PRESS) {
                int ret = (int)('0' + (code - KEYBOARD_0_PRESS));
                if (shift_l_pressed || shift_r_pressed)
                        return shift_promotion[ret];
                return ret;
        }
        if (code == KEYBOARD_SPACE_PRESS)
                return ' ';
        if (code == KEYBOARD_TAB_PRESS)
                return '\t';
        if (code == KEYBOARD_ENTER_PRESS)
                return '\n';
        if (code == KEYBOARD_BKSP_PRESS)
                return '\b';

        int ret;
        switch (code) {
        case KEYBOARD_BACKTICK_PRESS:
                ret = '`';
                break;
        case KEYBOARD_MIN_PRESS:
                ret = '-';
                break;
        case KEYBOARD_EQUAL_PRESS:
                ret = '=';
                break;
        case KEYBOARD_SQUARE_L_PRESS:
                ret = '[';
                break;
        case KEYBOARD_SQUARE_R_PRESS:
                ret = ']';
                break;
        case KEYBOARD_BACKSLASH_PRESS:
                ret = '\\';
                break;
        case KEYBOARD_SLASH_PRESS:
                ret = '/';
                break;
        case KEYBOARD_COMMA_PRESS:
                ret = ',';
                break;
        case KEYBOARD_DOT_PRESS:
                ret = '.';
                break;
        case KEYBOARD_SEMICOLON_PRESS:
                ret = ';';
                break;
        case KEYBOARD_APOSTRAPHE_PRESS:
                ret = '\'';
                break;
        default:
                return -1;
        }

        if (shift_l_pressed || shift_r_pressed)
                return shift_promotion[ret];
        return ret;
}
