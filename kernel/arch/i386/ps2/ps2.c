#include "ps2.h"

#include <stdint.h>
#include <stdio.h>

#include "idt.h"
#include "interrupt.h"
#include "pic.h"
#include "port.h"

enum ps2_port : uint16_t {
        PS2_DATA_PORT    = 0x60,
        PS2_STATUS_PORT  = 0x64,
        PS2_COMMAND_PORT = 0x64,
};

enum : uint8_t {
        PS2_STATUS_OB_FULL       = 0x1 << 0,
        PS2_STATUS_IB_FULL       = 0x1 << 1,
        PS2_STATUS_SYS_FLAG      = 0x1 << 2,
        PS2_STATUS_COMMAND       = 0x1 << 3,
        PS2_STATUS_TIMEOUT_ERROR = 0x1 << 6,
        PS2_STATUS_PARITY_ERROR  = 0x1 << 7,
};

enum : uint8_t {
        PS2_COMMAND_READ_BYTE_N     = 0x20,
        PS2_COMMAND_READ_CONFIG     = PS2_COMMAND_READ_BYTE_N + 0,
        PS2_COMMAND_WRITE_BYTE_N    = 0x60,
        PS2_COMMAND_WRITE_CONFIG    = PS2_COMMAND_WRITE_BYTE_N + 0,
        PS2_COMMAND_DISABLE_PORT2   = 0xA7,
        PS2_COMMAND_ENABLE_PORT2    = 0xA8,
        PS2_COMMAND_DISABLE_PORT1   = 0xAD,
        PS2_COMMAND_ENABLE_PORT1    = 0xAE,
        PS2_COMMAND_CONTROLLER_TEST = 0xAA,
        PS2_COMMAND_PORT1_TEST      = 0xAB,
        PS2_COMMAND_PORT2_TEST      = 0xA9,
        PS2_COMMAND_RESET           = 0xFF,
        PS2_COMMAND_WRITE_TO_PORT2  = 0xD4,
};

enum : uint8_t {
        PS2_CONTROL_PORT1_INTERRUPT_ENABLED   = 0x1 << 0,
        PS2_CONTROL_PORT2_INTERRUPT_ENABLED   = 0x1 << 1,
        PS2_CONTROL_SYSTEM_FLAG               = 0x1 << 2,
        PS2_CONTROL_PORT1_CLOCK_DISABLED      = 0x1 << 4,
        PS2_CONTROL_PORT2_CLOCK_DISABLED      = 0x1 << 5,
        PS2_CONTROL_PORT1_TRANSLATION_ENABLED = 0x1 << 6,
};

enum : uint8_t {
        PS2_CONTROLLER_TEST_PASSED = 0x55,
        PS2_CONTROLLER_TEST_FAILED = 0xFC,
        PS2_PORT_TEST_PASSED       = 0x00,
        PS2_PORT_TEST_CLOCK_LOW    = 0x01,
        PS2_PORT_TEST_CLOCK_HIGH   = 0x02,
        PS2_PORT_TEST_DATA_LOW     = 0x03,
        PS2_PORT_TEST_DATA_HIGH    = 0x04,
};

static uint8_t ports_enabled[2] = {};
static uint8_t read_byte        = 0;

static uint8_t ps2_ob_full(void)
{
        return inb(PS2_STATUS_PORT) & PS2_STATUS_OB_FULL;
}

static uint8_t ps2_ib_full(void)
{
        return inb(PS2_STATUS_PORT) & PS2_STATUS_IB_FULL;
}

static uint8_t ps2_inb(enum ps2_port port)
{
        if (port == PS2_STATUS_PORT)
                return inb(port);
        for (; !ps2_ob_full();)
                __asm__("pause");

        return inb(port);
}

static uint8_t ps2_outb(enum ps2_port port, uint8_t data, uint8_t allow_read)
{
        for (uint8_t i = 0; ps2_ib_full(); ++i) {
                if (i == 5) {
                        printf(
                            "Failed to write to PS/2 port because input buffer is still set\n");
                        return 1;
                }
                if (allow_read)
                        ps2_inb(PS2_DATA_PORT);
                __asm__("pause");
        }
        outb(port, data);
        outb(0x80, 0);
        return 0;
}

static void interrupt_irq_1(struct interrupt_stack_frame *frame)
{
        (void)frame;
        read_byte = 1;
        pic_send_eoi(1);
}

static uint8_t read_config(uint8_t allow_read)
{
        ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_READ_CONFIG, allow_read);
        return ps2_inb(PS2_DATA_PORT);
}

static void set_config(uint8_t config_byte, uint8_t allow_read)
{
        ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_WRITE_CONFIG, allow_read);
        ps2_outb(PS2_DATA_PORT, config_byte, allow_read);
}

static uint8_t test_port(uint8_t port)
{
        uint8_t test_command =
            port == 1 ? PS2_COMMAND_PORT1_TEST : PS2_COMMAND_PORT2_TEST;

        ps2_outb(PS2_COMMAND_PORT, test_command, 1);
        switch (ps2_inb(PS2_DATA_PORT)) {
        case PS2_PORT_TEST_PASSED:
                return 0;
        case PS2_PORT_TEST_CLOCK_LOW:
        case PS2_PORT_TEST_CLOCK_HIGH:
        case PS2_PORT_TEST_DATA_LOW:
        case PS2_PORT_TEST_DATA_HIGH:
        default:
                printf("Error with PS/2 port %d\n", port);
                return 1;
        }
}

void ps2_send_byte(uint8_t byte, uint8_t port)
{
        if (port > 2 || port < 1 || !ports_enabled[port - 1]) {
                printf("PS/2 port %u is not available\n", port);
                return;
        }

        if (port == 2)
                ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_WRITE_TO_PORT2, 0);

        ps2_outb(PS2_DATA_PORT, byte, 0);
}

uint8_t ps2_receive_byte(void)
{
        for (; !read_byte;)
                __asm__("hlt");

        read_byte = 0;
        return ps2_inb(PS2_DATA_PORT);
}

static uint8_t reset_device(uint8_t port)
{
        ps2_send_byte(PS2_COMMAND_RESET, port);

        for (uint8_t i = 0; i < 2; ++i) {
                uint8_t b = ps2_inb(PS2_DATA_PORT);
                if (b != 0xFA && b != 0xAA) {
                        printf("Failed to reset PS/2 port %u\n", port);
                        return 1;
                }
        }
        uint8_t ret = 0;
        char *device;
        if (!(inb(PS2_STATUS_PORT) & PS2_STATUS_OB_FULL)) {
                device = "Ancient AT keyboard";
                goto end;
        }

        switch (ps2_inb(PS2_DATA_PORT)) {
        case 0x00:
                device = "Standard PS/2 mouse";
                break;
        case 0x03:
                device = "Mouse with scroll wheel";
                break;
        case 0x04:
                device = "5-button mouse";
                break;
        case 0xAB:
                switch (ps2_inb(PS2_DATA_PORT)) {
                case 0x83:
                case 0xC1:
                        device = "MF2 keyboard";
                        break;
                case 0x84:
                        device = "IBM ThinkPad";
                        break;
                case 0x85:
                        device = "NCD N-97 keyboard";
                        break;
                case 0x86:
                        device = "122-key keyboard";
                        break;
                case 0x90:
                        device = "Japanese G keyboard";
                        break;
                case 0x91:
                        device = "Japanese P keyboard";
                        break;
                case 0x92:
                        device = "Japanese A keyboard";
                        break;
                default:
                        device = "Unknown device";
                        break;
                }
                break;
        case 0xAC:
                if (ps2_inb(PS2_DATA_PORT) == 0xA1)
                        device = "NCD Sun layout keyboard";
                else
                        device = "Unknown device";
                break;
        default:
                device = "Unknown device";
                ret    = 1;
                break;
        }

end:
        printf("Detected: '%s' in port %u\n", device, port);
        return ret;
}

uint8_t ps2_init(void)
{
        // Disable devices
        ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_DISABLE_PORT1, 1);
        ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_DISABLE_PORT2, 1);

        pic_send_eoi(1);

        // Flush buffer
        inb(PS2_DATA_PORT);

        pic_send_eoi(1);

        // Disable irq, clock, and translation for port 1
        uint8_t config_byte = read_config(1);
        config_byte &= ~PS2_CONTROL_PORT1_INTERRUPT_ENABLED &
                       ~PS2_CONTROL_PORT1_CLOCK_DISABLED &
                       ~PS2_CONTROL_PORT1_TRANSLATION_ENABLED;
        set_config(config_byte, 1);

        pic_send_eoi(1);

        // Perform self test
        config_byte = read_config(1);
        ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_CONTROLLER_TEST, 1);
        if (ps2_inb(PS2_DATA_PORT) != PS2_CONTROLLER_TEST_PASSED) {
                puts("PS/2 self test failed");
                return 1;
        }
        set_config(config_byte, 1);

        pic_send_eoi(1);

        // Check for 2 channels
        ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_ENABLE_PORT2, 1);
        uint8_t is_dual_channel =
            !(read_config(1) & PS2_CONTROL_PORT2_CLOCK_DISABLED);
        if (is_dual_channel) {
                ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_DISABLE_PORT2, 1);
                config_byte = read_config(1);
                config_byte &= ~PS2_CONTROL_PORT2_INTERRUPT_ENABLED &
                               ~PS2_CONTROL_PORT2_CLOCK_DISABLED;
                set_config(config_byte, 1);
        }

        pic_send_eoi(1);

        // Test interfaces
        uint8_t port1_enabled = !test_port(1);
        uint8_t port2_enabled = is_dual_channel ? !test_port(2) : 0;

        if (!port1_enabled && !port2_enabled) {
                puts("Failed to initialise any ports");
                return 1;
        }

        pic_send_eoi(1);

        // Enable devices
        if (port1_enabled)
                ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_ENABLE_PORT1, 1);
        if (port2_enabled)
                ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_ENABLE_PORT2, 1);

        config_byte = read_config(1);
        if (port1_enabled)
                config_byte |= PS2_CONTROL_PORT1_INTERRUPT_ENABLED;
        if (port2_enabled)
                config_byte |= PS2_CONTROL_PORT2_INTERRUPT_ENABLED;

        set_config(config_byte, 1);
        ports_enabled[0] = port1_enabled;
        ports_enabled[1] = port2_enabled;

        pic_send_eoi(1);

        // Reset devices
        if (ports_enabled[0])
                reset_device(1);
        if (ports_enabled[1])
                reset_device(2);

        pic_send_eoi(1);

        // Add interrupt irq1
        interrupt_set_handler(PIC_OFFSET + 1, interrupt_irq_1);

        pic_send_eoi(1);

        ps2_inb(PS2_DATA_PORT);

        return 0;
}
