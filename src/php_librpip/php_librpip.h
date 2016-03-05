#ifndef PHP_LIBRPIP_H
#define PHP_LIBRPIP_H 

#define PHP_LIBRPIP_VERSION "0.1.0"
#define PHP_LIBRPIP_EXTNAME "librpip"


#ifdef ZTS
#include "TSRM.h"
#endif
ZEND_BEGIN_MODULE_GLOBALS(librpip)
	long featureset;
	long boardid;
	long validpins;
ZEND_END_MODULE_GLOBALS(librpip)

#ifdef ZTS
#define LIBRPIP_G(v) TSRMG(librpip_globals_id, zend_librpip_globals *, v)
#else
#define LIBRPIP_G(v) (librpip_globals.v)
#endif


//basic module functions
PHP_MINFO_FUNCTION(librpip);
PHP_MINIT_FUNCTION(librpip);
PHP_RINIT_FUNCTION(librpip);


//librpip functions
PHP_FUNCTION(librpip_FeatureSet);
PHP_FUNCTION(librpip_GetBoardID);
PHP_FUNCTION(librpip_GetBoardName);
PHP_FUNCTION(librpip_Version);
PHP_FUNCTION(librpip_GpioConfigWrite);
PHP_FUNCTION(librpip_GpioPinWrite);
PHP_FUNCTION(librpip_GpioPinToggle);
PHP_FUNCTION(librpip_GpioPinPulse);
PHP_FUNCTION(librpip_GpioPinEventWait);
PHP_FUNCTION(librpip_GpioGetValidPins);
PHP_FUNCTION(librpip_I2cConfigWrite);
PHP_FUNCTION(librpip_PwmConfigWrite);
PHP_FUNCTION(librpip_PwmStatusWrite);
PHP_FUNCTION(librpip_PwmDutyPercentWrite);
PHP_FUNCTION(librpip_ServoConfigWrite);
PHP_FUNCTION(librpip_ServoPositionWrite);
PHP_FUNCTION(librpip_SpiConfigWrite);

extern zend_module_entry librpip_module_entry;
#define phpext_librpip_ptr &librpip_module_entry

uint32_t get_features_info(char* str, int len, uint32_t fs);
uint32_t get_variable_uint(char* variable, int cmd_len, int init);
uint32_t get_variable_str(char* variable, int cmd_len, char* str, int str_len, int init);
uint32_t run_function_write(char class, char* func, int func_len, char* func_params, int func_params_len);
uint32_t do_socket_comms(char* cmd, int cmd_len, char* response, int response_len);

#endif
