/*
 * Copyright (c) 2020 BedRock Systems, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <pebble/pebble.hpp>
#include <rpi4.hpp>

/* since MSC gives us page-aligned address */
static constexpr uint32 RPI4_FW_MBOX_OFFSET = 0x880;

Errno
Rpi4::probe(Pbl::Utcb *utcb, const char *cprman_id, const char *aux_id, const char *mbox_id,
            const char *gpio_id) {

    Sel clock_base(CPRMAN_BASE), aux_base(AUX_BASE), mbox_base(MBOX_BASE), gpio_base(GPIO_BASE);
    Errno err
        = Pbl::API::acquire_resource(utcb, cprman_id, Pbl::API::RES_REG, 0, clock_base, 0, false);
    if (err != Errno::ENONE) return err;

    err = Pbl::API::acquire_resource(utcb, aux_id, Pbl::API::RES_REG, 0, aux_base, 0, false);
    if (err != Errno::ENONE) return err;

    err = Pbl::API::acquire_resource(utcb, mbox_id, Pbl::API::RES_REG, 0, mbox_base, 0, false);
    if (err != Errno::ENONE) return err;

    err = Pbl::API::acquire_resource(utcb, gpio_id, Pbl::API::RES_REG, 0, gpio_base, 0, false);
    if (err != Errno::ENONE) return err;

    err = _clock_manager.probe(clock_base, aux_base);
    if (err != Errno::ENONE) return err;

    err = _pinctrl.probe(gpio_base);
    if (err != Errno::ENONE) return err;

    /* set up 1 page of uncached buffer space for communication with the firmware*/
    mword fw_shmem_va(FW_BASE), fw_shmem_pa;
    err = Pbl::API::dma_mmap(utcb, fw_shmem_va, PAGE_SIZE, 0xd, false, fw_shmem_pa);

    if (err != Errno::ENONE) {
        return err;
    }

    _fw.init(reinterpret_cast<void *>(mbox_base + RPI4_FW_MBOX_OFFSET),
             reinterpret_cast<void *>(fw_shmem_va), PAGE_SIZE,
             reinterpret_cast<void *>(fw_shmem_pa));

    _fw.set_gpio(130, 1); /* Firmware power LED off, checkpoint*/

    return err;
}

Errno
Rpi4::enable_clk(uint64 clk_id) {
    rpi_clock *clk = _clock_manager.get_clock(static_cast<uint8>(clk_id));
    if (!clk) return Errno::EINVAL;
    if (clk->is_prepared()) return Errno::ENONE;
    return clk->prepare();
}

Errno
Rpi4::get_clkrate(uint64 clk_id, uint64 &value) {
    rpi_clock *clk = _clock_manager.get_clock(static_cast<uint8>(clk_id));
    if (!clk) return Errno::EINVAL;
    value = clk->get_rate();
    return Errno::ENONE;
}

Errno
Rpi4::disable_clk(uint64 clk_id) {
    rpi_clock *clk = _clock_manager.get_clock(static_cast<uint8>(clk_id));
    if (!clk) return Errno::EINVAL;
    return clk->unprepare();
}

Errno
Rpi4::set_clkrate(uint64 clk_id, uint64 value) {
    rpi_clock *clk = _clock_manager.get_clock(static_cast<uint8>(clk_id));
    if (!clk) return Errno::EINVAL;
    return clk->set_rate(value);
}

uint32
Rpi4::get_max_clkid(void) {
    return _clock_manager.get_max_clock();
}

bool
Rpi4::is_clk_enabled(uint64 clk_id) {
    rpi_clock *clk = _clock_manager.get_clock(static_cast<uint8>(clk_id));
    if (!clk) return false;
    return clk->is_prepared();
}

bool
Rpi4::is_clk_valid(uint64 clk_id) {
    rpi_clock *clk = _clock_manager.get_clock(static_cast<uint8>(clk_id));
    if (clk)
        return true;
    else
        return false;
}

Errno
Rpi4::describe_clkrate(uint64 clk_id, Pm::clk_desc &rate) {
    rpi_clock *clk = _clock_manager.get_clock(static_cast<uint8>(clk_id));
    if (!clk)
        return Errno::EINVAL;
    else
        return clk->describe_rate(rate);
}

void
Rpi4::success() {
    /*Green LED ON*/
    _pinctrl.set_gpio(42, 1);
}

/* The undocumented firmware GPIO interface is not exposed to clients.
    currently, only the RED LED is known to be under firmware control */
Errno
Rpi4::handle_pinctrl(Pm::Pin *pins, uint32 num_pins, uint32 func) {
    Errno err = Errno::EINVAL;

    for (uint8 i = 0; i < num_pins; i++) {

        switch (func) {
        case PM_SET_PINFUNC:
            err = _pinctrl.set_pin_function(pins[i].id, pins[i].val);
            break;
        case PM_GET_PINFUNC:
            err = _pinctrl.get_pin_function(pins[i].id, pins[i].val);
            break;
        case PM_SET_PINPAD:
            err = _pinctrl.set_pin_pad(pins[i].id, pins[i].val);
            break;
        case PM_GET_PINPAD:
            err = _pinctrl.get_pin_pad(pins[i].id, pins[i].val);
            break;
        case PM_SET_GPIO:
            err = _pinctrl.set_gpio(pins[i].id, pins[i].val);
            break;
        case PM_GET_GPIO:
            err = _pinctrl.get_gpio(pins[i].id, pins[i].val);
            break;
        case PM_GET_GPIOTRIG:
            err = _pinctrl.get_gpio_trigger(pins[i].id, pins[i].val);
            break;
        case PM_SET_GPIOTRIG:
            err = _pinctrl.set_gpio_trigger(pins[i].id, pins[i].val);
            break;
        case PM_GET_GPIOEVT:
            err = _pinctrl.get_gpio_event(pins[i].id, pins[i].val);
            break;
        case PM_CLR_GPIOEVT:
            err = _pinctrl.clr_gpio_event(pins[i].id);
            break;

        default:
            err = Errno::ENOTSUP;
            break;
        }
    }
    return err;
}

uint32
Rpi4::get_max_nodeid(void) {
    return RPI_POWER_DOMAIN_COUNT;
}

Errno
Rpi4::enable_node(uint64 node_id) {
    if (node_id > RPI_POWER_DOMAIN_COUNT) return Errno::EINVAL;

    return _fw.set_power_domain(static_cast<uint32>(node_id), 1);
}

Errno
Rpi4::disable_node(uint64 node_id) {
    if (node_id > RPI_POWER_DOMAIN_COUNT) return Errno::EINVAL;

    return _fw.set_power_domain(static_cast<uint32>(node_id), 0);
}