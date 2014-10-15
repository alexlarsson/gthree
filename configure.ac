AC_PREREQ([2.63])

AC_INIT([gtkglobe],[0.0.1])

AC_PROG_CC
AM_PROG_CC_C_O
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

AC_CONFIG_SRCDIR([gthreeobject.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.11 no-define no-dist-gzip dist-bzip2 tar-ustar foreign])

# Enable silent rules is available
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE([enable])
AC_CONFIG_FILES([Makefile])

if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[[\ \	]]-Wall[[\ \	]]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi

pkg_modules="gtk+-3.0 epoxy json-glib-1.0 graphene-1.0"
PKG_CHECK_MODULES(GTHREE, [$pkg_modules])
AC_SUBST(GTHREE_CFLAGS)
AC_SUBST(GTHREE_LIBS)


AC_OUTPUT
