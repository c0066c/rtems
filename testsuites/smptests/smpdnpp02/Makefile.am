rtems_tests_PROGRAMS = smpdnpp02
smpdnpp02_SOURCES = init.c

dist_rtems_tests_DATA = smpdnpp02.scn smpdnpp02.doc

include $(RTEMS_ROOT)/make/custom/@RTEMS_BSP@.cfg
include $(top_srcdir)/../automake/compile.am
include $(top_srcdir)/../automake/leaf.am

AM_CPPFLAGS += -I$(top_srcdir)/../support/include

LINK_OBJS = $(smpdnpp02_OBJECTS)
LINK_LIBS = $(smpdnpp02_LDLIBS)

smpdnpp02$(EXEEXT): $(smpdnpp02_OBJECTS) $(smpdnpp02_DEPENDENCIES)
	@rm -f smpdnpp02$(EXEEXT)
	$(make-exe)

include $(top_srcdir)/../automake/local.am
