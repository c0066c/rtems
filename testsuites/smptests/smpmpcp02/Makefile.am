rtems_tests_PROGRAMS = smpmpcp02
smpmpcp02_SOURCES = init.c

dist_rtems_tests_DATA = smpmpcp02.scn smpmpcp02.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(smpmpcp02_OBJECTS)
LINK_LIBS = $(smpmpcp02_LDLIBS)

smpmpcp02$(EXEEXT): $(smpmpcp02_OBJECTS) $(smpmpcp02_DEPENDENCIES)
	@rm -f smpmpcp02$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
