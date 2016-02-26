#ifndef PHP_LIBRPIP_H
#define PHP_LIBRPIP_H 

#define PHP_LIBRPIP_VERSION "0.1.0"
#define PHP_LIBRPIP_EXTNAME "librpip"

// declaration of a custom my_function()
PHP_FUNCTION(my_function);

extern zend_module_entry librpip_module_entry;
#define phpext_librpip_ptr &librpip_module_entry

#endif