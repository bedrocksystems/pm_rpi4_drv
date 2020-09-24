#
# Copyright (C) 2020 BedRock Systems, Inc.
#
# SPDX-License-Identifier: GPL-2.0
#
PBL_ROOT	?= ../../lib/pebble/
PBL_SRC		?= ../../lib/pebble/src/
BRS_ROOT	?= ../../zeta/
LIBDIR		= ../../lib/

LINK_SCRIPT = $(OBJDIR)pebble.lds

APPNAME = pm_rpi4_drv
CC_SRCS = main.cpp rpi4.cpp rpi_clock.cpp

include deps.mk

OBJS     = $(CC_SRCS:%.cpp=$(OBJDIR)%.o)

include $(PBL_ROOT)support/build/rules.mk

# custom rule to make sure the linker file is pulled from pebble
$(OBJDIR)%.lds : $(PBL_SRC)$(ARCH_INC)/%.lds $(BUILD_CONF_DEPS)
	@- mkdir -p $(@D) 2>&1 > /dev/null
	@$(ECHO) "$(TAB)[CPP] $(subst $(GIT_DIR),,$(abspath $@))"
	$(CPP) -I. -I$(TOPDIR)include $(SFLAGS) -MT $@ -P $< -o $@
