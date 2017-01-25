
rtems_tests_PROGRAMS = sp78
sp78_SOURCES = init.c ../../support/src/spin.c

dist_rtems_tests_DATA = sp78.scn
dist_rtems_tests_DATA += sp78.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am


AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(sp78_OBJECTS)
LINK_LIBS = $(sp78_LDLIBS)

sp78$(EXEEXT): $(sp78_OBJECTS) $(sp78_DEPENDENCIES)
	@rm -f sp78$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
