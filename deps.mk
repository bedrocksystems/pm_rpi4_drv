LIBS = pebble
LINKLIBS = pebble

$(eval $(call dep_hook,pm_rpi4_drv,$(LIBS)))
