# Panoptic -- Linux / OS X Makefile

# Panoptic assumes that Thumb and SCM can be found in adjacent directories
# and both have been built: that libthumb, libscm, and bin2c all exist.

THUMB_DIR = ../thumb
SCM_DIR   = ../scm

# Panoptic configuration piggy-backs atop the Thumb configuration.

include $(THUMB_DIR)/Makedefs

LIBS   += -L$(THUMB_DIR)/$(CONFIG) -L$(SCM_DIR)/$(CONFIG) -lthumb -lscm
CFLAGS += -I$(THUMB_DIR)/include   -I$(SCM_DIR)

B2C = $(THUMB_DIR)/etc/bin2c

#------------------------------------------------------------------------------

OBJS= view-gui.o view-app.o panoptic.o data.o
DEPS= $(OBJS:.o=.d)
TARG= panoptic

#------------------------------------------------------------------------------

$(CONFIG)/$(TARG): $(CONFIG) $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

$(CONFIG) :
	mkdir -p $(CONFIG)

clean:
	$(RM) $(OBJS) $(DEPS) $(TARG) data/data.zip

#------------------------------------------------------------------------------

data.cpp : data/data.zip
	$(B2C) panoptic_data < $< > $@

data/data.zip :
	$(MAKE) -C data

#------------------------------------------------------------------------------
# Package the target in a ZIP archive, including the OS and date in the name.

VER = $(shell date "+%Y%m%d")

ifdef OSX
	DIST = panoptic-$(VER)-osx.zip
endif
ifdef LINUX
	DIST = panoptic-$(VER)-lin.zip
endif

dist : $(CONFIG)/$(TARG)
	strip $(CONFIG)/$(TARG)
	zip -9 $(DIST) $(CONFIG)/$(TARG)

#------------------------------------------------------------------------------

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif
