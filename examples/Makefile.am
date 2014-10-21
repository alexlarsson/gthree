NULL =

AM_CPPFLAGS =					\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	-DLIBEXECDIR=\"$(libexecdir)\"		\
	-DBINDIR=\"$(bindir)\"			\
	-DLIBDIR=\"$(libdir)\"			\
	$(GTHREE_CFLAGS)			\
	$(NULL)

noinst_PROGRAMS =				\
	gtkcube				\
	$(NULL)

gtkcube_CFLAGS = \
	$(GTHREE_CFLAGS)					\
	$(NULL)

gtkcube_SOURCES =						\
	gtkcube.c						\
	$(NULL)

gtkcube_LDADD = \
	$(GTHREE_LIBS)						\
	$(top_builddir)/gthree/libgthree-1.la			\
	$(NULL)

EXTRA_DIST =		\
	crate.gif	\
	$(NULL)
