NULL =

AM_CPPFLAGS =					\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	-DLIBEXECDIR=\"$(libexecdir)\"		\
	-DBINDIR=\"$(bindir)\"			\
	-DLIBDIR=\"$(libdir)\"			\
	-DGTHREE_COMPILATION			\
	$(NULL)

lib_LTLIBRARIES = libgthree-1.la

libgthree_1_ladir = $(includedir)/gthree

gthree_public_h_sources = \
	gthree.h \
	gthreearea.h \
	gthreebasicmaterial.h \
	gthreecamera.h \
	gthreeenums.h \
	gthreeface.h \
	gthreegeometry.h \
	gthreematerial.h \
	gthreemesh.h \
	gthreemultimaterial.h \
	gthreeobject.h \
	gthreerenderer.h \
	gthreescene.h \
	gthreetexture.h \
	gthreetypes.h \
	gthreeprogram.h			\
	gthreeshader.h \
	gthreeuniforms.h \
	gthreeutils.h			\
	$(NULL)

gthree_private_h_sources =		\
	gthree-resources.h		\
	gthreebufferprivate.h		\
	gthreegeometrygroupprivate.h	\
	gthreeobjectprivate.h		\
	gthreeprivate.h			\
	$(NULL)

gthree_built_private_headers =			\
	gthree-resources.h			\
	$(NULL)

gthree_built_sources =				\
	gthree-resources.c			\
	${gthree_built_private_headers}		\
	$(NULL)

gthree_c_sources = \
	gthree-resources.c			\
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
	gthreeutils.c			\
	gthreerenderer.c \
	gthreescene.c \
	gthreeshader.c \
	gthreeshader.h \
	gthreetexture.c \
	$(NULL)

MAINTAINERCLEANFILES = \
	$(gthree_built_sources)			\
	$(NULL)

gthreeincludedir = $(includedir)/gthree-1.0/gthree
gthreeinclude_HEADERS = $(gthree_public_h_sources)

EXTRA_DIST = $(gthree_private_h_sources) $(gthree_built_sources)

BUILT_SOURCES = \
	$(gthree_built_sources) \
	$(NULL)

resource_files = $(shell glib-compile-resources --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/gthree.gresource.xml)

gthree-resources.c: gthree.gresource.xml $(resource_files)
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-source --c-name gthree $(srcdir)/gthree.gresource.xml

gthree-resources.h: gthree.gresource.xml $(resource_files)
	$(AM_V_GEN) glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-header --c-name gthree $(srcdir)/gthree.gresource.xml


libgthree_1_la_SOURCES = $(gthree_c_sources)

libgthree_1_la_CFLAGS = \
	$(GTHREE_CFLAGS)					\
	$(NULL)

libgthree_1_la_LIBADD =						\
	$(GTHREE_LIBS)					\
	$(NULL)


EXTRA_DIST =
