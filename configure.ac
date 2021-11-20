AC_PREREQ([2.68])
AC_INIT([feud], [0.0.5], [vitalie@ciubotaru.tokyo])
AC_CONFIG_SRCDIR([feud.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [pow], [], AC_MSG_ERROR("math library not found"))
AC_CHECK_LIB([ncurses], [wgetch], [], AC_MSG_ERROR("ncurses library not found"))
# FIXME: Replace `main' with a function in `-lrt':
#AC_CHECK_LIB([rt], [main])

# Checks for header files.
AC_CHECK_HEADERS([ncurses.h stdlib.h string.h stdint.h endian.h])

# Checks for library functions.
AC_FUNC_MALLOC

AC_ARG_ENABLE(custom_rng,
AS_HELP_STRING([--enable-custom-rng],
               [enable a custom random number generator, default: no]),
[case "${enableval}" in
             yes) custom_rng=true ;;
             no)  custom_rng=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-custom-rng]) ;;
esac],
[custom_rng=false])

if test x"$custom_rng" = x"true"; then
AC_DEFINE([CUSTOM_RNG], [], [A custom RNG to be used instead of rand()])
fi

#AC_CONFIG_FILES([Makefile])
AC_OUTPUT([Makefile])
