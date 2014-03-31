include ../thumb/Makedefs

#------------------------------------------------------------------------------

OBJS= view-gui.o view-app.o panoptic.o data.o
DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

CFLAGS += -I../thumb/include
THUMB   = -L../thumb/src -lthumb
SCM     = scm/libscm.a
TARG    = panoptic

#------------------------------------------------------------------------------

$(TARG): scm $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $(OBJS) $(THUMB) $(SCM) $(LIBS)

clean:
	$(RM) $(OBJS) $(DEPS) $(TARG) data.zip

#------------------------------------------------------------------------------

scm : .FORCE
	$(MAKE) -C scm

.FORCE :

#------------------------------------------------------------------------------
# Package the contents of the data directory in an embedded ZIP archive.

DATA= $(shell find data -name \*.md   \
                     -o -name \*.xml  \
                     -o -name \*.csv  \
                     -o -name \*.vert \
                     -o -name \*.frag)

data.zip : $(DATA)
	(cd data && zip -FS9r ../data.zip $(subst data/,,$(DATA)))

data.cpp : data.zip
	xxd -i data.zip > data.cpp

#------------------------------------------------------------------------------
# Package the target in a ZIP archive, including the OS and date in the name.

VER = $(shell date "+%Y%m%d")

ifdef ISMACOS
	ZIP = $(TARG)-osx-$(VER).zip
endif
ifdef ISLINUX
	ZIP = $(TARG)-lin-$(VER).zip
endif
ifdef ISMINGW
	ZIP = $(TARG)-win-$(VER).zip
endif

dist : $(TARG)
	strip $(TARG)
	zip $(ZIP) $(TARG)

#------------------------------------------------------------------------------

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

export CC
export CXX
export CFLAGS
