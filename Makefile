include ../thumb/Makedefs

#------------------------------------------------------------------------------

OBJS= view-gui.o view-app.o panoptic.o
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
	$(RM) $(OBJS) $(DEPS) panoptic

#------------------------------------------------------------------------------

scm : .FORCE
	$(MAKE) -C scm

.FORCE :

#------------------------------------------------------------------------------

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPS)
endif

export CC
export CXX
export CFLAGS
