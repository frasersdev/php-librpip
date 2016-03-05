PHP_ARG_ENABLE(librpip, whether to enable librpip,
[ --enable-librpip   Enable librpip])

if test "$PHP_LIBRPIP" = "yes"; then
  AC_DEFINE(HAVE_LIBRPIP, 1, [Whether you have librpip])
  PHP_NEW_EXTENSION(librpip, php_librpip.c, $ext_shared)
fi