/*
 * Copyright (c) 2020 BedRock Systems, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#pragma once
#include <config.hpp>
#include <drv_ipc.hpp>
#include <rpi_clock.hpp>
#include <rpi_fw.hpp>
#include <rpi_pinctrl.hpp>

static constexpr uint32 CPRMAN_BASE = 0x40000000;
static constexpr uint32 CPRMAN_SIZE = 0x2000;
static constexpr uint32 AUX_BASE = (CPRMAN_BASE + CPRMAN_SIZE);
static constexpr uint32 AUX_SIZE = 0x1000;
static constexpr uint32 MBOX_BASE = (AUX_BASE + AUX_SIZE);
static constexpr uint32 MBOX_SIZE = 0x1000;
static constexpr uint32 GPIO_BASE = (MBOX_BASE + MBOX_SIZE);
static constexpr uint32 GPIO_SIZE = 0x1000;
static constexpr uint32 FW_BASE = (GPIO_BASE + GPIO_SIZE);
static constexpr uint32 FW_SIZE = 0x1000;
static constexpr uint32 DEV_MMIO_END = (FW_BASE + FW_SIZE);

class Rpi4 {
public:
    Errno probe(Pbl::Utcb *utcb, const char *cprman_id, const char *aux_id, const char *mbox_id,
                const char *gpio_id);

    Errno enable_clk(uint64 clk_id);

    Errno get_clkrate(uint64 clk_id, uint64 &value);

    Errno disable_clk(uint64 clk_id);

    Errno set_clkrate(uint64 clk_id, uint64 value);

    uint32 get_max_clkid(void);

    Errno describe_clkrate(uint64 clk_id, Pm::clk_desc &rate);

    bool is_clk_enabled(uint64 clk_id);

    bool is_clk_valid(uint64 clk_id);

    uint32 get_max_nodeid(void);

    Errno enable_node(uint64 node_id);

    Errno disable_node(uint64 node_id);

    Errno handle_pinctrl(Pm::Pin *pins, uint32 num_pins, uint32 func);

    /*use LEDs to signal successful initialization*/
    void success(void);

private:
    /* available devices */
    cprman _clock_manager;
    rpi_pinctrl _pinctrl;
    rpi_fw _fw;
};
