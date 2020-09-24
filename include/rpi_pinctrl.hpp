/*
 * Copyright (c) 2020 BedRock Systems, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#pragma once
#include <pebble/io.hpp>
#include <pm.hpp>

#define NUM_GPIO 58 /*0-57*/

class rpi_pinctrl {
private:
    /*register offsets for 32-Bit accesses*/
    enum {
        GPFSEL0 = 0x00,
        GPFSEL1 = 0x04,
        GPFSEL2 = 0x08,
        GPFSEL3 = 0x0c,
        GPFSEL4 = 0x10,
        GPFSEL5 = 0x14,
        RESERVED0 = 0x18,
        GPSET0 = 0x1c,
        GPSET1 = 0x20,
        RESERVED1 = 0x24,
        GPCLR0 = 0x28,
        GPCLR1 = 0x2c,
        RESERVED2 = 0x30,
        GPLEV0 = 0x34,
        GPLEV1 = 0x38,
        RESERVED3 = 0x3c,
        GPEDS0 = 0x40,
        GPEDS1 = 0x44,
        RESERVED4 = 0x48,
        GPREN0 = 0x4c,
        GPREN1 = 0x50,
        RESERVED5 = 0x54,
        GPFEN0 = 0x58,
        GPFEN1 = 0x5c,
        RESERVED6 = 0x60,
        GPHEN0 = 0x64,
        GPHEN1 = 0x68,
        RESERVED7 = 0x6c,
        GPLEN0 = 0x70,
        GPLEN1 = 0x74,
        RESERVED8 = 0x78,
        GPAREN0 = 0x7c,
        GPAREN1 = 0x80,
        RESERVED9 = 0x84,
        GPAFEN0 = 0x88,
        GPAFEN1 = 0x8c,
        RESERVED10 = 0x90,
        GPIO_PUP_PDN_CNTRL_REG0 = 0xe4,
        GPIO_PUP_PDN_CNTRL_REG1 = 0xe8,
        GPIO_PUP_PDN_CNTRL_REG2 = 0xec,
        GPIO_PUP_PDN_CNTRL_REG3 = 0xf0,
    };

    enum fsel : uint8 {
        GPIO_IN = 0,
        GPIO_OUT = 1,
        GPIO_ATL5 = 2,
        GPIO_ATL4 = 3,
        GPIO_ATL0 = 4,
        GPIO_ATL1 = 5,
        GPIO_ATL2 = 6,
        GPIO_ATL3 = 7,
        GPIO_NUM_FUNC = 8,
    };

    static constexpr uint32 GPIO_FSEL_MASK = 0x7;
    static constexpr uint32 GPIO_FSEL_REG(uint32 pin) { return (GPFSEL0 + ((pin / 10) * 4)); }
    static constexpr uint32 GPIO_FSEL_SHIFT(uint32 pin) { return ((pin % 10) * 3); }
    static constexpr uint32 GPIO_PUP_PDN_REG(uint32 pin) {
        return (GPIO_PUP_PDN_CNTRL_REG0 + ((pin / 16) * 4));
    }
    static constexpr uint32 GPIO_PUP_PDN_SHIFT(uint32 pin) { return ((pin % 16) * 2); }
    static constexpr uint32 GPIO_PUP_PDN_MASK = 0x3;

    static constexpr uint32 GPIO_REG(uint32 base, uint32 pin) { return (base + ((pin / 32) * 4)); }
    static constexpr uint32 GPIO_REG_SHIFT_MASK = 0x1f;
    static constexpr uint32 GPIO_SHIFT(uint32 pin) { return ((pin)&GPIO_REG_SHIFT_MASK); }

    mword _base;

public:
    rpi_pinctrl() { _base = 0; }

    Errno probe(mword base) {
        _base = base;
        return Errno::ENONE;
    }

    Errno set_pin_function(uint32 pin, uint32 val) {
        if (pin >= NUM_GPIO) return Errno::EINVAL;
        outd((_base + GPIO_FSEL_REG(pin)), ((val & GPIO_FSEL_MASK) << GPIO_FSEL_SHIFT(pin)));
        return Errno::ENONE;
    }

    Errno get_pin_function(uint32 pin, uint32 &val) {
        if (pin >= NUM_GPIO) return Errno::EINVAL;
        uint32 reg = ind(_base + GPIO_FSEL_REG(pin));
        val = (reg >> GPIO_FSEL_SHIFT(pin)) & GPIO_FSEL_MASK;
        return Errno::ENONE;
    }

    Errno set_pin_pad(uint32 pin, uint32 val) {
        if (pin >= NUM_GPIO) return Errno::EINVAL;
        outd((_base + GPIO_PUP_PDN_REG(pin)),
             ((val & GPIO_PUP_PDN_MASK) << GPIO_PUP_PDN_SHIFT(pin)));
        return Errno::ENONE;
    }

    Errno get_pin_pad(uint32 pin, uint32 &val) {
        if (pin >= NUM_GPIO) return Errno::EINVAL;
        uint32 reg = ind(_base + GPIO_PUP_PDN_REG(pin));
        val = (reg >> GPIO_PUP_PDN_SHIFT(pin)) & GPIO_PUP_PDN_MASK;
        return Errno::ENONE;
    }

    Errno set_gpio(uint32 pin, uint32 val) {
        if (pin >= NUM_GPIO) return Errno::EINVAL;
        /* check if set or clear */
        uint8 base_reg = (val > 0) ? GPSET0 : GPCLR0;
        outd((_base + GPIO_REG(base_reg, pin)), (1u << (pin & GPIO_REG_SHIFT_MASK)));
        return Errno::ENONE;
    }

    Errno get_gpio(uint32 pin, uint32 &val) {
        if (pin >= NUM_GPIO) return Errno::EINVAL;
        uint32 reg = ind(_base + GPIO_REG(GPLEV0, pin));
        val = (reg >> (pin & GPIO_REG_SHIFT_MASK)) & 1u;
        return Errno::ENONE;
    }

    Errno set_gpio_trigger(uint32 pin, uint32 val) {
        if (pin >= NUM_GPIO) return Errno::EINVAL;
        bool is_clr = (val & Pm::iotrig::TRIG_CLR) > 0;
        uint32 reg;
        Errno err = Errno::ENONE;
        switch (val & Pm::iotrig::TRIG_MASK) {
        case Pm::iotrig::TRIG_NONE:
            /* clear all triggers ?*/
            break;

        case Pm::iotrig::LEVEL_HIGH:
            reg = ind(_base + GPIO_REG(GPHEN0, pin));
            reg = is_clr ? (reg & ~(1u << GPIO_SHIFT(pin))) : (reg | (1u << GPIO_SHIFT(pin)));
            outd((_base + GPIO_REG(GPHEN0, pin)), reg);
            break;
        case Pm::iotrig::LEVEL_LOW:
            reg = ind(_base + GPIO_REG(GPLEN0, pin));
            reg = is_clr ? (reg & ~(1u << GPIO_SHIFT(pin))) : (reg | (1u << GPIO_SHIFT(pin)));
            outd((_base + GPIO_REG(GPLEN0, pin)), reg);
            break;
        case Pm::iotrig::EDGE_FALL:
            reg = ind(_base + GPIO_REG(GPFEN0, pin));
            reg = is_clr ? (reg & ~(1u << GPIO_SHIFT(pin))) : (reg | (1u << GPIO_SHIFT(pin)));
            outd((_base + GPIO_REG(GPFEN0, pin)), reg);
            break;
        case Pm::iotrig::EDGE_RISE:
            reg = ind(_base + GPIO_REG(GPREN0, pin));
            reg = is_clr ? (reg & ~(1u << GPIO_SHIFT(pin))) : (reg | (1u << GPIO_SHIFT(pin)));
            outd((_base + GPIO_REG(GPREN0, pin)), reg);
            break;
        case Pm::iotrig::EDGE_FALL_ASYNC:
            reg = ind(_base + GPIO_REG(GPAFEN0, pin));
            reg = is_clr ? (reg & ~(1u << GPIO_SHIFT(pin))) : (reg | (1u << GPIO_SHIFT(pin)));
            outd((_base + GPIO_REG(GPAFEN0, pin)), reg);
            break;
        case Pm::iotrig::EDGE_RISE_ASYNC:
            reg = ind(_base + GPIO_REG(GPAREN0, pin));
            reg = is_clr ? (reg & ~(1u << GPIO_SHIFT(pin))) : (reg | (1u << GPIO_SHIFT(pin)));
            outd((_base + GPIO_REG(GPAREN0, pin)), reg);
            break;
        default:
            return Errno::EINVAL;
            break;
        }
        return err;
    }

    Errno get_gpio_trigger(uint32 pin, uint32 &val) {
        if (pin >= NUM_GPIO) return Errno::EINVAL;
        uint32 reg;
        val = Pm::iotrig::TRIG_NONE;

        reg = ind(_base + GPIO_REG(GPHEN0, pin));
        if ((reg >> GPIO_SHIFT(pin)) & 1) val |= Pm::iotrig::LEVEL_HIGH;

        reg = ind(_base + GPIO_REG(GPLEN0, pin));
        if ((reg >> GPIO_SHIFT(pin)) & 1) val |= Pm::iotrig::LEVEL_LOW;

        reg = ind(_base + GPIO_REG(GPFEN0, pin));
        if ((reg >> GPIO_SHIFT(pin)) & 1) val |= Pm::iotrig::EDGE_FALL;

        reg = ind(_base + GPIO_REG(GPREN0, pin));
        if ((reg >> GPIO_SHIFT(pin)) & 1) val |= Pm::iotrig::EDGE_RISE;

        reg = ind(_base + GPIO_REG(GPAFEN0, pin));
        if ((reg >> GPIO_SHIFT(pin)) & 1) val |= Pm::iotrig::EDGE_FALL_ASYNC;

        reg = ind(_base + GPIO_REG(GPAREN0, pin));
        if ((reg >> GPIO_SHIFT(pin)) & 1) val |= Pm::iotrig::EDGE_RISE_ASYNC;

        return Errno::ENONE;
    }

    Errno get_gpio_event(uint32 pin, uint32 &val) {
        if (pin >= NUM_GPIO) return Errno::EINVAL;
        uint32 reg;
        reg = ind(_base + GPIO_REG(GPEDS0, pin));
        val = (reg >> GPIO_SHIFT(pin)) & 1;
        return Errno::ENONE;
    }

    Errno clr_gpio_event(uint32 pin) {
        if (pin >= NUM_GPIO) return Errno::EINVAL;
        outd((_base + GPIO_REG(GPEDS0, pin)), (1u << GPIO_SHIFT(pin)));
        return Errno::ENONE;
    }
};
