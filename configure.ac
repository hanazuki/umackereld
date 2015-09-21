# -*- Autoconf -*-

AC_PREREQ([2.69])
AC_INIT([umackereld], [0.0.0], [kasumi@rollingapple.net])
AM_INIT_AUTOMAKE([foreign 1.15])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

AC_ARG_WITH([sysroot],
  AS_HELP_STRING([--with-sysroot], [Set system root path]))

AC_ARG_VAR(STAGING_DIR, [Path to the target directory in OpenWrt buildroot])

AC_PROG_CC

AC_CHECK_LIB([mbedtls], [ssl_init])
AC_CHECK_LIB([ubox], [uloop_init])
AC_CHECK_LIB([uci], [uci_alloc_context])
AC_CHECK_LIB([json-c], [json_object_new_object])
AC_CHECK_LIB([curl], [curl_global_init])

AC_CHECK_HEADERS([libubox/uloop.h libubox/ulog.h], [], [AC_MSG_ERROR("libubox is not found in include path.")])
AC_CHECK_HEADERS([uci.h], [], [AC_MSG_ERROR("libuci is not found in include path.")])
AC_CHECK_HEADERS([json-c/json.h], [], [AC_MSG_ERROR("libjson-c is not found in include path.")])
AC_CHECK_HEADERS([curl/curl.h], [], [AC_MSG_ERROR("libcurl is not found in include path.")])

AC_C_CONST

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
