/*
 * Copyright (c) 2020 BedRock Systems, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0
 *
 * This implementation is derived from linux kernel sources with the following copyright headers-
 * Copyright (C) 2010,2015 Broadcom
 * Copyright (C) 2012 Stephen Warren
 */
#include <rpi_clock.hpp>

uint32
min(uint32 a, uint32 b) {
    return a < b ? a : b;
}

uint64
max(uint64 a, uint64 b) {
    return a > b ? a : b;
}

bool
bcm2835_pll::is_pll(void) {
    return true;
}

bool
bcm2835_pll::is_prepared(void) {
    return _cprman->read(_data->a2w_ctrl_reg) & A2W_PLL_CTRL_PRST_DISABLE;
}

Errno
bcm2835_pll::prepare(void) {

    _cprman->write(_data->a2w_ctrl_reg, _cprman->read(_data->a2w_ctrl_reg) & ~A2W_PLL_CTRL_PWRDN);

    /* Take the PLL out of reset. */
    _cprman->write(_data->cm_ctrl_reg, _cprman->read(_data->cm_ctrl_reg) & ~CM_PLL_ANARST);

    /* Wait for the PLL to lock. */
    while (!(_cprman->read(CM_LOCK) & _data->lock_mask)) {
        /*FIXME:use delay*/
    }

    _cprman->write(_data->a2w_ctrl_reg,
                   _cprman->read(_data->a2w_ctrl_reg) | A2W_PLL_CTRL_PRST_DISABLE);

    return Errno::ENONE;
}

Errno
bcm2835_pll::unprepare(void) {
    _cprman->write(_data->cm_ctrl_reg, CM_PLL_ANARST);
    _cprman->write(_data->a2w_ctrl_reg, _cprman->read(_data->a2w_ctrl_reg) | A2W_PLL_CTRL_PWRDN);
    return Errno::ENONE;
}

long
bcm2835_pll::rate_from_divisors(uint64 parent_rate, uint32 ndiv, uint32 fdiv, uint32 pdiv) {
    uint64 rate;
    if (pdiv == 0) return 0;

    rate = static_cast<uint64>(parent_rate) * ((ndiv << A2W_PLL_FRAC_BITS) + fdiv);
    rate = rate / pdiv;
    return rate >> A2W_PLL_FRAC_BITS;
}

uint64
bcm2835_pll::get_rate(void) {
    uint32 a2wctrl = _cprman->read(_data->a2w_ctrl_reg);
    uint32 ndiv, pdiv, fdiv;
    bool using_prediv;
    uint64 parent_rate = (_cprman->get_clock(_parent))->get_rate();

    if (parent_rate == 0) return 0;

    fdiv = _cprman->read(_data->frac_reg) & A2W_PLL_FRAC_MASK;
    ndiv = (a2wctrl & A2W_PLL_CTRL_NDIV_MASK) >> A2W_PLL_CTRL_NDIV_SHIFT;
    pdiv = (a2wctrl & A2W_PLL_CTRL_PDIV_MASK) >> A2W_PLL_CTRL_PDIV_SHIFT;
    using_prediv = _cprman->read(_data->ana_reg_base + 4) & _data->ana->_fb_prediv_mask;

    if (using_prediv) {
        ndiv *= 2;
        fdiv *= 2;
    }

    return static_cast<uint64>(rate_from_divisors(parent_rate, ndiv, fdiv, pdiv));
}

void
bcm2835_pll::choose_ndiv_and_fdiv(uint64 rate, uint64 parent_rate, uint32 *ndiv, uint32 *fdiv) {
    uint64 div;

    div = rate << A2W_PLL_FRAC_BITS;
    div = div / parent_rate;

    *ndiv = static_cast<uint32>(div >> A2W_PLL_FRAC_BITS);
    *fdiv = div & ((1 << A2W_PLL_FRAC_BITS) - 1);
}

void
bcm2835_pll::write_ana(uint32 ana_reg_base, uint32 *ana) {
    int i;

    /*
     * ANA register setup is done as a series of writes to
     * ANA3-ANA0, in that order.  This lets us write all 4
     * registers as a single cycle of the serdes interface (taking
     * 100 xosc clocks), whereas if we were to update ana0, 1, and
     * 3 individually through their partial-write registers, each
     * would be their own serdes cycle.
     */
    for (i = 3; i >= 0; i--)
        _cprman->write(ana_reg_base + static_cast<uint32>(i) * 4u, ana[i]);
}

Errno
bcm2835_pll::set_rate(uint64 rate) {
    bool was_using_prediv, use_fb_prediv, do_ana_setup_first;
    uint32 ndiv, fdiv, a2w_ctl;
    uint32 ana[4];
    int i;
    uint64 parent_rate = (_cprman->get_clock(_parent))->get_rate();

    if (rate > _data->max_fb_rate) {
        use_fb_prediv = true;
        rate /= 2;
    } else {
        use_fb_prediv = false;
    }

    choose_ndiv_and_fdiv(rate, parent_rate, &ndiv, &fdiv);

    for (i = 3; i >= 0; i--)
        ana[i] = _cprman->read(_data->ana_reg_base + static_cast<uint32>(i) * 4u);

    was_using_prediv = ana[1] & _data->ana->_fb_prediv_mask;

    ana[0] &= ~_data->ana->_mask0;
    ana[0] |= _data->ana->_set0;
    ana[1] &= ~_data->ana->_mask1;
    ana[1] |= _data->ana->_set1;
    ana[3] &= ~_data->ana->_mask3;
    ana[3] |= _data->ana->_set3;

    if (was_using_prediv && !use_fb_prediv) {
        ana[1] &= ~_data->ana->_fb_prediv_mask;
        do_ana_setup_first = true;
    } else if (!was_using_prediv && use_fb_prediv) {
        ana[1] |= _data->ana->_fb_prediv_mask;
        do_ana_setup_first = false;
    } else {
        do_ana_setup_first = true;
    }

    /* Unmask the reference clock from the oscillator. */
    _cprman->write(A2W_XOSC_CTRL, _cprman->read(A2W_XOSC_CTRL) | _data->reference_enable_mask);

    if (do_ana_setup_first) write_ana(_data->ana_reg_base, ana);

    /* Set the PLL multiplier from the oscillator. */
    _cprman->write(_data->frac_reg, fdiv);

    a2w_ctl = _cprman->read(_data->a2w_ctrl_reg);
    a2w_ctl &= ~A2W_PLL_CTRL_NDIV_MASK;
    a2w_ctl |= ndiv << A2W_PLL_CTRL_NDIV_SHIFT;
    a2w_ctl &= ~A2W_PLL_CTRL_PDIV_MASK;
    a2w_ctl |= 1 << A2W_PLL_CTRL_PDIV_SHIFT;
    _cprman->write(_data->a2w_ctrl_reg, a2w_ctl);

    if (!do_ana_setup_first) write_ana(_data->ana_reg_base, ana);

    return Errno::ENONE;
}

long
bcm2835_pll::round_rate(uint64 rate) {
    uint32 ndiv, fdiv;
    uint64 parent_rate = (_cprman->get_clock(_parent))->get_rate();

    rate = (rate > _data->min_rate) ? ((rate < _data->max_rate) ? rate : _data->max_rate) :
                                      _data->min_rate;

    choose_ndiv_and_fdiv(rate, parent_rate, &ndiv, &fdiv);

    return rate_from_divisors(parent_rate, ndiv, fdiv, 1);
}

Errno
bcm2835_pll::determine_rate(struct clk_rate_request *) {
    return Errno::ENOTSUP;
}

Errno
bcm2835_pll::set_parent(uint8) {
    return Errno::ENOTSUP;
}

uint8
bcm2835_pll::get_parent(void) {
    return _parent;
}

bool
bcm2835_pll_divider::is_pll(void) {
    return true;
}

bool
bcm2835_pll_divider::is_prepared(void) {
    return !(_cprman->read(_data->a2w_reg) & A2W_PLL_CHANNEL_DISABLE);
}

Errno
bcm2835_pll_divider::prepare(void) {
    _cprman->write(_data->a2w_reg, _cprman->read(_data->a2w_reg) & ~A2W_PLL_CHANNEL_DISABLE);

    _cprman->write(_data->cm_reg, _cprman->read(_data->cm_reg) & ~_data->hold_mask);

    return Errno::ENONE;
}

Errno
bcm2835_pll_divider::unprepare(void) {
    _cprman->write(_data->cm_reg,
                   (_cprman->read(_data->cm_reg) & ~_data->load_mask) | _data->hold_mask);
    _cprman->write(_data->a2w_reg, _cprman->read(_data->a2w_reg) | A2W_PLL_CHANNEL_DISABLE);

    return Errno::ENONE;
}

uint64
bcm2835_pll_divider::get_rate(void) {
    unsigned int val;
    uint64 parent_rate = (_cprman->get_clock(_parent))->get_rate();

    val = _cprman->read(_data->a2w_reg) >> A2W_PLL_DIV_SHIFT;
    val &= (1u << (A2W_PLL_DIV_BITS + 1)) - 1;

    if (val == 0) val = 1;

    return CLOCK_DIV_UP(parent_rate, static_cast<uint64>(val));
}

Errno
bcm2835_pll_divider::set_rate(uint64 rate) {
    uint32 cm, div, max_div = 1 << A2W_PLL_DIV_BITS;
    uint64 parent_rate = (_cprman->get_clock(_parent))->get_rate();

    div = static_cast<uint32>(CLOCK_DIV_UP(parent_rate, rate));

    div = min(div, max_div);
    if (div == max_div) div = 0;

    _cprman->write(_data->a2w_reg, div);
    cm = _cprman->read(_data->cm_reg);
    _cprman->write(_data->cm_reg, cm | _data->load_mask);
    _cprman->write(_data->cm_reg, cm & ~_data->load_mask);

    return Errno::ENONE;
}

long
bcm2835_pll_divider::round_rate(uint64) {
    return -1;
}

Errno
bcm2835_pll_divider::determine_rate(struct clk_rate_request *) {
    return Errno::ENOTSUP;
}

Errno
bcm2835_pll_divider::set_parent(uint8) {
    return Errno::ENOTSUP;
}

uint8
bcm2835_pll_divider::get_parent(void) {
    return _parent;
}

bool
bcm2835_clock::is_pll(void) {
    return false;
}

bool
bcm2835_clock::is_prepared(void) {
    return (_cprman->read(_data->ctl_reg) & CM_ENABLE) != 0;
}

Errno
bcm2835_clock::prepare(void) {
    _cprman->write(_data->ctl_reg, _cprman->read(_data->ctl_reg) | CM_ENABLE);
    return Errno::ENONE;
}

Errno
bcm2835_clock::unprepare(void) {
    _cprman->write(_data->ctl_reg, _cprman->read(_data->ctl_reg) & ~CM_ENABLE);

    while (_cprman->read(_data->ctl_reg) & CM_BUSY)
        ;
    return Errno::ENONE;
}

long
bcm2835_clock::rate_from_divisor(uint64 parent_rate, uint32 div) {
    uint64 temp;

    if (_data->int_bits == 0 && _data->frac_bits == 0) return static_cast<long>(parent_rate);

    /*
     * The divisor is a 12.12 fixed point field, but only some of
     * the bits are populated in any given clock.
     */
    div >>= CM_DIV_FRAC_BITS - _data->frac_bits;
    div &= (1u << (_data->int_bits + _data->frac_bits)) - 1;

    if (div == 0) return 0;

    temp = parent_rate << _data->frac_bits;
    temp = temp / div;

    return static_cast<long>(temp);
}

uint64
bcm2835_clock::get_rate(void) {
    uint32 div;
    rpi_clock *parent = _cprman->get_clock(_data->parents[get_parent()]);

    if (!parent) return 0;

    uint64 parent_rate = parent->get_rate();

    if (_data->int_bits == 0 && _data->frac_bits == 0) return parent_rate;

    div = _cprman->read(_data->div_reg);

    return static_cast<uint64>(rate_from_divisor(parent_rate, div));
}

uint32
bcm2835_clock::choose_div(uint64 rate, uint64 parent_rate, bool round_up) {
    uint32 unused_frac_mask = GENMASK(CM_DIV_FRAC_BITS - _data->frac_bits, 0) >> 1;
    uint64 temp = parent_rate << CM_DIV_FRAC_BITS;
    uint64 rem;
    uint32 div, mindiv, maxdiv;

    rem = temp / rate;
    div = static_cast<uint32>(temp);

    /* Round up and mask off the unused bits */
    if (round_up && ((div & unused_frac_mask) != 0 || rem != 0)) div += unused_frac_mask + 1;
    div &= ~unused_frac_mask;

    /* different clamping limits apply for a mash clock */
    if (_data->is_mash_clock) {
        /* clamp to min divider of 2 */
        mindiv = 2 << CM_DIV_FRAC_BITS;
        /* clamp to the highest possible integer divider */
        maxdiv = (BIT(_data->int_bits) - 1) << CM_DIV_FRAC_BITS;
    } else {
        /* clamp to min divider of 1 */
        mindiv = 1 << CM_DIV_FRAC_BITS;
        /* clamp to the highest possible fractional divider */
        maxdiv
            = GENMASK(_data->int_bits + CM_DIV_FRAC_BITS - 1, CM_DIV_FRAC_BITS - _data->frac_bits);
    }

    /* apply the clamping  limits */
    div = max_t(uint32, div, mindiv);
    div = min_t(uint32, div, maxdiv);

    return div;
}

Errno
bcm2835_clock::set_rate(uint64 rate) {
    uint64 parent_rate = (_cprman->get_clock(_parent))->get_rate();
    uint32 div = choose_div(rate, parent_rate, false);
    uint32 ctl;

    /*
     * Setting up frac support
     *
     * In principle it is recommended to stop/start the clock first,
     * but as we set CLK_SET_RATE_GATE during registration of the
     * clock this requirement should be take care of by the
     * clk-framework.
     */
    ctl = _cprman->read(_data->ctl_reg) & ~CM_FRAC;
    ctl |= (div & CM_DIV_FRAC_MASK) ? CM_FRAC : 0;
    _cprman->write(_data->ctl_reg, ctl);

    _cprman->write(_data->div_reg, div);

    return Errno::ENONE;
}

long
bcm2835_clock::round_rate(uint64) {
    return -1;
}

uint64
bcm2835_clock::choose_div_and_prate(uint64 rate, uint32 *div, uint64 *prate, uint64 *avgrate) {
    rpi_clock *parent;

    parent = _cprman->get_clock(_parent);

    *prate = parent->get_rate();
    *div = choose_div(rate, *prate, true);

    *avgrate = static_cast<uint64>(rate_from_divisor(*prate, *div));

    if (_data->low_jitter && (*div & CM_DIV_FRAC_MASK)) {
        unsigned long high, low;
        uint32 int_div = *div & ~CM_DIV_FRAC_MASK;

        high = static_cast<unsigned long>(rate_from_divisor(*prate, int_div));
        int_div += CM_DIV_FRAC_MASK + 1;
        low = static_cast<unsigned long>(rate_from_divisor(*prate, int_div));

        /*
         * Return a value which is the maximum deviation
         * below the ideal rate, for use as a metric.
         */
        return *avgrate - max(*avgrate - low, high - *avgrate);
    }
    return *avgrate;
}

Errno
bcm2835_clock::determine_rate(struct clk_rate_request *req) {
    rpi_clock *parent;
    uint8 best_parent = BCM2711_INVALID;
    bool current_parent_is_pllc;
    uint64 rate, best_rate = 0;
    uint64 prate, best_prate = 0;
    uint64 avgrate, best_avgrate = 0;
    uint32 div, i;

    current_parent_is_pllc = (_parent >= BCM2835_PLLC_CORE0) && (_parent <= BCM2835_PLLC_PER);

    /*
     * Select parent clock that results in the closest but lower rate
     */
    for (i = 0; i < _data->num_mux_parents; ++i) {
        parent = _cprman->get_clock(_data->parents[i]);
        if (!parent) continue;

        /*
         * Don't choose a PLLC-derived clock as our parent
         * unless it had been manually set that way.  PLLC's
         * frequency gets adjusted by the firmware due to
         * over-temp or under-voltage conditions, without
         * prior notification to our clock consumer.
         */
        if (((_data->parents[i] >= BCM2835_PLLC_CORE0) && (_data->parents[i] <= BCM2835_PLLC_PER))
            && !current_parent_is_pllc)
            continue;

        rate = choose_div_and_prate(req->rate, &div, &prate, &avgrate);
        if (rate > best_rate && rate <= req->rate) {
            best_parent = _data->parents[i];
            best_prate = prate;
            best_rate = rate;
            best_avgrate = avgrate;
        }
    }

    if (best_parent == BCM2711_INVALID) return Errno::EINVAL;

    req->best_parent = best_parent;
    req->best_parent_rate = best_prate;

    req->rate = best_avgrate;

    return Errno::ENONE;
}

Errno
bcm2835_clock::set_parent(uint8 idx) {
    if (_data->parents[idx] == BCM2711_INVALID) return Errno::EINVAL;

    uint8 src = idx & CM_SRC_MASK;
    bool was_enabled = is_prepared();

    if (was_enabled) unprepare();

    uint32 ctrl = _cprman->read(_data->ctl_reg);
    ctrl &= ~(CM_SRC_MASK);
    ctrl |= src;
    _cprman->write(_data->ctl_reg, ctrl);
    _parent = _data->parents[idx];

    if (was_enabled) prepare();

    return Errno::ENONE;
}

uint8
bcm2835_clock::get_parent(void) {
    uint8 src = _cprman->read(_data->ctl_reg) & CM_SRC_MASK;

    if (src != _parent) _parent = src; /*changed by firmware?*/

    return src;
}

bool
bcm2835_vcpu_clock::is_prepared(void) {
    return true;
}

Errno
bcm2835_vcpu_clock::prepare(void) {
    return Errno::ENONE;
}

Errno
bcm2835_vcpu_clock::unprepare(void) {
    return Errno::ENOTSUP;
}

bcm2835_pll::bcm2835_pll(struct bcm2835_pll_data const *data) {
    /* All of the PLLs derive from the external oscillator. */
    _parent = data->parent;
    _data = data;
}

bcm2835_pll_divider::bcm2835_pll_divider(struct bcm2835_pll_divider_data const *data) {
    _parent = data->parent;
    _data = data;
}

bcm2835_clock::bcm2835_clock(struct bcm2835_clock_data const *data) { _data = data; }

void
bcm2835_clock::init(cprman *cm) {
    _cprman = cm;
    uint8 parent_idx = get_parent();
    rpi_clock *source = _cprman->get_clock(_data->parents[parent_idx]);

    /* try to set to a "valid" parent*/
    if (!source) {
        for (uint8 i = 0; i < _data->num_mux_parents; i++) {
            if (_cprman->get_clock(_data->parents[i]) != nullptr) {
                parent_idx = i;
                if (Errno::ENONE == set_parent(parent_idx)) break;
            }
        }
    }
    _parent = _data->parents[parent_idx];
}

bcm2835_vcpu_clock::bcm2835_vcpu_clock(struct bcm2835_clock_data const *data)
    : bcm2835_clock(data) {}

bcm2835_gate::bcm2835_gate(struct bcm2835_gate_data const *data) {
    _data = data;
    _parent = data->parent;
}

void
bcm2835_gate::init(cprman *cm) {
    _cprman = cm;
}

bool
bcm2835_gate::is_pll(void) {
    return false;
}

bool
bcm2835_gate::is_prepared(void) {
    return (_cprman->read(_data->ctl_reg) & CM_GATE) != 0;
}

Errno
bcm2835_gate::prepare(void) {
    _cprman->write(_data->ctl_reg, _cprman->read(_data->ctl_reg) | CM_GATE);
    return Errno::ENONE;
}

Errno
bcm2835_gate::unprepare(void) {
    _cprman->write(_data->ctl_reg, _cprman->read(_data->ctl_reg) & ~CM_GATE);
    return Errno::ENONE;
}

uint64
bcm2835_gate::get_rate(void) {
    return _cprman->get_clock(_parent)->get_rate();
}

Errno
bcm2835_gate::set_rate(uint64) {
    return Errno::ENOTSUP;
}

long
bcm2835_gate::round_rate(uint64) {
    return -1;
}

Errno
bcm2835_gate::determine_rate(struct clk_rate_request *) {
    return Errno::ENOTSUP;
}

Errno
bcm2835_gate::set_parent(uint8) {
    return Errno::ENOTSUP;
}

uint8
bcm2835_gate::get_parent(void) {
    return _parent;
}

bool
bcm2835_aux_clk::is_prepared(void) {
    return (_cprman->read_aux(_data->ctl_reg) & (1u << _shift)) != 0;
}

Errno
bcm2835_aux_clk::prepare(void) {
    _cprman->write_aux(_data->ctl_reg, _cprman->read_aux(_data->ctl_reg) | (1u << _shift));
    return Errno::ENONE;
}

Errno
bcm2835_aux_clk::unprepare(void) {
    _cprman->write_aux(_data->ctl_reg, _cprman->read_aux(_data->ctl_reg) & ~(1u << _shift));
    return Errno::ENONE;
}

cprman::cprman(void) {
    for (uint16 i = 0; i < BCM2711_CLOCK_TOTAL; i++)
        _clks[i] = nullptr;
}

cprman::~cprman(void) {}

/*Clock tree instantiation*/
/*BCM2711_FIXED_OSC*/
static bcm2835_fixed_clk rpi4_fixed_osc(54000000);
/*BCM2835_PLLA*/
static bcm2835_pll rpi4_plla(&plla);
/*BCM2835_PLLA_CORE*/
static bcm2835_pll_divider rpi4_plla_core(&plla_core);
/*BCM2835_PLLA_PER*/
static bcm2835_pll_divider rpi4_plla_per(&plla_per);
/*BCM2835_PLLA_DSI0*/
static bcm2835_pll_divider rpi4_plla_dsi0(&plla_dsi0);
/*BCM2835_PLLA_CCP2*/
static bcm2835_pll_divider rpi4_plla_ccp2(&plla_ccp2);
/*BCM2835_PLLC*/
static bcm2835_pll rpi4_pllc(&pllc);
/*BCM2835_PLLC_CORE0*/
static bcm2835_pll_divider rpi4_pllc_core0(&pllc_core0);
/*BCM2835_PLLC_CORE1*/
static bcm2835_pll_divider rpi4_pllc_core1(&pllc_core1);
/*BCM2835_PLLC_CORE2*/
static bcm2835_pll_divider rpi4_pllc_core2(&pllc_core2);
/*BCM2835_PLLC_PER*/
static bcm2835_pll_divider rpi4_pllc_per(&pllc_per);
/*BCM2835_PLLD*/
static bcm2835_pll rpi4_plld(&plld);
/*BCM2835_PLLD_CORE*/
static bcm2835_pll_divider rpi4_plld_core(&plld_core);
/*BCM2835_PLLD_PER*/
static bcm2835_pll_divider rpi4_plld_per(&plld_per);
/*BCM2835_PLLD_DSI0*/
static bcm2835_pll_divider rpi4_plld_dsi0(&plld_dsi0);
/*BCM2835_PLLD_DSI1*/
static bcm2835_pll_divider rpi4_plld_dsi1(&plld_dsi1);
/*BCM2835_CLOCK_OTP*/
static bcm2835_clock rpi4_otp(&otp_data);
/*BCM2835_CLOCK_TIMER*/
static bcm2835_clock rpi4_timer(&timer_data);
/*BCM2835_CLOCK_TSENS*/
static bcm2835_clock rpi4_tsense(&tsense_data);
/*BCM2835_CLOCK_TEC*/
static bcm2835_clock rpi4_tec(&tec_data);
/*BCM2835_CLOCK_H264*/
static bcm2835_clock rpi4_h264(&h264_data);
/*BCM2835_CLOCK_ISP*/
static bcm2835_clock rpi4_isp(&isp_data);
/*BCM2835_CLOCK_SDRAM*/
static bcm2835_clock rpi4_sdram(&sdram_data);
/*BCM2835_CLOCK_V3D*/
static bcm2835_clock rpi4_v3d(&v3d_data);
/*BCM2835_CLOCK_VPU*/
static bcm2835_vcpu_clock rpi4_vpu(&vpu_data);
/*BCM2835_CLOCK_AVEO*/
static bcm2835_clock rpi4_aveo(&aveo_data);
/*BCM2835_CLOCK_CAM0*/
static bcm2835_clock rpi4_cam0(&cam0_data);
/*BCM2835_CLOCK_CAM1*/
static bcm2835_clock rpi4_cam1(&cam1_data);
/*BCM2835_CLOCK_DFT*/
static bcm2835_clock rpi4_dft(&dft_data);
/*BCM2835_CLOCK_DPI*/
static bcm2835_clock rpi4_dpi(&dpi_data);
/*BCM2835_CLOCK_EMMC*/
static bcm2835_clock rpi4_emmc(&emmc_data);
/*BCM2711_CLOCK_EMMC2*/
static bcm2835_clock rpi4_emmc2(&emmc2_data);
/*BCM2835_CLOCK_GP0*/
static bcm2835_clock rpi4_gp0(&gp0_data);
/*BCM2835_CLOCK_GP1*/
static bcm2835_clock rpi4_gp1(&gp1_data);
/*BCM2835_CLOCK_GP2*/
static bcm2835_clock rpi4_gp2(&gp2_data);
/*BCM2835_CLOCK_HSM*/
static bcm2835_clock rpi4_hsm(&hsm_data);
/*BCM2835_CLOCK_PCM*/
static bcm2835_clock rpi4_pcm(&pcm_data);
/*BCM2835_CLOCK_PWM*/
static bcm2835_clock rpi4_pwm(&pwm_data);
/*BCM2835_CLOCK_SLIM*/
static bcm2835_clock rpi4_slim(&slim_data);
/*BCM2835_CLOCK_SMI*/
static bcm2835_clock rpi4_smi(&smi_data);
/*BCM2835_CLOCK_UART*/
static bcm2835_clock rpi4_uart(&uart_data);
/*BCM2835_CLOCK_VEC*/
static bcm2835_clock rpi4_vec(&vec_data);
/*BCM2835_CLOCK_DSI0E*/
static bcm2835_clock rpi4_dsi0e(&dsi0e_data);
/*BCM2835_CLOCK_DSI1E*/
static bcm2835_clock rpi4_dsi1e(&dsi1e_data);
/*BCM2835_CLOCK_PERI_IMAGE*/
static bcm2835_gate rpi4_peri_image(&peri_image_data);
/*BCM2711_CLOCK_AUX_UART*/
static bcm2835_aux_clk rpi4_aux_uart(&aux_uart_data, CM_AUX_UART_SHIFT);
/*BCM2711_CLOCK_AUX_SPI1*/
static bcm2835_aux_clk rpi4_aux_spi1(&aux_uart_data, CM_AUX_SPI1_SHIFT);
/*BCM2711_CLOCK_AUX_SPI2*/
static bcm2835_aux_clk rpi4_aux_spi2(&aux_uart_data, CM_AUX_SPI2_SHIFT);

Errno
cprman::probe(mword base, mword aux_base) {
    _base = base;
    _aux_base = aux_base;

    _clks[BCM2711_FIXED_OSC] = &rpi4_fixed_osc;

    _clks[BCM2835_PLLA] = &rpi4_plla;

    _clks[BCM2835_PLLA_CORE] = &rpi4_plla_core;

    _clks[BCM2835_PLLA_PER] = &rpi4_plla_per;

    _clks[BCM2835_PLLA_DSI0] = &rpi4_plla_dsi0;

    _clks[BCM2835_PLLA_CCP2] = &rpi4_plla_ccp2;

    /* PLLB is controlled by the vcpu firmare, driving core clocks.
     If guests need access to it, we can make it available via the mailbox interface. */

    _clks[BCM2835_PLLC] = &rpi4_pllc;

    _clks[BCM2835_PLLC_CORE0] = &rpi4_pllc_core0;

    _clks[BCM2835_PLLC_CORE1] = &rpi4_pllc_core1;

    _clks[BCM2835_PLLC_CORE2] = &rpi4_pllc_core2;

    _clks[BCM2835_PLLC_PER] = &rpi4_pllc_per;

    _clks[BCM2835_PLLD] = &rpi4_plld;

    _clks[BCM2835_PLLD_CORE] = &rpi4_plld_core;

    _clks[BCM2835_PLLD_PER] = &rpi4_plld_per;

    _clks[BCM2835_PLLD_DSI0] = &rpi4_plld_dsi0;

    _clks[BCM2835_PLLD_DSI1] = &rpi4_plld_dsi1;

    /* PLLH is not present in BCM2711, the process hangs when trying to enable it. */

    _clks[BCM2835_CLOCK_OTP] = &rpi4_otp;

    _clks[BCM2835_CLOCK_TIMER] = &rpi4_timer;

    _clks[BCM2835_CLOCK_TSENS] = &rpi4_tsense;

    _clks[BCM2835_CLOCK_TEC] = &rpi4_tec;

    _clks[BCM2835_CLOCK_H264] = &rpi4_h264;

    _clks[BCM2835_CLOCK_ISP] = &rpi4_isp;

    _clks[BCM2835_CLOCK_SDRAM] = &rpi4_sdram;

    _clks[BCM2835_CLOCK_V3D] = &rpi4_v3d;

    _clks[BCM2835_CLOCK_VPU] = &rpi4_vpu;

    _clks[BCM2835_CLOCK_AVEO] = &rpi4_aveo;

    _clks[BCM2835_CLOCK_CAM0] = &rpi4_cam0;

    _clks[BCM2835_CLOCK_CAM1] = &rpi4_cam1;

    _clks[BCM2835_CLOCK_DFT] = &rpi4_dft;

    _clks[BCM2835_CLOCK_DPI] = &rpi4_dpi;

    _clks[BCM2835_CLOCK_EMMC] = &rpi4_emmc;

    _clks[BCM2711_CLOCK_EMMC2] = &rpi4_emmc2;

    _clks[BCM2835_CLOCK_GP0] = &rpi4_gp0;

    _clks[BCM2835_CLOCK_GP1] = &rpi4_gp1;

    _clks[BCM2835_CLOCK_GP2] = &rpi4_gp2;

    _clks[BCM2835_CLOCK_HSM] = &rpi4_hsm;

    _clks[BCM2835_CLOCK_PCM] = &rpi4_pcm;

    _clks[BCM2835_CLOCK_PWM] = &rpi4_pwm;

    _clks[BCM2835_CLOCK_SLIM] = &rpi4_slim;

    _clks[BCM2835_CLOCK_SMI] = &rpi4_smi;

    _clks[BCM2835_CLOCK_UART] = &rpi4_uart;

    _clks[BCM2835_CLOCK_VEC] = &rpi4_vec;

    _clks[BCM2835_CLOCK_DSI0E] = &rpi4_dsi0e;

    _clks[BCM2835_CLOCK_DSI1E] = &rpi4_dsi1e;

    _clks[BCM2835_CLOCK_PERI_IMAGE] = &rpi4_peri_image;

    _clks[BCM2711_CLOCK_AUX_UART] = &rpi4_aux_uart;

    _clks[BCM2711_CLOCK_AUX_SPI1] = &rpi4_aux_spi1;

    _clks[BCM2711_CLOCK_AUX_SPI2] = &rpi4_aux_spi2;

    for (uint8 i = 0; i < BCM2711_CLOCK_TOTAL; i++)
        if (_clks[i]) _clks[i]->init(this);

    return Errno::ENONE;
}