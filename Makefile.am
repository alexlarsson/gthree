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
	gthreeface.h \
	gthreegeometry.h \
	gthreematerial.h \
	gthreemesh.h \
	gthreeobject.h \
	gthreerenderer.h \
	gthreescene.h \
	$(NULL)

libgthree_la_SOURCES = \
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
	gthreemesh.c \
	gthreeobject.c \
	gthreeprivate.h \
	gthreerenderer.c \
	gthreescene.c \
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
	$(NULL)

gtkglobe_LDADD = \
	$(GTHREE_LIBS)						\
	libgthree.la						\
	$(NULL)
