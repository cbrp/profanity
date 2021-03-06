#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([profanity], [0.1.7], [boothj5web@gmail.com])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign subdir-objects])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([ncurses], [main], [], 
    [AC_MSG_ERROR([ncurses is required for profanity])])
AC_CHECK_LIB([resolv], [main], [], 
    [AC_MSG_ERROR([libresolv is required for profanity])])
AC_CHECK_LIB([ssl], [main], [], 
    [AC_MSG_ERROR([openssl is required for profanity])])
AC_CHECK_LIB([expat], [main], [], 
    [AC_MSG_ERROR([expat is required for profanity])])
AC_CHECK_LIB([xml2], [main], [], 
    [AC_MSG_ERROR([xml2 is required for profanity])])
AC_CHECK_LIB([strophe], [main], [], 
    [AC_MSG_ERROR([libstrophe is required for profanity])])
AC_CHECK_LIB([glib-2.0], [main], [], 
    [AC_MSG_ERROR([glib-2.0 is required for profanity])])
AC_CHECK_LIB([curl], [main], [], 
    [AC_MSG_ERROR([libcurl is required for profanity])])
AC_CHECK_LIB([notify], [main], [], 
    [AC_MSG_NOTICE([libnotify not found])])
AC_CHECK_LIB([headunit], [main], [], 
    [AC_MSG_NOTICE([headunit not found, will not be able to run tests])])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

PKG_CHECK_MODULES([DEPS], [openssl glib-2.0 libcurl])
PKG_CHECK_MODULES([NOTIFY], [libnotify], [], 
    [AC_MSG_NOTICE([libnotify module not found])])

AM_CFLAGS="-O3 -Werror -Wall -Wextra -Wno-unused-parameter "        
AM_CFLAGS="$AM_CFLAGS -Wno-unused-but-set-variable -Wno-unused-result "
AM_CFLAGS="$AM_CFLAGS -Wno-missing-field-initializers "
AM_CFLAGS="$AM_CFLAGS -lstrophe -lxml2 -lexpat -lncurses -lcurl -lresolv "
AM_CFLAGS="$AM_CFLAGS $DEPS_LIBS $NOTIFY_LIBS"

AM_CPPFLAGS="$DEPS_CFLAGS $NOTIFY_CFLAGS"

AC_SUBST(AM_CFLAGS)
AC_SUBST(AM_CPPFLAGS)

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([atexit memset strdup strndup strstr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
