AC_INIT([binrsync], [0.1], [clabbe.montjoie@gmail.com])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AM_CONDITIONAL([IS_NUNUX], [test `uname` = 'Linux'])
AM_CONDITIONAL([IS_SOLARIS], [test `uname` = 'SunOS'])
AM_CONDITIONAL([IS_OPENBSD], [test `uname` = 'OpenBSD'])
AM_CONDITIONAL([NUM_RELEASE], [test `uname` = 'Linux'])

AC_PREREQ
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])

AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADERS([stdlib.h])
AC_CHECK_HEADERS([string.h])
AC_CHECK_HEADERS([unistd.h])

AC_CHECK_FUNCS([strerror])

AC_C_INLINE

AC_FUNC_MALLOC

AC_CONFIG_FILES([
 Makefile
])
AC_OUTPUT

cat <<EOF

------------------ Summary ------------------
  Prefix.........: $prefix
  C Compiler.....: $CC $CFLAGS $CPPFLAGS
  Linker.........: $LD $LDFLAGS $LIBS
---------------------------------------------

Check the above options and compile with:
 ${MAKE-make}

EOF
