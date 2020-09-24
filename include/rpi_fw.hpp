/*
 * Copyright (c) 2020 BedRock Systems, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 * This implementation is derived from u-boot sources with the following copyright headers-
 * Copyright (C) 2012 Stephen Warren
 */
#include <pebble/io.hpp>
#include <pm.hpp>
#include <raspberrypi-power.h>
#include <rpi_mbox.h>

struct fw_gpio_msg {
    struct bcm2835_mbox_hdr hdr;
    struct bcm2835_mbox_tag_gpio_state fw_gpio;
    uint32 end_tag;
};

struct fw_gpio_set_config_msg {
    struct bcm2835_mbox_hdr hdr;
    struct bcm2835_mbox_tag_gpio_set_config fw_gpio;
    uint32 end_tag;
};

struct fw_gpio_get_config_msg {
    struct bcm2835_mbox_hdr hdr;
    struct bcm2835_mbox_tag_gpio_get_config fw_gpio;
    uint32 end_tag;
};

struct fw_pd_msg {
    struct bcm2835_mbox_hdr hdr;
    struct bcm2835_mbox_tag_set_power_state fw_pd;
    uint32 end_tag;
};

struct clk_rate_msg {
    struct bcm2835_mbox_hdr hdr;
    struct bcm2835_mbox_tag_get_clock_rate fw_clk;
    uint32 end_tag;
};

class rpi_fw {
public:
    struct bcm2835_mbox_regs *_mbox;
    void *_buffer;
    void *_buffer_pa;
    uint32 _buf_size;

    /**
     * VPU is a 32-bit processor, buffer used to communicate with it
     * must be 32-bit addressable
     */
    inline uint64 phys_to_bus(uint64 phy_addr) { return 0xc0000000 | phy_addr; }

    inline uint64 bus_to_phys(uint64 bus_addr) { return bus_addr & ~0xc0000000; }

    /**
     * synchronous mailbox interface, used to communicate
     * the address of the property interface protocol
     * buffer
     */
    Errno call_fw_raw(uint32 &ret) {
        uint64 buf = phys_to_bus(reinterpret_cast<uint64>(_buffer_pa));

        if (buf & BCM2835_CHAN_MASK) {
            return Errno::EINVAL;
        }

        /* clear unprocessed firmware responses, if any */
        while (true) {
            uint32 val = ind(reinterpret_cast<mword>(&_mbox->mail0_status));
            if (val & BCM2835_MBOX_STATUS_RD_EMPTY) break;
            val = ind(reinterpret_cast<mword>(&_mbox->read));
        }

        /* wait for channel to be free */
        while (true) {
            uint32 val = ind(reinterpret_cast<mword>(&_mbox->mail1_status));
            if (!(val & BCM2835_MBOX_STATUS_WR_FULL)) break;
        }

        /* send command */
        uint32 data = BCM2835_MBOX_PACK(BCM2835_MBOX_PROP_CHAN, static_cast<uint32>(buf));
        outd(reinterpret_cast<mword>(&_mbox->write), data);

        /* wait for response */
        while (true) {
            uint32 reg = ind(reinterpret_cast<mword>(&_mbox->mail0_status));
            if (!(reg & BCM2835_MBOX_STATUS_RD_EMPTY)) break;
        }

        uint32 resp = ind(reinterpret_cast<mword>(&_mbox->read));
        if (BCM2835_MBOX_UNPACK_CHAN(resp) != BCM2835_MBOX_PROP_CHAN) {
            return Errno::ENOTSUP; /*response not for our request*/
        }

        ret = BCM2835_MBOX_UNPACK_DATA(resp);
        return Errno::ENONE;
    }

    /**
     * assumption: the buffer passed is non-cached and the
     * address is physical.
     */
    Errno call_fw_prop(void) {
        uint32 resp;
        Errno err = call_fw_raw(resp);
        if (err != Errno::ENONE) return err;

        if (resp != phys_to_bus(reinterpret_cast<uint64>(_buffer_pa))) {
            return Errno::ETIMEDOUT;
        }

        struct bcm2835_mbox_hdr *buf = reinterpret_cast<struct bcm2835_mbox_hdr *>(_buffer);
        if (buf->code != BCM2835_MBOX_RESP_CODE_SUCCESS) {
            return Errno::ENOTSUP;
        }

        return err;
    }

    rpi_fw(void) {}

    void init(void *mbox_base, void *buf_addr, uint32 buf_size, void *buf_paddr) {
        _mbox = static_cast<struct bcm2835_mbox_regs *>(mbox_base);
        _buffer = buf_addr;
        _buf_size = buf_size;
        _buffer_pa = buf_paddr;
    }

    Errno set_gpio(uint32 gpio, uint32 val) {
        Errno err = Errno::ENONE;
        struct fw_gpio_msg *msg = reinterpret_cast<struct fw_gpio_msg *>(_buffer);
        BCM2835_MBOX_INIT_HDR(msg);
        BCM2835_MBOX_INIT_TAG(&msg->fw_gpio, SET_GPIO_STATE);
        msg->fw_gpio.body.req.gpio_id = gpio;
        msg->fw_gpio.body.req.state = val;
        err = call_fw_prop();
        return err;
    }

    Errno set_pin_function(uint32, uint32) { return Errno::ENONE; }

    Errno set_power_domain(uint32 pd, uint32 val) {
        Errno err = Errno::ENONE;
        struct fw_pd_msg *msg = reinterpret_cast<struct fw_pd_msg *>(_buffer);
        BCM2835_MBOX_INIT_HDR(msg);
        BCM2835_MBOX_INIT_TAG(&msg->fw_pd, SET_POWER_STATE);
        msg->fw_pd.body.req.device_id = pd;
        msg->fw_pd.body.req.state = val;
        err = call_fw_prop();
        return err;
    }

    Errno get_power_domain(uint32 pd) {
        Errno err = Errno::ENONE;
        struct fw_pd_msg *msg = reinterpret_cast<struct fw_pd_msg *>(_buffer);
        BCM2835_MBOX_INIT_HDR(msg);
        BCM2835_MBOX_INIT_TAG(&msg->fw_pd, GET_POWER_STATE);
        msg->fw_pd.body.req.device_id = pd;
        err = call_fw_prop();
        if (msg->fw_pd.body.resp.state != 0) return Errno::ENOTSUP;
        return err;
    }

    Errno get_clk_rate(uint32 clk_id) {
        Errno err = Errno::ENONE;
        struct clk_rate_msg *msg = reinterpret_cast<struct clk_rate_msg *>(_buffer);
        BCM2835_MBOX_INIT_HDR(msg);
        BCM2835_MBOX_INIT_TAG(&msg->fw_clk, GET_POWER_STATE);
        msg->fw_clk.body.req.clock_id = clk_id;
        msg->fw_clk.body.resp.rate_hz = 0;
        err = call_fw_prop();

        if (msg->fw_clk.body.resp.rate_hz == 0) return Errno::ENOTSUP;

        return err;
    }
};
