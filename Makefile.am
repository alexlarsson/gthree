NULL =

AM_CPPFLAGS =					\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	-DLIBEXECDIR=\"$(libexecdir)\"		\
	-DBINDIR=\"$(bindir)\"			\
	-DLIBDIR=\"$(libdir)\"			\
	$(NULL)

noinst_LTLIBRARIES = libgthree.la

bin_PROGRAMS =					\
	gtkglobe				\
	$(NULL)

libgthree_ladir = $(includedir)/gthree

libgthree_la_HEADERS = \
	gthreearea.h \
	gthreecamera.h \
	gthreeenums.h \
	gthreeface.h \
	gthreegeometry.h \
	gthreematerial.h \
	gthreebasicmaterial.h \
	gthreemultimaterial.h \
	gthreemesh.h \
	gthreetexture.h \
	gthreeobject.h \
	gthreerenderer.h \
	gthreescene.h \
	gthreetypes.h \
	$(NULL)

libgthree_la_SOURCES = \
	gthree-resources.c \
	gthree-resources.h \
	gthreearea.c \
	gthreebuffer.c \
	gthreebuffer.h \
	gthreecamera.c \
	gthreeface.c \
	gthreegeometry-utils.c \
	gthreegeometry.c \
	gthreegeometrygroup.c \
	gthreegeometrygroup.h \
	gthreematerial.c \
	gthreebasicmaterial.c \
	gthreemultimaterial.c \
	gthreemesh.c \
	gthreeobject.c \
	gthreeprivate.h \
	gthreeprogram.c \
	gthreeprogram.h \
	gthreeuniforms.c \
	gthreeuniforms.h \
	gthreerenderer.c \
	gthreescene.c \
	gthreeshader.c \
	gthreeshader.h \
	gthreetexture.c \
	$(NULL)

libgthree_la_CFLAGS = \
	$(GTHREE_CFLAGS)					\
	$(NULL)

libgthree_la_LIBADD =						\
	$(GTHREE_LIBS)					\
	$(NULL)

gtkglobe_CFLAGS = \
	$(GTHREE_CFLAGS)					\
	$(NULL)

gtkglobe_SOURCES =						\
	gtkglobe.c						\
	gthree-resources.c					\
	$(NULL)

gtkglobe_LDADD = \
	$(GTHREE_LIBS)						\
	libgthree.la						\
	$(NULL)

resource_files = $(shell glib-compile-resources --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/gthree.gresource.xml)

gthree-resources.c: gthree.gresource.xml $(resource_files)
	glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-source --c-name gthree $(srcdir)/gthree.gresource.xml

gthree-resources.h: gthree.gresource.xml $(resource_files)
	glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-header --c-name gthree $(srcdir)/gthree.gresource.xml
