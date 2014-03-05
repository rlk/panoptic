include ../thumb/Makedefs

#------------------------------------------------------------------------------

OBJS= view-gui.o view-app.o panoptic.o data.o
DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

CFLAGS += -I../thumb/include
THUMB   = -L../thumb/src -lthumb
SCM     = scm/libscm.a

#------------------------------------------------------------------------------

all : panoptic

panoptic: scm $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $(OBJS) $(THUMB) $(SCM) $(LIBS)

clean:
	$(RM) $(OBJS) $(DEPS) panoptic data.zip

#------------------------------------------------------------------------------

scm : .FORCE
	$(MAKE) -C scm

.FORCE :

#------------------------------------------------------------------------------

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

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

export CC
export CXX
export CFLAGS
