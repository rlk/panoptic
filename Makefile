include ../thumb/Makedefs

#------------------------------------------------------------------------------

OBJS= view-load.o view-app.o panoptic.o
DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

CFLAGS += -I../thumb/include
LIBS   += -L../thumb/src -lthumb scm/libscm.a

#------------------------------------------------------------------------------

all : panoptic

panoptic: scm $(OBJS) $(THUMB)
	$(CXX) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

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

export CFLAGS


