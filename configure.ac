AC_PREREQ([2.63])

AC_INIT([gtkglobe],[0.0.1])

AC_PROG_CC
AM_PROG_CC_C_O
AC_DISABLE_STATIC

LT_PREREQ([2.2.6])
LT_INIT([disable-static])
LT_LIB_M

m4_define([glib_required_version], [2.42.0])

AM_PATH_GLIB_2_0(glib_required_version, :,
  AC_MSG_ERROR([
*** GLIB glib_required_version or better is required. The latest version of
*** GLIB is always available from ftp://ftp.gtk.org/pub/gtk/.]),
  gobject gmodule-no-export)

AC_CONFIG_SRCDIR([gthree/gthreeobject.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.11 no-define no-dist-gzip dist-bzip2 tar-ustar foreign])

# Enable silent rules is available
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE([enable])

if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[[\ \	]]-Wall[[\ \	]]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi

GTHREE_PACKAGES="gtk+-3.0 >= 3.15.1 graphene-1.0 >= 1.1.1"
GTHREE_PRIVATE_PACKAGES="epoxy json-glib-1.0 "
PKG_CHECK_MODULES(GTHREE, [$GTHREE_PACKAGES $GTHREE_PRIVATE_PACKAGES])
AC_SUBST(GTHREE_PACKAGES)
AC_SUBST(GTHREE_PRIVATE_PACKAGES)
AC_SUBST(GTHREE_CFLAGS)
AC_SUBST(GTHREE_LIBS)
AC_SUBST(LIBM)

GOBJECT_INTROSPECTION_CHECK([1.42])

AC_CONFIG_FILES([
gthree-1.0.pc
Makefile
examples/Makefile
gthree/Makefile
])
AC_OUTPUT
