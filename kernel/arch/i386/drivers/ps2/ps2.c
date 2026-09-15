#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/interrupt.h>

#include <drivers/pic.h>
#include <drivers/port.h>
#include <drivers/ps2.h>

#include "context.h"
#include "idt.h"
#include "interrupt.h"

enum ps2_port : uint8_t {
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

enum {
        PS2_QUEUE_SIZE = 0x1 << 6,
};

struct ps2_queue {
        size_t tail, head;
        unsigned char data[PS2_QUEUE_SIZE];
};

static uint8_t ports_enabled[2]                = {};
static volatile struct ps2_queue ps2_queues[2] = {{}, {}};

static uint8_t ps2_queue_pop(uint8_t port)
{
        auto queue = ps2_queues + port - 1;
        if (queue->tail == queue->head)
                return 0;
        uint8_t ret = queue->data[queue->head];
        queue->head = (queue->head + 1) % PS2_QUEUE_SIZE;
        return ret;
}

static void ps2_queue_push(uint8_t port, uint8_t data)
{
        auto queue       = ps2_queues + port - 1;
        size_t next_head = (queue->tail + 1) % PS2_QUEUE_SIZE;
        if (next_head == queue->head)
                ps2_queue_pop(port);
        queue->data[queue->tail] = data;
        queue->tail              = next_head;
}

static size_t ps2_queue_size(uint8_t port)
{
        auto queue = ps2_queues + port - 1;
        return queue->head <= queue->tail
                   ? queue->tail - queue->head
                   : PS2_QUEUE_SIZE - queue->head - queue->tail;
}

static uint8_t ps2_ob_full(void)
{
        return inb(PS2_STATUS_PORT) & PS2_STATUS_OB_FULL;
}

static uint8_t ps2_ib_full(void)
{
        return inb(PS2_STATUS_PORT) & PS2_STATUS_IB_FULL;
}

static int ps2_inb(enum ps2_port port)
{
        if (port == PS2_STATUS_PORT)
                return inb(port);
        for (size_t timeout = 100000; !ps2_ob_full(); --timeout) {
                if (!timeout)
                        return -1;
                __asm__("pause");
        }

        return inb(port);
}

static uint8_t ps2_outb(enum ps2_port port, uint8_t data)
{
        for (size_t timeout = 100000; ps2_ib_full(); --timeout) {
                if (!timeout) {
                        printf("PS/2 outb timeout on port %u\n", port);
                        return 1;
                }
                __asm__("pause");
        }
        outb(port, data);
        outb(0x80, 0);
        return 0;
}

static void interrupt_irq_1(struct interrupt_stack_frame *frame)
{
        (void)frame;
        ps2_queue_push(1, inb(PS2_DATA_PORT));
        pic_send_eoi(1);
}

static void interrupt_irq_12(struct interrupt_stack_frame *frame)
{
        (void)frame;
        ps2_queue_push(2, inb(PS2_DATA_PORT));
        pic_send_eoi(12);
}

static uint8_t read_config(void)
{
        ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_READ_CONFIG);
        return ps2_inb(PS2_DATA_PORT);
}

static void set_config(uint8_t config_byte)
{
        ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_WRITE_CONFIG);
        ps2_outb(PS2_DATA_PORT, config_byte);
}

static uint8_t test_port(uint8_t port)
{
        uint8_t test_command =
            port == 1 ? PS2_COMMAND_PORT1_TEST : PS2_COMMAND_PORT2_TEST;

        ps2_outb(PS2_COMMAND_PORT, test_command);
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
                ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_WRITE_TO_PORT2);

        ps2_outb(PS2_DATA_PORT, byte);
}

uint8_t ps2_receive_byte(uint8_t port)
{
        if (port > 2 || port < 1 || !ports_enabled[port - 1]) {
                printf("PS/2 port %u is not available\n", port);
                return 0;
        }
        for (; !ps2_queue_size(port);)
                __asm__("hlt");

        return ps2_queue_pop(port);
}

static uint8_t reset_device(uint8_t port)
{
        ps2_send_byte(PS2_COMMAND_RESET, port);

        int ack = ps2_inb(PS2_DATA_PORT);
        if (ack != 0xFA) {
                printf("Failed to reset PS/2 port %u\n", port);
                return 1;
        }
        ack = ps2_inb(PS2_DATA_PORT);
        if (ack >= 0 && ack != 0xAA) {
                printf("PS/2 self test failed port %u\n", port);
                return 1;
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
        uint8_t int_enabled = interrupt_check_enabled();
        if (int_enabled)
                interrupt_disable();
        // Disable devices
        ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_DISABLE_PORT1);
        ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_DISABLE_PORT2);

        // Flush buffer
        ps2_inb(PS2_DATA_PORT);

        // Disable irq, clock, and translation for port 1
        uint8_t config_byte = read_config();
        config_byte &= ~PS2_CONTROL_PORT1_INTERRUPT_ENABLED;
        config_byte &= ~PS2_CONTROL_PORT1_TRANSLATION_ENABLED;
        config_byte |= PS2_CONTROL_PORT1_CLOCK_DISABLED;
        set_config(config_byte);

        // Perform self test
        config_byte = read_config();
        ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_CONTROLLER_TEST);
        if (ps2_inb(PS2_DATA_PORT) != PS2_CONTROLLER_TEST_PASSED) {
                puts("PS/2 self test failed");
                return 1;
        }
        set_config(config_byte);

        // Check for 2 channels
        ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_ENABLE_PORT2);
        uint8_t is_dual_channel =
            !(read_config() & PS2_CONTROL_PORT2_CLOCK_DISABLED);
        if (is_dual_channel) {
                ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_DISABLE_PORT2);
                config_byte = read_config();
                config_byte &= ~PS2_CONTROL_PORT2_INTERRUPT_ENABLED;
                config_byte |= PS2_CONTROL_PORT2_CLOCK_DISABLED;
                set_config(config_byte);
        }

        // Test interfaces
        uint8_t port1_enabled = !test_port(1);
        uint8_t port2_enabled = is_dual_channel ? !test_port(2) : 0;

        if (!port1_enabled && !port2_enabled) {
                puts("Failed to initialise any ports");
                return 1;
        }

        // Enable devices
        if (port1_enabled)
                ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_ENABLE_PORT1);
        if (port2_enabled)
                ps2_outb(PS2_COMMAND_PORT, PS2_COMMAND_ENABLE_PORT2);

        ports_enabled[0] = port1_enabled;
        ports_enabled[1] = port2_enabled;

        // Reset devices
        if (ports_enabled[0])
                reset_device(1);
        if (ports_enabled[1])
                reset_device(2);

        // Add interrupt irqs
        if (ports_enabled[0])
                interrupt_set_handler(PIC_OFFSET + 1, interrupt_irq_1);
        if (ports_enabled[1])
                interrupt_set_handler(PIC_OFFSET + 12, interrupt_irq_12);

        config_byte = read_config();
        if (port1_enabled)
                config_byte |= PS2_CONTROL_PORT1_INTERRUPT_ENABLED;
        if (port2_enabled)
                config_byte |= PS2_CONTROL_PORT2_INTERRUPT_ENABLED;

        set_config(config_byte);

        while (ps2_ob_full())
                inb(PS2_DATA_PORT);

        if (int_enabled)
                interrupt_enable();

        return 0;
}
