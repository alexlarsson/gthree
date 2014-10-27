NULL =

AM_CPPFLAGS =					\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	-DLIBEXECDIR=\"$(libexecdir)\"		\
	-DBINDIR=\"$(bindir)\"			\
	-DLIBDIR=\"$(libdir)\"			\
	$(GTHREE_CFLAGS)			\
	$(NULL)

noinst_PROGRAMS =			\
	gtkcube				\
	model				\
	materials			\
	envmap				\
	$(NULL)

gtkcube_CFLAGS = \
	$(GTHREE_CFLAGS)					\
	$(NULL)

gtkcube_SOURCES =						\
	gtkcube.c						\
	utils.c							\
	$(NULL)

gtkcube_LDADD = \
	$(GTHREE_LIBS)						\
	$(top_builddir)/gthree/libgthree-1.la			\
	$(NULL)

model_CFLAGS = \
	$(GTHREE_CFLAGS)					\
	$(NULL)

model_SOURCES =						\
	model.c						\
	utils.c						\
	$(NULL)

model_LDADD = \
	$(GTHREE_LIBS)						\
	$(top_builddir)/gthree/libgthree-1.la			\
	$(NULL)

materials_CFLAGS = \
	$(GTHREE_CFLAGS)					\
	$(NULL)

materials_SOURCES =						\
	materials.c						\
	utils.c							\
	$(NULL)

materials_LDADD = \
	$(GTHREE_LIBS)						\
	$(top_builddir)/gthree/libgthree-1.la			\
	$(NULL)

envmap_CFLAGS = \
	$(GTHREE_CFLAGS)					\
	$(NULL)

envmap_SOURCES =						\
	envmap.c						\
	utils.c							\
	$(NULL)

envmap_LDADD = \
	$(GTHREE_LIBS)						\
	$(top_builddir)/gthree/libgthree-1.la			\
	$(NULL)

EXTRA_DIST =		\
	crate.gif	\
	$(NULL)
