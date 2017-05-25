if ENABLE_SNAP_PLUGIN

DISTCLEANFILES =
BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST = $(plugin_DATA)

plugindir = $(libdir)/gnome-builder/plugins
plugin_LTLIBRARIES = libsnap-plugin.la
dist_plugin_DATA = snap.plugin

libsnap_plugin_la_SOURCES =               \
	gbp-snap-plugin.c                 \
	gbp-snap-utils.c                  \
	gbp-snap-utils.h                  \
	gbp-snap-build-system.c           \
	gbp-snap-build-system.h           \
	gbp-snap-pipeline-addin.c         \
	gbp-snap-pipeline-addin.h         \
	$(NULL)

nodist_libsnap_plugin_la_SOURCES =        \
	gbp-snap-resources.c              \
	gbp-snap-resources.h

libsnap_plugin_la_CFLAGS = $(PLUGIN_CFLAGS) $(YAML_CFLAGS)
libsnap_plugin_la_LIBADD = $(YAML_LIBS)
libsnap_plugin_la_LDFLAGS = $(PLUGIN_LDFLAGS)

glib_resources_c = gbp-snap-resources.c
glib_resources_h = gbp-snap-resources.h
glib_resources_xml = gbp-snap-resources.gresource.xml
glib_resources_namespace = gbp_snap
include $(top_srcdir)/build/autotools/Makefile.am.gresources

include $(top_srcdir)/plugins/Makefile.plugin

endif

-include $(top_srcdir)/git.mk
