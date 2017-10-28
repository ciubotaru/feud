AC_PREREQ([2.68])
AC_INIT([feud], [0.0.1], [vitalie@ciubotaru.tokyo])
AC_CONFIG_SRCDIR([feud.c])
#AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [ncurses])
# FIXME: Replace `main' with a function in `-lrt':
#AC_CHECK_LIB([rt], [main])

# Checks for header files.
AC_CHECK_HEADERS([ncurses.h stdlib.h string.h stdint.h endian.h])

# Checks for library functions.
AC_FUNC_MALLOC

#AC_CONFIG_FILES([Makefile])
AC_OUTPUT([Makefile])
