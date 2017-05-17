rtems_tests_PROGRAMS = smpmpcp01
smpmpcp01_SOURCES = init.c

dist_rtems_tests_DATA = smpmpcp01.scn smpmpcp01.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(smpmpcp01_OBJECTS)
LINK_LIBS = $(smpmpcp01_LDLIBS)

smpmpcp01$(EXEEXT): $(smpmpcp01_OBJECTS) $(smpmpcp01_DEPENDENCIES)
	@rm -f smpmpcp01$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
