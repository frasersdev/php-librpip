#ifndef PHP_LIBRPIP_H
#define PHP_LIBRPIP_H 

#define PHP_LIBRPIP_VERSION "0.1.0"
#define PHP_LIBRPIP_EXTNAME "librpip"

//basic module functions
PHP_MINIT_FUNCTION(librpip);
PHP_MINFO_FUNCTION(librpip);
PHP_MSHUTDOWN_FUNCTION(librpip)


//librpip functions
PHP_FUNCTION(librpip_GetBoardID);
PHP_FUNCTION(librpip_VersionStr);
PHP_FUNCTION(librpip_ErrorGetCode);
PHP_FUNCTION(librpip_ErrorGetDescription);

extern zend_module_entry librpip_module_entry;
#define phpext_librpip_ptr &librpip_module_entry

#endif