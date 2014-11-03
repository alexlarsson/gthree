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
	cubes				\
	multi				\
	model				\
	materials			\
	envmap				\
	shader				\
	$(NULL)

cubes_CFLAGS = \
	$(GTHREE_CFLAGS)					\
	$(NULL)

cubes_SOURCES =						\
	cubes.c						\
	utils.c							\
	$(NULL)

cubes_LDADD = \
	$(GTHREE_LIBS)						\
	$(top_builddir)/gthree/libgthree-1.la			\
	$(NULL)

multi_CFLAGS = \
	$(GTHREE_CFLAGS)					\
	$(NULL)

multi_SOURCES =						\
	multi.c						\
	utils.c							\
	$(NULL)

multi_LDADD = \
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

shader_CFLAGS = \
	$(GTHREE_CFLAGS)					\
	$(NULL)

shader_SOURCES =						\
	shader.c						\
	utils.c							\
	$(NULL)

shader_LDADD = \
	$(GTHREE_LIBS)						\
	$(top_builddir)/gthree/libgthree-1.la			\
	$(NULL)

EXTRA_DIST =		\
	crate.gif	\
	$(NULL)
