#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <php.h>
#include "php-librpip.h"
 

// set {NULL, NULL, NULL} as the last record to mark the end of list
static zend_function_entry librpip_functions[] = {
    PHP_FE(my_function, NULL)
    {NULL, NULL, NULL}
};
 
// the following code creates an entry for the module and registers it with Zend.
zend_module_entry librpip_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_LIBRPIP_EXTNAME,
    librpip_functions,
    NULL, // name of the MINIT function or NULL if not applicable
    NULL, // name of the MSHUTDOWN function or NULL if not applicable
    NULL, // name of the RINIT function or NULL if not applicable
    NULL, // name of the RSHUTDOWN function or NULL if not applicable
    NULL, // name of the MINFO function or NULL if not applicable
    PHP_LIBRPIP_VERSION,
    STANDARD_MODULE_PROPERTIES
};
 
ZEND_GET_MODULE(librpip)
 
// implementation of a custom my_function()
PHP_FUNCTION(my_function)
{
    RETURN_STRING("This is my function", 1);
}

