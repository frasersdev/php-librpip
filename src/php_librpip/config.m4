PHP_ARG_ENABLE(php_librpip, whether to enable php_librpip,
[ --enable-php_librpip   Enable php_librpip])
 
if test "$PHP_LIBRPIP" = "yes"; then
  AC_DEFINE(HAVE_LIBRPIP, 1, [Whether you have php_librpip])
  PHP_NEW_EXTENSION(php_librpip, php_librpip.c, $ext_shared)
fi