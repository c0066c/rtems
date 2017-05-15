rtems_tests_PROGRAMS = smpdpcp01
smpdpcp01_SOURCES = init.c

dist_rtems_tests_DATA = smpdpcp01.scn smpdpcp01.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(smpdpcp01_OBJECTS)
LINK_LIBS = $(smpdpcp01_LDLIBS)

smpdpcp01$(EXEEXT): $(smpdpcp01_OBJECTS) $(smpdpcp01_DEPENDENCIES)
	@rm -f smpdpcp01$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
