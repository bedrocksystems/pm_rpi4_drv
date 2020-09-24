/*
 * Copyright (c) 2020 BedRock Systems, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 * This implementation is derived from linux kernel sources with the following copyright headers-
 * Copyright (C) 2010,2015 Broadcom
 * Copyright (C) 2012 Stephen Warren
 */

#pragma once
#include <bcm2835.h>
#include <pebble/io.hpp>
#include <pm.hpp>

/* macros to help integrate kernel code here */
#define BITS_PER_LONG 32

#define GENMASK(h, l) (((~0U) << (l)) & (~0U >> (BITS_PER_LONG - 1 - (h))))

#define BIT(nr) (1U << (nr))

#define CLOCK_DIV_UP(x, y) (((x) + (y)-1) / (y))

#define min_t(type, x, y)                                                                          \
    ({                                                                                             \
        type __min1 = (x);                                                                         \
        type __min2 = (y);                                                                         \
        __min1 < __min2 ? __min1 : __min2;                                                         \
    })

#define max_t(type, x, y)                                                                          \
    ({                                                                                             \
        type __max1 = (x);                                                                         \
        type __max2 = (y);                                                                         \
        __max1 > __max2 ? __max1 : __max2;                                                         \
    })

/* BCM clock definitions */
#define CM_GNRICCTL 0x000
#define CM_GNRICDIV 0x004
#define CM_DIV_FRAC_BITS 12
#define CM_DIV_FRAC_MASK GENMASK(CM_DIV_FRAC_BITS - 1, 0)

#define CM_VPUCTL 0x008
#define CM_VPUDIV 0x00c
#define CM_SYSCTL 0x010
#define CM_SYSDIV 0x014
#define CM_PERIACTL 0x018
#define CM_PERIADIV 0x01c
#define CM_PERIICTL 0x020
#define CM_PERIIDIV 0x024
#define CM_H264CTL 0x028
#define CM_H264DIV 0x02c
#define CM_ISPCTL 0x030
#define CM_ISPDIV 0x034
#define CM_V3DCTL 0x038
#define CM_V3DDIV 0x03c
#define CM_CAM0CTL 0x040
#define CM_CAM0DIV 0x044
#define CM_CAM1CTL 0x048
#define CM_CAM1DIV 0x04c
#define CM_CCP2CTL 0x050
#define CM_CCP2DIV 0x054
#define CM_DSI0ECTL 0x058
#define CM_DSI0EDIV 0x05c
#define CM_DSI0PCTL 0x060
#define CM_DSI0PDIV 0x064
#define CM_DPICTL 0x068
#define CM_DPIDIV 0x06c
#define CM_GP0CTL 0x070
#define CM_GP0DIV 0x074
#define CM_GP1CTL 0x078
#define CM_GP1DIV 0x07c
#define CM_GP2CTL 0x080
#define CM_GP2DIV 0x084
#define CM_HSMCTL 0x088
#define CM_HSMDIV 0x08c
#define CM_OTPCTL 0x090
#define CM_OTPDIV 0x094
#define CM_PCMCTL 0x098
#define CM_PCMDIV 0x09c
#define CM_PWMCTL 0x0a0
#define CM_PWMDIV 0x0a4
#define CM_SLIMCTL 0x0a8
#define CM_SLIMDIV 0x0ac
#define CM_SMICTL 0x0b0
#define CM_SMIDIV 0x0b4
/* no definition for 0x0b8  and 0x0bc */
#define CM_TCNTCTL 0x0c0
#define CM_TCNT_SRC1_SHIFT 12
#define CM_TCNTCNT 0x0c4
#define CM_TECCTL 0x0c8
#define CM_TECDIV 0x0cc
#define CM_TD0CTL 0x0d0
#define CM_TD0DIV 0x0d4
#define CM_TD1CTL 0x0d8
#define CM_TD1DIV 0x0dc
#define CM_TSENSCTL 0x0e0
#define CM_TSENSDIV 0x0e4
#define CM_TIMERCTL 0x0e8
#define CM_TIMERDIV 0x0ec
#define CM_UARTCTL 0x0f0
#define CM_UARTDIV 0x0f4
#define CM_VECCTL 0x0f8
#define CM_VECDIV 0x0fc
#define CM_PULSECTL 0x190
#define CM_PULSEDIV 0x194
#define CM_SDCCTL 0x1a8
#define CM_SDCDIV 0x1ac
#define CM_ARMCTL 0x1b0
#define CM_AVEOCTL 0x1b8
#define CM_AVEODIV 0x1bc
#define CM_EMMCCTL 0x1c0
#define CM_EMMCDIV 0x1c4
#define CM_EMMC2CTL 0x1d0
#define CM_EMMC2DIV 0x1d4
#define CM_AUX_GATE 0x4
/* General bits for the CM_*CTL regs */
#define CM_ENABLE BIT(4)
#define CM_KILL BIT(5)
#define CM_GATE_BIT 6
#define CM_GATE BIT(CM_GATE_BIT)
#define CM_BUSY BIT(7)
#define CM_BUSYD BIT(8)
#define CM_FRAC BIT(9)
#define CM_SRC_SHIFT 0
#define CM_SRC_BITS 4
#define CM_SRC_MASK 0xfu
#define CM_SRC_GND 0
#define CM_SRC_OSC 1
#define CM_SRC_TESTDEBUG0 2
#define CM_SRC_TESTDEBUG1 3
#define CM_SRC_PLLA_CORE 4
#define CM_SRC_PLLA_PER 4
#define CM_SRC_PLLC_CORE0 5
#define CM_SRC_PLLC_PER 5
#define CM_SRC_PLLC_CORE1 8
#define CM_SRC_PLLD_CORE 6
#define CM_SRC_PLLD_PER 6
#define CM_SRC_PLLH_AUX 7
#define CM_SRC_PLLC_CORE1 8
#define CM_SRC_PLLC_CORE2 9

#define CM_OSCCOUNT 0x100

#define CM_PLLA 0x104
#define CM_PLL_ANARST BIT(8)
#define CM_PLLA_HOLDPER BIT(7)
#define CM_PLLA_LOADPER BIT(6)
#define CM_PLLA_HOLDCORE BIT(5)
#define CM_PLLA_LOADCORE BIT(4)
#define CM_PLLA_HOLDCCP2 BIT(3)
#define CM_PLLA_LOADCCP2 BIT(2)
#define CM_PLLA_HOLDDSI0 BIT(1)
#define CM_PLLA_LOADDSI0 BIT(0)

#define CM_PLLC 0x108
#define CM_PLLC_HOLDPER BIT(7)
#define CM_PLLC_LOADPER BIT(6)
#define CM_PLLC_HOLDCORE2 BIT(5)
#define CM_PLLC_LOADCORE2 BIT(4)
#define CM_PLLC_HOLDCORE1 BIT(3)
#define CM_PLLC_LOADCORE1 BIT(2)
#define CM_PLLC_HOLDCORE0 BIT(1)
#define CM_PLLC_LOADCORE0 BIT(0)

#define CM_PLLD 0x10c
#define CM_PLLD_HOLDPER BIT(7)
#define CM_PLLD_LOADPER BIT(6)
#define CM_PLLD_HOLDCORE BIT(5)
#define CM_PLLD_LOADCORE BIT(4)
#define CM_PLLD_HOLDDSI1 BIT(3)
#define CM_PLLD_LOADDSI1 BIT(2)
#define CM_PLLD_HOLDDSI0 BIT(1)
#define CM_PLLD_LOADDSI0 BIT(0)

#define CM_PLLH 0x110
#define CM_PLLH_LOADRCAL BIT(2)
#define CM_PLLH_LOADAUX BIT(1)
#define CM_PLLH_LOADPIX BIT(0)

#define CM_LOCK 0x114
#define CM_LOCK_FLOCKH BIT(12)
#define CM_LOCK_FLOCKD BIT(11)
#define CM_LOCK_FLOCKC BIT(10)
#define CM_LOCK_FLOCKB BIT(9)
#define CM_LOCK_FLOCKA BIT(8)

#define CM_EVENT 0x118
#define CM_DSI1ECTL 0x158
#define CM_DSI1EDIV 0x15c
#define CM_DSI1PCTL 0x160
#define CM_DSI1PDIV 0x164
#define CM_DFTCTL 0x168
#define CM_DFTDIV 0x16c

#define CM_PLLB 0x170
#define CM_PLLB_HOLDARM BIT(1)
#define CM_PLLB_LOADARM BIT(0)

#define A2W_PLLA_CTRL 0x1100
#define A2W_PLLC_CTRL 0x1120
#define A2W_PLLD_CTRL 0x1140
#define A2W_PLLH_CTRL 0x1160
#define A2W_PLLB_CTRL 0x11e0
#define A2W_PLL_CTRL_PRST_DISABLE BIT(17)
#define A2W_PLL_CTRL_PWRDN BIT(16)
#define A2W_PLL_CTRL_PDIV_MASK 0x000007000u
#define A2W_PLL_CTRL_PDIV_SHIFT 12
#define A2W_PLL_CTRL_NDIV_MASK 0x0000003ffu
#define A2W_PLL_CTRL_NDIV_SHIFT 0

#define A2W_PLLA_ANA0 0x1010
#define A2W_PLLC_ANA0 0x1030
#define A2W_PLLD_ANA0 0x1050
#define A2W_PLLH_ANA0 0x1070
#define A2W_PLLB_ANA0 0x10f0

#define A2W_PLL_KA_SHIFT 7
#define A2W_PLL_KA_MASK GENMASK(9, 7)
#define A2W_PLL_KI_SHIFT 19
#define A2W_PLL_KI_MASK GENMASK(21, 19)
#define A2W_PLL_KP_SHIFT 15
#define A2W_PLL_KP_MASK GENMASK(18, 15)

#define A2W_PLLH_KA_SHIFT 19
#define A2W_PLLH_KA_MASK GENMASK(21, 19)
#define A2W_PLLH_KI_LOW_SHIFT 22
#define A2W_PLLH_KI_LOW_MASK GENMASK(23, 22)
#define A2W_PLLH_KI_HIGH_SHIFT 0
#define A2W_PLLH_KI_HIGH_MASK GENMASK(0, 0)
#define A2W_PLLH_KP_SHIFT 1
#define A2W_PLLH_KP_MASK GENMASK(4, 1)

#define A2W_XOSC_CTRL 0x1190
#define A2W_XOSC_CTRL_PLLB_ENABLE BIT(7)
#define A2W_XOSC_CTRL_PLLA_ENABLE BIT(6)
#define A2W_XOSC_CTRL_PLLD_ENABLE BIT(5)
#define A2W_XOSC_CTRL_DDR_ENABLE BIT(4)
#define A2W_XOSC_CTRL_CPR1_ENABLE BIT(3)
#define A2W_XOSC_CTRL_USB_ENABLE BIT(2)
#define A2W_XOSC_CTRL_HDMI_ENABLE BIT(1)
#define A2W_XOSC_CTRL_PLLC_ENABLE BIT(0)

#define A2W_PLLA_FRAC 0x1200
#define A2W_PLLC_FRAC 0x1220
#define A2W_PLLD_FRAC 0x1240
#define A2W_PLLH_FRAC 0x1260
#define A2W_PLLB_FRAC 0x12e0
#define A2W_PLL_FRAC_MASK ((1 << A2W_PLL_FRAC_BITS) - 1)
#define A2W_PLL_FRAC_BITS 20

#define A2W_PLL_CHANNEL_DISABLE BIT(8)
#define A2W_PLL_DIV_BITS 8
#define A2W_PLL_DIV_SHIFT 0

#define A2W_PLLA_DSI0 0x1300
#define A2W_PLLA_CORE 0x1400
#define A2W_PLLA_PER 0x1500
#define A2W_PLLA_CCP2 0x1600

#define A2W_PLLC_CORE2 0x1320
#define A2W_PLLC_CORE1 0x1420
#define A2W_PLLC_PER 0x1520
#define A2W_PLLC_CORE0 0x1620

#define A2W_PLLD_DSI0 0x1340
#define A2W_PLLD_CORE 0x1440
#define A2W_PLLD_PER 0x1540
#define A2W_PLLD_DSI1 0x1640

#define A2W_PLLH_AUX 0x1360
#define A2W_PLLH_RCAL 0x1460
#define A2W_PLLH_PIX 0x1560
#define A2W_PLLH_STS 0x1660

#define A2W_PLLH_CTRLR 0x1960
#define A2W_PLLH_FRACR 0x1a60
#define A2W_PLLH_AUXR 0x1b60
#define A2W_PLLH_RCALR 0x1c60
#define A2W_PLLH_PIXR 0x1d60
#define A2W_PLLH_STSR 0x1e60

#define A2W_PLLB_ARM 0x13e0
#define A2W_PLLB_SP0 0x14e0
#define A2W_PLLB_SP1 0x15e0
#define A2W_PLLB_SP2 0x16e0

#define CM_AUX_UART_SHIFT 0
#define CM_AUX_SPI1_SHIFT 1
#define CM_AUX_SPI2_SHIFT 2

#define LOCK_TIMEOUT_NS 100000000
#define BCM2835_MAX_FB_RATE 1750000000u

#define SOC_BCM2835 BIT(0)
#define SOC_BCM2711 BIT(1)
#define SOC_ALL (SOC_BCM2835 | SOC_BCM2711)

class rpi_clock;

class bcm2835_pll_ana_bits {
public:
    uint32 _mask0;
    uint32 _set0;
    uint32 _mask1;
    uint32 _set1;
    uint32 _mask3;
    uint32 _set3;
    uint32 _fb_prediv_mask;

    bcm2835_pll_ana_bits(bool pllh = false) {
        if (!pllh) {
            _mask0 = 0;
            _set0 = 0;
            _mask1 = A2W_PLL_KI_MASK | A2W_PLL_KP_MASK;
            _set1 = (2 << A2W_PLL_KI_SHIFT) | (8 << A2W_PLL_KP_SHIFT);
            _mask3 = A2W_PLL_KA_MASK;
            _set3 = (2 << A2W_PLL_KA_SHIFT);
            _fb_prediv_mask = BIT(14);
        } else {
            _mask0 = A2W_PLLH_KA_MASK | A2W_PLLH_KI_LOW_MASK;
            _set0 = (2 << A2W_PLLH_KA_SHIFT) | (2 << A2W_PLLH_KI_LOW_SHIFT);
            _mask1 = A2W_PLLH_KI_HIGH_MASK | A2W_PLLH_KP_MASK;
            _set1 = (6 << A2W_PLLH_KP_SHIFT);
            _mask3 = 0;
            _set3 = 0;
            _fb_prediv_mask = BIT(11);
        }
    };
};

struct bcm2835_pll_data {
    uint32 cm_ctrl_reg;
    uint32 a2w_ctrl_reg;
    uint32 frac_reg;
    uint32 ana_reg_base;
    uint32 reference_enable_mask;
    /* Bit in CM_LOCK to indicate when the PLL has locked. */
    uint32 lock_mask;
    const bcm2835_pll_ana_bits *ana;
    uint64 min_rate;
    uint64 max_rate;
    uint64 max_fb_rate;
    uint8 parent;
};

struct bcm2835_pll_divider_data {
    uint32 cm_reg;
    uint32 a2w_reg;
    uint32 load_mask;
    uint32 hold_mask;
    uint32 fixed_divider;
    uint8 parent;
};

struct bcm2835_clock_data {
    uint8 parents[10];
    uint8 num_mux_parents;

    /* Bitmap encoding which parents accept rate change propagation. */
    unsigned int set_rate_parent;

    uint32 ctl_reg;
    uint32 div_reg;

    /* Number of integer bits in the divider */
    uint32 int_bits;
    /* Number of fractional bits in the divider */
    uint32 frac_bits;

    bool is_mash_clock;
    bool low_jitter;
};

struct bcm2835_gate_data {
    uint8 parent;
    uint32 ctl_reg;
};

struct clk_rate_request {
    uint64 rate;
    uint64 min_rate;
    uint64 max_rate;
    uint64 best_parent_rate;
    uint8 best_parent;
};

class cprman {
public:
    Errno probe(mword base, mword aux_base);

    void write(uint32 reg, uint32 val) { outd((_base + reg), (CM_PASSWORD | val)); }

    uint32 read(uint32 reg) {
        uint32 ret = ind(_base + reg);
        return ret;
    }

    void write_aux(uint32 reg, uint32 val) { outd((_aux_base + reg), val); }

    uint32 read_aux(uint32 reg) {
        uint32 ret = ind(_aux_base + reg);
        return ret;
    }

    rpi_clock *get_clock(uint8 id) { return (id < BCM2711_CLOCK_TOTAL) ? _clks[id] : nullptr; }

    uint8 get_max_clock(void) { return (BCM2711_CLOCK_TOTAL - 1); }

    cprman(void);

    ~cprman(void);

private:
    mword _base;
    mword _aux_base;
    static constexpr uint32 CM_PASSWORD = 0x5a000000;
    rpi_clock *_clks[BCM2711_CLOCK_TOTAL]; /*add fixed osc clock*/
};

/**
 * FIXME: Clock interface should derive from the generic Clock in pm_plat.
 */
class rpi_clock {
public:
    virtual bool is_pll(void) = 0;

    virtual bool is_prepared(void) = 0;

    virtual Errno prepare(void) = 0;

    virtual Errno unprepare(void) = 0;

    virtual uint64 get_rate(void) = 0;

    virtual Errno set_rate(uint64 rate) = 0;

    virtual long round_rate(uint64 rate) = 0;

    virtual Errno determine_rate(struct clk_rate_request *req) = 0;

    virtual Errno set_parent(uint8 idx) = 0;

    virtual uint8 get_parent(void) = 0;

    virtual Errno describe_rate(Pm::clk_desc &desc) = 0;

    virtual void init(cprman *cm) = 0;

    virtual ~rpi_clock() {}

protected:
    uint8 _parent;
    uint64 _rate;
};

/**
 * Fixed clock definition for the oscillator.
 * The device DTS also has other fixed clocks, but not directly relevant to the clock manager.
 */
class bcm2835_fixed_clk : public rpi_clock {
public:
    bool is_pll(void) override { return false; }

    bool is_prepared(void) override { return true; }

    Errno prepare(void) override { return Errno::ENONE; }

    Errno unprepare(void) override { return Errno::ENOTSUP; }

    uint64 get_rate(void) override { return _rate; }

    Errno set_rate(uint64) override { return Errno::ENOTSUP; }

    long round_rate(uint64) override { return -1; }

    Errno determine_rate(struct clk_rate_request *) override { return Errno::ENOTSUP; }

    Errno set_parent(uint8) override { return Errno::ENOTSUP; }

    uint8 get_parent(void) override { return _parent; }

    void init(cprman *) override {}

    virtual Errno describe_rate(Pm::clk_desc &desc) override {
        desc.triplet = false;
        desc.min = static_cast<uint32>(_rate);
        return Errno::ENONE;
    }

    bcm2835_fixed_clk(uint64 rate) { _rate = rate; }
};

/**
 * PLLs A, C and D controlled by us, PLL B controlled by firmware.
 * The mailbox interface can be used to query PLL B data.
 */
class bcm2835_pll : public rpi_clock {
public:
    void choose_ndiv_and_fdiv(uint64 rate, uint64 parent_rate, uint32 *ndiv, uint32 *fdiv);

    void write_ana(uint32 ana_reg_base, uint32 *ana);

    bcm2835_pll(struct bcm2835_pll_data const *data);

    bool is_pll(void) override;

    bool is_prepared(void) override;

    Errno prepare(void) override;

    Errno unprepare(void) override;

    uint64 get_rate(void) override;

    Errno set_rate(uint64 rate) override;

    long round_rate(uint64 rate) override;

    Errno determine_rate(struct clk_rate_request *req) override;

    Errno set_parent(uint8 idx) override;

    uint8 get_parent(void) override;

    Errno describe_rate(Pm::clk_desc &desc) override {
        desc.triplet = false;
        desc.min = static_cast<uint32>(get_rate());
        return Errno::ENONE;
    }

    void init(cprman *cm) override { _cprman = cm; }

private:
    cprman *_cprman;
    const struct bcm2835_pll_data *_data;
    long rate_from_divisors(uint64 parent_rate, uint32 ndiv, uint32 fdiv, uint32 pdiv);
};

class bcm2835_pll_divider : public rpi_clock {
public:
    bool is_pll(void) override;

    bool is_prepared(void) override;

    Errno prepare(void) override;

    Errno unprepare(void) override;

    uint64 get_rate(void) override;

    Errno set_rate(uint64 rate) override;

    long round_rate(uint64 rate) override;

    Errno determine_rate(struct clk_rate_request *req) override;

    Errno set_parent(uint8 idx) override;

    uint8 get_parent(void) override;

    void init(cprman *cm) override { _cprman = cm; }

    Errno describe_rate(Pm::clk_desc &desc) override {
        desc.triplet = false;
        desc.min = static_cast<uint32>(get_rate());
        return Errno::ENONE;
    }

    bcm2835_pll_divider(struct bcm2835_pll_divider_data const *data);

private:
    cprman *_cprman;
    const struct bcm2835_pll_divider_data *_data;
};

class bcm2835_clock : public rpi_clock {
public:
    uint32 choose_div(uint64 rate, uint64 parent_rate, bool round_up);

    long rate_from_divisor(uint64 parent_rate, uint32 div);

    uint64 choose_div_and_prate(uint64 rate, uint32 *div, uint64 *prate, uint64 *avgrate);

    bool is_pll(void) override;

    bool is_prepared(void) override;

    Errno prepare(void) override;

    Errno unprepare(void) override;

    uint64 get_rate(void) override;

    Errno set_rate(uint64 rate) override;

    long round_rate(uint64 rate) override;

    Errno determine_rate(struct clk_rate_request *req) override;

    Errno set_parent(uint8 idx) override;

    uint8 get_parent(void) override;

    void init(cprman *cm) override;

    Errno describe_rate(Pm::clk_desc &desc) override {
        desc.triplet = false;
        desc.min = static_cast<uint32>(get_rate());
        return Errno::ENONE;
    }

    bcm2835_clock(struct bcm2835_clock_data const *data);

protected:
    cprman *_cprman;
    const struct bcm2835_clock_data *_data;
};

class bcm2835_vcpu_clock : public bcm2835_clock {
public:
    bool is_prepared(void) override;

    Errno prepare(void) override;

    Errno unprepare(void) override;

    bcm2835_vcpu_clock(struct bcm2835_clock_data const *data);
};

class bcm2835_gate : public rpi_clock {
public:
    bool is_pll(void) override;

    bool is_prepared(void) override;

    Errno prepare(void) override;

    Errno unprepare(void) override;

    uint64 get_rate(void) override;

    Errno set_rate(uint64 rate) override;

    long round_rate(uint64 rate) override;

    Errno determine_rate(struct clk_rate_request *req) override;

    Errno set_parent(uint8 idx) override;

    uint8 get_parent(void) override;

    void init(cprman *cm) override;

    Errno describe_rate(Pm::clk_desc &desc) override {
        desc.triplet = false;
        desc.min = static_cast<uint32>(get_rate());
        return Errno::ENONE;
    }

    bcm2835_gate(struct bcm2835_gate_data const *data);

protected:
    cprman *_cprman;
    const struct bcm2835_gate_data *_data;
};

class bcm2835_aux_clk : public bcm2835_gate {
public:
    bool is_prepared(void) override;

    Errno prepare(void) override;

    Errno unprepare(void) override;

    bcm2835_aux_clk(struct bcm2835_gate_data const *data, uint8 shift) : bcm2835_gate(data) {
        _shift = shift;
    }

    ~bcm2835_aux_clk(void) {}

private:
    uint8 _shift;
};

/* Clock Configurations, DSI clocks ignored*/

/*BCM2835_PLLA*/
const bcm2835_pll_ana_bits plla_ana;
const struct bcm2835_pll_data plla = {.cm_ctrl_reg = CM_PLLA,
                                      .a2w_ctrl_reg = A2W_PLLA_CTRL,
                                      .frac_reg = A2W_PLLA_FRAC,
                                      .ana_reg_base = A2W_PLLA_ANA0,
                                      .reference_enable_mask = A2W_XOSC_CTRL_PLLA_ENABLE,
                                      .lock_mask = CM_LOCK_FLOCKA,
                                      .ana = &plla_ana,
                                      .min_rate = 600000000u,
                                      .max_rate = 2400000000u,
                                      .max_fb_rate = BCM2835_MAX_FB_RATE,
                                      .parent = BCM2711_FIXED_OSC};

/*BCM2835_PLLA_CORE*/
const struct bcm2835_pll_divider_data plla_core = {
    .cm_reg = CM_PLLA,
    .a2w_reg = A2W_PLLA_CORE,
    .load_mask = CM_PLLA_LOADCORE,
    .hold_mask = CM_PLLA_HOLDCORE,
    .fixed_divider = 1,
    .parent = BCM2835_PLLA,
};

/*BCM2835_PLLA_PER*/
const struct bcm2835_pll_divider_data plla_per = {
    .cm_reg = CM_PLLA,
    .a2w_reg = A2W_PLLA_PER,
    .load_mask = CM_PLLA_LOADPER,
    .hold_mask = CM_PLLA_HOLDPER,
    .fixed_divider = 1,
    .parent = BCM2835_PLLA,
};

/*BCM2835_PLLA_DSI0*/
const struct bcm2835_pll_divider_data plla_dsi0 = {
    .cm_reg = CM_PLLA,
    .a2w_reg = A2W_PLLA_DSI0,
    .load_mask = CM_PLLA_LOADDSI0,
    .hold_mask = CM_PLLA_HOLDDSI0,
    .fixed_divider = 1,
    .parent = BCM2835_PLLA,
};

/*BCM2835_PLLA_CCP2*/
const struct bcm2835_pll_divider_data plla_ccp2 = {
    .cm_reg = CM_PLLA,
    .a2w_reg = A2W_PLLA_CCP2,
    .load_mask = CM_PLLA_LOADCCP2,
    .hold_mask = CM_PLLA_HOLDCCP2,
    .fixed_divider = 1,
    .parent = BCM2835_PLLA,
};

/*BCM2835_PLLC*/
const bcm2835_pll_ana_bits pllc_ana;
const struct bcm2835_pll_data pllc = {
    .cm_ctrl_reg = CM_PLLC,
    .a2w_ctrl_reg = A2W_PLLC_CTRL,
    .frac_reg = A2W_PLLC_FRAC,
    .ana_reg_base = A2W_PLLC_ANA0,
    .reference_enable_mask = A2W_XOSC_CTRL_PLLC_ENABLE,
    .lock_mask = CM_LOCK_FLOCKC,
    .ana = &pllc_ana,
    .min_rate = 600000000u,
    .max_rate = 3000000000u,
    .max_fb_rate = BCM2835_MAX_FB_RATE,
    .parent = BCM2711_FIXED_OSC,
};

/*BCM2835_PLLC_CORE1*/
const struct bcm2835_pll_divider_data pllc_core0 = {
    .cm_reg = CM_PLLC,
    .a2w_reg = A2W_PLLC_CORE0,
    .load_mask = CM_PLLC_LOADCORE0,
    .hold_mask = CM_PLLC_HOLDCORE0,
    .fixed_divider = 1,
    .parent = BCM2835_PLLC,
};

/*BCM2835_PLLC_CORE1*/
const struct bcm2835_pll_divider_data pllc_core1 = {
    .cm_reg = CM_PLLC,
    .a2w_reg = A2W_PLLC_CORE1,
    .load_mask = CM_PLLC_LOADCORE1,
    .hold_mask = CM_PLLC_HOLDCORE1,
    .fixed_divider = 1,
    .parent = BCM2835_PLLC,
};

/*BCM2835_PLLC_CORE2*/
const struct bcm2835_pll_divider_data pllc_core2 = {
    .cm_reg = CM_PLLC,
    .a2w_reg = A2W_PLLC_CORE2,
    .load_mask = CM_PLLC_LOADCORE2,
    .hold_mask = CM_PLLC_HOLDCORE2,
    .fixed_divider = 1,
    .parent = BCM2835_PLLC,
};

/*BCM2835_PLLC_PER*/
const struct bcm2835_pll_divider_data pllc_per = {
    .cm_reg = CM_PLLC,
    .a2w_reg = A2W_PLLC_PER,
    .load_mask = CM_PLLC_LOADPER,
    .hold_mask = CM_PLLC_HOLDPER,
    .fixed_divider = 1,
    .parent = BCM2835_PLLC,
};

/*BCM2835_PLLD*/
const bcm2835_pll_ana_bits plld_ana;
const struct bcm2835_pll_data plld = {
    .cm_ctrl_reg = CM_PLLD,
    .a2w_ctrl_reg = A2W_PLLD_CTRL,
    .frac_reg = A2W_PLLD_FRAC,
    .ana_reg_base = A2W_PLLD_ANA0,
    .reference_enable_mask = A2W_XOSC_CTRL_PLLD_ENABLE,
    .lock_mask = CM_LOCK_FLOCKD,
    .ana = &plld_ana,
    .min_rate = 600000000u,
    .max_rate = 2400000000u,
    .max_fb_rate = BCM2835_MAX_FB_RATE,
    .parent = BCM2711_FIXED_OSC,
};

/*BCM2835_PLLD_CORE*/
const struct bcm2835_pll_divider_data plld_core = {
    .cm_reg = CM_PLLD,
    .a2w_reg = A2W_PLLD_CORE,
    .load_mask = CM_PLLD_LOADCORE,
    .hold_mask = CM_PLLD_HOLDCORE,
    .fixed_divider = 1,
    .parent = BCM2835_PLLD,
};

/*BCM2835_PLLD_PER*/
const struct bcm2835_pll_divider_data plld_per = {
    .cm_reg = CM_PLLD,
    .a2w_reg = A2W_PLLD_PER,
    .load_mask = CM_PLLD_LOADPER,
    .hold_mask = CM_PLLD_HOLDPER,
    .fixed_divider = 1,
    .parent = BCM2835_PLLD,
};

/*BCM2835_PLLD_DSI0*/
const struct bcm2835_pll_divider_data plld_dsi0 = {
    .cm_reg = CM_PLLD,
    .a2w_reg = A2W_PLLD_DSI0,
    .load_mask = CM_PLLD_LOADDSI0,
    .hold_mask = CM_PLLD_HOLDDSI0,
    .fixed_divider = 1,
    .parent = BCM2835_PLLD,
};

/*BCM2835_PLLD_DSI1*/
const struct bcm2835_pll_divider_data plld_dsi1 = {
    .cm_reg = CM_PLLD,
    .a2w_reg = A2W_PLLD_DSI1,
    .load_mask = CM_PLLD_LOADDSI1,
    .hold_mask = CM_PLLD_HOLDDSI1,
    .fixed_divider = 1,
    .parent = BCM2835_PLLD,
};

/*BCM2835_PLLH*/
const bcm2835_pll_ana_bits pllh_ana(true);
const struct bcm2835_pll_data pllh = {
    .cm_ctrl_reg = CM_PLLH,
    .a2w_ctrl_reg = A2W_PLLH_CTRL,
    .frac_reg = A2W_PLLH_FRAC,
    .ana_reg_base = A2W_PLLH_ANA0,
    .reference_enable_mask = A2W_XOSC_CTRL_PLLC_ENABLE,
    .lock_mask = CM_LOCK_FLOCKH,
    .ana = &pllh_ana,
    .min_rate = 600000000u,
    .max_rate = 3000000000u,
    .max_fb_rate = BCM2835_MAX_FB_RATE,
    .parent = BCM2711_FIXED_OSC,
};

/* One Time Programmable Memory clock.  Maximum 10Mhz. */
/*BCM2835_CLOCK_OTP*/
const struct bcm2835_clock_data otp_data {
    .parents = {BCM2711_INVALID, BCM2711_FIXED_OSC, BCM2711_INVALID, BCM2711_INVALID},
    .num_mux_parents = 4, .set_rate_parent = 0, .ctl_reg = CM_OTPCTL, .div_reg = CM_OTPDIV,
    .int_bits = 4, .frac_bits = 0, .is_mash_clock = false, .low_jitter = false,
};

/*
 * Used for a 1Mhz clock for the system clocksource, and also used
 * bythe watchdog timer and the camera pulse generator.
 */
/*BCM2835_CLOCK_TIMER*/
const struct bcm2835_clock_data timer_data {
    .parents = {BCM2711_INVALID, BCM2711_FIXED_OSC, BCM2711_INVALID, BCM2711_INVALID},
    .num_mux_parents = 4, .set_rate_parent = 0, .ctl_reg = CM_TIMERCTL, .div_reg = CM_TIMERDIV,
    .int_bits = 6, .frac_bits = 12, .is_mash_clock = false, .low_jitter = false,
};

/*
 * Clock for the temperature sensor.
 * Generally run at 2Mhz, max 5Mhz.
 */
/*BCM2835_CLOCK_TSENS*/
const struct bcm2835_clock_data tsense_data {
    .parents = {BCM2711_INVALID, BCM2711_FIXED_OSC, BCM2711_INVALID, BCM2711_INVALID},
    .num_mux_parents = 4, .set_rate_parent = 0, .ctl_reg = CM_TSENSCTL, .div_reg = CM_TSENSDIV,
    .int_bits = 5, .frac_bits = 0, .is_mash_clock = false, .low_jitter = false,
};

/*BCM2835_CLOCK_TEC*/
const struct bcm2835_clock_data tec_data {
    .parents = {BCM2711_INVALID, BCM2711_FIXED_OSC, BCM2711_INVALID, BCM2711_INVALID},
    .num_mux_parents = 4, .set_rate_parent = 0, .ctl_reg = CM_TECCTL, .div_reg = CM_TECDIV,
    .int_bits = 6, .frac_bits = 0, .is_mash_clock = false, .low_jitter = false,
};

/* clocks with vpu parent mux */

/*BCM2835_CLOCK_H264*/
const struct bcm2835_clock_data h264_data {
    .parents = {BCM2711_INVALID,    BCM2711_FIXED_OSC,  BCM2711_INVALID,   BCM2711_INVALID,
                BCM2835_PLLA_CORE,  BCM2835_PLLC_CORE0, BCM2835_PLLD_CORE, BCM2711_INVALID,
                BCM2835_PLLC_CORE1, BCM2835_PLLC_CORE2},
    .num_mux_parents = 10, .set_rate_parent = 0, .ctl_reg = CM_H264CTL, .div_reg = CM_H264DIV,
    .int_bits = 4, .frac_bits = 8, .is_mash_clock = false, .low_jitter = false,
};

/*BCM2835_CLOCK_ISP*/
const struct bcm2835_clock_data isp_data {
    .parents = {BCM2711_INVALID,    BCM2711_FIXED_OSC,  BCM2711_INVALID,   BCM2711_INVALID,
                BCM2835_PLLA_CORE,  BCM2835_PLLC_CORE0, BCM2835_PLLD_CORE, BCM2711_INVALID,
                BCM2835_PLLC_CORE1, BCM2835_PLLC_CORE2},
    .num_mux_parents = 10, .set_rate_parent = 0, .ctl_reg = CM_ISPCTL, .div_reg = CM_ISPDIV,
    .int_bits = 4, .frac_bits = 8, .is_mash_clock = false, .low_jitter = false,
};

/*
 * Secondary SDRAM clock.  Used for low-voltage modes when the PLL
 * in the SDRAM controller can't be used.
 */
/*BCM2835_CLOCK_SDRAM*/
const struct bcm2835_clock_data sdram_data {
    .parents = {BCM2711_INVALID,    BCM2711_FIXED_OSC,  BCM2711_INVALID,   BCM2711_INVALID,
                BCM2835_PLLA_CORE,  BCM2835_PLLC_CORE0, BCM2835_PLLD_CORE, BCM2711_INVALID,
                BCM2835_PLLC_CORE1, BCM2835_PLLC_CORE2},
    .num_mux_parents = 10, .set_rate_parent = 0, .ctl_reg = CM_SDCCTL, .div_reg = CM_SDCDIV,
    .int_bits = 6, .frac_bits = 0, .is_mash_clock = false, .low_jitter = false,
};

/*BCM2835_CLOCK_V3D*/
const struct bcm2835_clock_data v3d_data {
    .parents = {BCM2711_INVALID,    BCM2711_FIXED_OSC,  BCM2711_INVALID,   BCM2711_INVALID,
                BCM2835_PLLA_CORE,  BCM2835_PLLC_CORE0, BCM2835_PLLD_CORE, BCM2711_INVALID,
                BCM2835_PLLC_CORE1, BCM2835_PLLC_CORE2},
    .num_mux_parents = 10, .set_rate_parent = 0, .ctl_reg = CM_V3DCTL, .div_reg = CM_V3DDIV,
    .int_bits = 4, .frac_bits = 8, .is_mash_clock = false, .low_jitter = false,
};

/*
 * VPU clock.  This doesn't have an enable bit, since it drives
 * the bus for everything else, and is special so it doesn't need
 * to be gated for rate changes.  It is also known as "clk_audio"
 * in various hardware documentation.
 */
/*BCM2835_CLOCK_VPU*/
const struct bcm2835_clock_data vpu_data {
    .parents = {BCM2711_INVALID,    BCM2711_FIXED_OSC,  BCM2711_INVALID,   BCM2711_INVALID,
                BCM2835_PLLA_CORE,  BCM2835_PLLC_CORE0, BCM2835_PLLD_CORE, BCM2711_INVALID,
                BCM2835_PLLC_CORE1, BCM2835_PLLC_CORE2},
    .num_mux_parents = 10, .set_rate_parent = 0, .ctl_reg = CM_VPUCTL, .div_reg = CM_VPUDIV,
    .int_bits = 12, .frac_bits = 8, .is_mash_clock = false, .low_jitter = false,
};

/* clocks with per parent mux */
/*BCM2835_CLOCK_AVEO*/
const struct bcm2835_clock_data aveo_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_AVEOCTL, .div_reg = CM_AVEODIV,
    .int_bits = 4, .frac_bits = 0, .is_mash_clock = false, .low_jitter = false,
};

/*BCM2835_CLOCK_CAM0*/
const struct bcm2835_clock_data cam0_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_CAM0CTL, .div_reg = CM_CAM0DIV,
    .int_bits = 4, .frac_bits = 8, .is_mash_clock = false, .low_jitter = false,
};

/*BCM2835_CLOCK_CAM1*/
const struct bcm2835_clock_data cam1_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_CAM1CTL, .div_reg = CM_CAM1DIV,
    .int_bits = 4, .frac_bits = 8, .is_mash_clock = false, .low_jitter = false,
};

/*BCM2835_CLOCK_DFT*/
const struct bcm2835_clock_data dft_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_DFTCTL, .div_reg = CM_DFTDIV,
    .int_bits = 5, .frac_bits = 0, .is_mash_clock = false, .low_jitter = false,
};

/*BCM2835_CLOCK_DPI*/
const struct bcm2835_clock_data dpi_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_DPICTL, .div_reg = CM_DPIDIV,
    .int_bits = 4, .frac_bits = 8, .is_mash_clock = false, .low_jitter = false,
};

/* Arasan EMMC clock */
/*BCM2835_CLOCK_EMMC*/
const struct bcm2835_clock_data emmc_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_EMMCCTL, .div_reg = CM_EMMCDIV,
    .int_bits = 4, .frac_bits = 8, .is_mash_clock = false, .low_jitter = false,
};

/*BCM2711_CLOCK_EMMC2*/
const struct bcm2835_clock_data emmc2_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_EMMC2CTL, .div_reg = CM_EMMC2DIV,
    .int_bits = 4, .frac_bits = 8, .is_mash_clock = false, .low_jitter = false,
};

/* General purpose (GPIO) clocks */
/*BCM2835_CLOCK_GP0*/
const struct bcm2835_clock_data gp0_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_GP0CTL, .div_reg = CM_GP0DIV,
    .int_bits = 12, .frac_bits = 12, .is_mash_clock = true, .low_jitter = false,
};

/*BCM2835_CLOCK_GP1*/
const struct bcm2835_clock_data gp1_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_GP1CTL, .div_reg = CM_GP1DIV,
    .int_bits = 12, .frac_bits = 12, .is_mash_clock = true, .low_jitter = false,
};

/*BCM2835_CLOCK_GP2*/
const struct bcm2835_clock_data gp2_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_GP2CTL, .div_reg = CM_GP2DIV,
    .int_bits = 12, .frac_bits = 12, .is_mash_clock = false, .low_jitter = false,
};

/* HDMI state machine */
/*BCM2835_CLOCK_HSM*/
const struct bcm2835_clock_data hsm_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_HSMCTL, .div_reg = CM_HSMDIV,
    .int_bits = 4, .frac_bits = 8, .is_mash_clock = false, .low_jitter = false,
};

/*BCM2835_CLOCK_PCM*/
const struct bcm2835_clock_data pcm_data {
    .parents = {BCM2711_INVALID, BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2711_INVALID, BCM2711_INVALID,   BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_PCMCTL, .div_reg = CM_PCMDIV,
    .int_bits = 12, .frac_bits = 12, .is_mash_clock = true, .low_jitter = true,
};

/*BCM2835_CLOCK_PWM*/
const struct bcm2835_clock_data pwm_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_PWMCTL, .div_reg = CM_PWMDIV,
    .int_bits = 12, .frac_bits = 12, .is_mash_clock = true, .low_jitter = false,
};

/*BCM2835_CLOCK_SLIM*/
const struct bcm2835_clock_data slim_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_SLIMCTL, .div_reg = CM_SLIMDIV,
    .int_bits = 12, .frac_bits = 12, .is_mash_clock = true, .low_jitter = false,
};

/*BCM2835_CLOCK_SMI*/
const struct bcm2835_clock_data smi_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_SMICTL, .div_reg = CM_SMIDIV,
    .int_bits = 4, .frac_bits = 8, .is_mash_clock = false, .low_jitter = false,
};

/*BCM2835_CLOCK_UART*/
const struct bcm2835_clock_data uart_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_UARTCTL, .div_reg = CM_UARTDIV,
    .int_bits = 10, .frac_bits = 12, .is_mash_clock = false, .low_jitter = false,
};

/* TV encoder clock.  Only operating frequency is 108Mhz.  */
/*BCM2835_CLOCK_VEC*/
const struct bcm2835_clock_data vec_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_VECCTL, .div_reg = CM_VECDIV,
    .int_bits = 4, .frac_bits = 0, .is_mash_clock = false, .low_jitter = false,
};

/*BCM2835_CLOCK_DSI0E*/
const struct bcm2835_clock_data dsi0e_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_DSI0ECTL, .div_reg = CM_DSI0EDIV,
    .int_bits = 4, .frac_bits = 8, .is_mash_clock = false, .low_jitter = false,
};

/*BCM2835_CLOCK_DSI1E*/
const struct bcm2835_clock_data dsi1e_data {
    .parents = {BCM2711_INVALID,  BCM2711_FIXED_OSC, BCM2711_INVALID,  BCM2711_INVALID,
                BCM2835_PLLA_PER, BCM2835_PLLC_PER,  BCM2835_PLLD_PER, BCM2711_INVALID},
    .num_mux_parents = 8, .set_rate_parent = 0, .ctl_reg = CM_DSI1ECTL, .div_reg = CM_DSI1EDIV,
    .int_bits = 4, .frac_bits = 8, .is_mash_clock = false, .low_jitter = false,
};

/*
 * CM_PERIICTL (and CM_PERIACTL, CM_SYSCTL and CM_VPUCTL if
 * you have the debug bit set in the power manager, which we
 * don't bother exposing) are individual gates off of the
 * non-stop vpu clock.
 */
/*BCM2835_CLOCK_PERI_IMAGE*/
const struct bcm2835_gate_data peri_image_data {
    .parent = BCM2835_CLOCK_VPU, .ctl_reg = CM_PERIICTL,
};

/*BCM2711_CLOCK_AUX_UART*/
const struct bcm2835_gate_data aux_uart_data {
    .parent = BCM2835_CLOCK_VPU, .ctl_reg = CM_AUX_GATE,
};

/*BCM2711_CLOCK_AUX_SPI1*/
const struct bcm2835_gate_data aux_spi1_data {
    .parent = BCM2835_CLOCK_VPU, .ctl_reg = CM_AUX_GATE,
};

/*BCM2711_CLOCK_AUX_SPI2*/
const struct bcm2835_gate_data aux_spi2_data {
    .parent = BCM2835_CLOCK_VPU, .ctl_reg = CM_AUX_GATE,
};
