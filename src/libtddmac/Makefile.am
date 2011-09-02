INCLUDES = -I$(top_builddir) \
	   -I$(top_srcdir)/include

lib_LTLIBRARIES = libtddmac.la

libtddmac_la_SOURCES = \
	tddmac.c

libtddmac_la_CFLAGS = $(UIOMUX_CFLAGS) -Wall -Werror
libtddmac_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@ @SHLIB_VERSION_ARG@
libtddmac_la_LIBADD = $(UIOMUX_LIBS)
