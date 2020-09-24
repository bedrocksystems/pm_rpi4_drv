/*
 * Copyright (c) 2020 BedRock Systems, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#pragma once
#include <pm.hpp>

namespace drv_ipc {

enum method {
    HANDLE_SMC = 1,
    CLK_IS_ENABLED,
    CLK_GET_MAX,
    CLK_ENABLE,
    CLK_DISABLE,
    CLK_GET_RATE,
    CLK_SET_RATE,
    CLK_DESCRIBE_RATE,
    NODE_GET_MAX,
    NODE_ENABLE,
    NODE_DISABLE,
    PINCTRL_HANDLE,
};

struct header {
    method id;
    header() = delete;
    constexpr header(method _id) : id(_id) {}

    __ALWAYS_INLINE__
    constexpr inline size_t size() const { return size_t(1); }
};

struct ret {
    Errno errno;

    __ALWAYS_INLINE__
    constexpr inline size_t size() const { return size_t(1); }
};

struct clk_enable_args : header {
    uint64 clk_id;

    clk_enable_args(uint64 _id) : header(CLK_ENABLE), clk_id(_id) {}

    __ALWAYS_INLINE__
    constexpr static inline size_t size() {
        return (sizeof(clk_enable_args) + sizeof(mword) - 1) / sizeof(mword);
    }
};

struct clk_enable_ret : ret {};

struct clk_disable_args : header {
    uint64 clk_id;

    clk_disable_args(uint64 _id) : header(CLK_DISABLE), clk_id(_id) {}

    __ALWAYS_INLINE__
    constexpr static inline size_t size() {
        return (sizeof(clk_disable_args) + sizeof(mword) - 1) / sizeof(mword);
    }
};

struct clk_disable_ret : ret {};

struct clk_is_enabled_args : header {
    uint64 clk_id;

    clk_is_enabled_args(uint64 _id) : header(CLK_IS_ENABLED), clk_id(_id) {}

    __ALWAYS_INLINE__
    constexpr static inline size_t size() {
        return (sizeof(clk_is_enabled_args) + sizeof(mword) - 1) / sizeof(mword);
    }
};

struct clk_is_enabled_ret : ret {
    uint8 enabled;

    __ALWAYS_INLINE__
    constexpr static inline size_t size() {
        return (sizeof(clk_is_enabled_ret) + sizeof(mword) - 1) / sizeof(mword);
    }
};

struct clk_get_max_args : header {
    clk_get_max_args(void) : header(CLK_GET_MAX) {}
};

struct clk_get_max_ret : ret {
    uint32 max_id;

    __ALWAYS_INLINE__
    constexpr static inline size_t size() {
        return (sizeof(clk_get_max_ret) + sizeof(mword) - 1) / sizeof(mword);
    }
};

struct clk_get_rate_args : header {
    uint64 clk_id;

    clk_get_rate_args(uint64 _id) : header(CLK_GET_RATE), clk_id(_id) {}

    __ALWAYS_INLINE__
    constexpr static inline size_t size() {
        return (sizeof(clk_get_rate_args) + sizeof(mword) - 1) / sizeof(mword);
    }
};

struct clk_get_rate_ret : ret {
    uint64 rate;

    __ALWAYS_INLINE__
    constexpr static inline size_t size() {
        return (sizeof(clk_get_rate_ret) + sizeof(mword) - 1) / sizeof(mword);
    }
};

struct clk_set_rate_args : header {
    uint64 clk_id;
    uint64 rate;

    clk_set_rate_args(uint64 _id, uint64 _rate) : header(CLK_GET_RATE), clk_id(_id), rate(_rate) {}

    __ALWAYS_INLINE__
    constexpr static inline size_t size() {
        return (sizeof(clk_set_rate_args) + sizeof(mword) - 1) / sizeof(mword);
    }
};

struct clk_set_rate_ret : ret {};

struct clk_describe_rate_args : header {
    uint64 clk_id;

    clk_describe_rate_args(uint64 _id) : header(CLK_DESCRIBE_RATE), clk_id(_id) {}

    __ALWAYS_INLINE__
    constexpr static inline size_t size() {
        return (sizeof(clk_describe_rate_args) + sizeof(mword) - 1) / sizeof(mword);
    }
};

struct clk_describe_rate_ret : ret {
    Pm::clk_desc desc;

    __ALWAYS_INLINE__
    constexpr static inline size_t size() {
        return (sizeof(clk_describe_rate_ret) + sizeof(mword) - 1) / sizeof(mword);
    }
};

struct node_get_max_args : header {
    node_get_max_args(void) : header(NODE_GET_MAX) {}
};

struct node_get_max_ret : ret {
    uint32 max_id;

    __ALWAYS_INLINE__
    constexpr static inline size_t size() {
        return (sizeof(node_get_max_ret) + sizeof(mword) - 1) / sizeof(mword);
    }
};

struct node_disable_args : header {
    uint64 node_id;

    node_disable_args(uint64 _id) : header(NODE_DISABLE), node_id(_id) {}

    __ALWAYS_INLINE__
    constexpr static inline size_t size() {
        return (sizeof(node_disable_args) + sizeof(mword) - 1) / sizeof(mword);
    }
};

struct node_disable_ret : ret {};

struct node_enable_args : header {
    uint64 node_id;

    node_enable_args(uint64 _id) : header(NODE_DISABLE), node_id(_id) {}

    __ALWAYS_INLINE__
    constexpr static inline size_t size() {
        return (sizeof(node_enable_args) + sizeof(mword) - 1) / sizeof(mword);
    }
};

struct node_enable_ret : ret {};

struct pinctrl_args_ipc : header {
    uint32 func;
    uint32 num_pins;
    Pm::Pin pins[];

    pinctrl_args_ipc(uint8 _func, Pm::Pin *_pins, uint32 _num_pins) : header(PINCTRL_HANDLE) {
        num_pins = _num_pins;
        func = _func;
        for (uint8 i = 0; i < num_pins; i++)
            pins[i] = _pins[i];
    }
    /*Size must be explicit!*/
};

struct pinctrl_ret_ipc : ret {
    Pm::Pin pins[];
    /*Size must be explicit!*/
};

}
