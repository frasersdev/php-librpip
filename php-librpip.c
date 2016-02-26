#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <php.h>
#include <librpip.h>
#include "php-librpip.h"
 

// set {NULL, NULL, NULL} as the last record to mark the end of list
static zend_function_entry librpip_functions[] = {
	PHP_FE(librpip_GetBoardID, NULL)
	PHP_FE(librpip_VersionStr, NULL)
	PHP_FE(librpip_ErrorGetCode, NULL)
	PHP_FE(librpip_ErrorGetDescription, NULL)	
	{NULL, NULL, NULL}
};
 
// the following code creates an entry for the module and registers it with Zend.
zend_module_entry librpip_module_entry = {
    STANDARD_MODULE_HEADER,
    PHP_LIBRPIP_EXTNAME,
    librpip_functions,
    PHP_MINIT(librpip), // name of the MINIT function or NULL if not applicable
    PHP_MSHUTDOWN(librpip), // name of the MSHUTDOWN function or NULL if not applicable
    NULL, // name of the RINIT function or NULL if not applicable
    NULL, // name of the RSHUTDOWN function or NULL if not applicable
    PHP_MINFO(librpip), // name of the MINFO function or NULL if not applicable
    PHP_LIBRPIP_VERSION,
    STANDARD_MODULE_PROPERTIES
};
 
ZEND_GET_MODULE(librpip)

PHP_MINFO_FUNCTION(librpip)
{
    php_info_print_table_start();
    php_info_print_table_row(2, "librpip support", "enabled");
    php_info_print_table_end();
}

PHP_MINIT_FUNCTION(librpip)
{
    uint32_t fs;
    fs = librpipInit(LIBRPIP_BOARD_DETECT, 0, 0);
}

PHP_MSHUTDOWN_FUNCTION(librpip)
{
    librpipClose();
}

// functions
PHP_FUNCTION(librpip_GetBoardID)
{
	RETURN_LONG(librpipGetBoardID());
}

PHP_FUNCTION(librpip_VersionStr)
{
	char version[40];
	librpipVersionStr(&version[0], sizeof(version));
	RETURN_STRING(version, 1);
}

PHP_FUNCTION(librpip_ErrorGetCode)
{
	RETURN_LONG(librpipErrorGetCode());
}

PHP_FUNCTION(librpip_ErrorGetDescription)
{
	char errordesc[200];
	librpipErrorGetDescription(&errordesc[0], sizeof(errordesc));
	RETURN_STRING(errordesc, 1);
}