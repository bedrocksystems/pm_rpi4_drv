/*
 * Copyright (c) 2020 BedRock Systems, Inc.
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#pragma once
#include <pebble/pebble.hpp>
#include <pebble/types.hpp>

#define ASSERT(_expr_)                                                                             \
    do {                                                                                           \
        if (__builtin_expect(!(_expr_), 0)) {                                                      \
            __builtin_trap();                                                                      \
        }                                                                                          \
    } while (false);

__ALWAYS_INLINE__
static inline void
early_error(mword err) {
    void (*bad_function)() = *(reinterpret_cast<void (*)()>(0xbad00000 + err));
    bad_function();
}

namespace Pm {

/* used for bulk requests to the pin controller */
typedef struct Pin_t {
    uint32 id;
    uint32 val;
} Pin;

enum iopad : uint8 {
    PAD_NONE = 0,
    PULLUP = 1,
    PULLDOWN = 2,
};

enum iotrig : uint32 {
    TRIG_NONE = 0,
    LEVEL_HIGH = (1u << 0),
    LEVEL_LOW = (1u << 1),
    EDGE_RISE = (1u << 2),
    EDGE_FALL = (1u << 3),
    EDGE_RISE_ASYNC = (1u << 4),
    EDGE_FALL_ASYNC = (1u << 5),
    TRIG_CLR = (1u << 17),
    TRIG_MASK = ((1u << 16) - 1),
};

enum ioval : uint8 {
    CLR = 0,
    SET = 1,
};

typedef struct {
    uint32 min;
    uint32 max;
    uint32 step;
    bool triplet;
} clk_desc;

}

/* pinctrl protocol is not part of SCMI */
enum Pm_custom_ops {
    PM_ENABLE_NODE = 0x5,
    PM_SET_PINFUNC = 0x15u,
    PM_GET_PINFUNC = 0x16u,
    PM_SET_PINPAD = 0x17u,
    PM_GET_PINPAD = 0x18u,
    PM_SET_GPIO = 0x19u,
    PM_GET_GPIO = 0x1au,
    PM_GET_GPIOTRIG = 0x1bu,
    PM_SET_GPIOTRIG = 0x1cu,
    PM_GET_GPIOEVT = 0x1eu,
    PM_CLR_GPIOEVT = 0x1fu,
};

struct Ennode_args_ipc {
    uint64 node_id;
};

struct Ennode_ret_ipc {
    Errno err;
};

struct Pin_args_ipc {
    uint32 num_pins;
    Pm::Pin pins[];
};

struct Pin_ret_ipc {
    uint32 retval;
    Pm::Pin pins[];
};
