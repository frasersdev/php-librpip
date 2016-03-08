/*
 * php_librpip - a php extension to use Raspberry PI peripherals 
 * from php.
 * 
 * Copyright (C) 2016 Fraser Stuart

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
 
 #ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <php.h>
#include <librpip.h>
#include "php_librpip.h"
 

// set {NULL, NULL, NULL} as the last record to mark the end of list
static zend_function_entry librpip_functions[] = {
	PHP_FE(librpip_FeatureSet, NULL)
	PHP_FE(librpip_GetBoardID, NULL)
	PHP_FE(librpip_GetBoardName, NULL)	
	PHP_FE(librpip_Version, NULL)
	PHP_FE(librpip_GpioConfigPinRead, NULL)	
	PHP_FE(librpip_GpioConfigPinWrite, NULL)
	PHP_FE(librpip_GpioPinRead, NULL)
	PHP_FE(librpip_GpioPinWrite, NULL)
	PHP_FE(librpip_GpioPinToggle, NULL)
	PHP_FE(librpip_GpioPinPulse, NULL)
	PHP_FE(librpip_GpioPinEvent, NULL)
	PHP_FE(librpip_GpioPinEventWait, NULL)
	PHP_FE(librpip_GpioGetValidPins, NULL)
	PHP_FE(librpip_I2cConfigRead, NULL)
	PHP_FE(librpip_I2cConfigWrite, NULL)
	PHP_FE(librpip_PwmConfigRead, NULL)
	PHP_FE(librpip_PwmConfigWrite, NULL)
	PHP_FE(librpip_PwmStatusRead, NULL)
	PHP_FE(librpip_PwmStatusWrite, NULL)
	PHP_FE(librpip_PwmDutyPercentWrite, NULL)
	PHP_FE(librpip_ServoConfigRead, NULL)
	PHP_FE(librpip_ServoConfigWrite, NULL)
	PHP_FE(librpip_ServoPositionWrite, NULL)
	PHP_FE(librpip_SpiConfigRead, NULL)	
	PHP_FE(librpip_SpiConfigWrite, NULL)
	PHP_FE(librpip_UartConfigRead, NULL)
	PHP_FE(librpip_UartConfigWrite, NULL)			
	{NULL, NULL, NULL}
};
 
// the following code creates an entry for the module and registers it with Zend.
zend_module_entry librpip_module_entry = {
	STANDARD_MODULE_HEADER,
	PHP_LIBRPIP_EXTNAME,
	librpip_functions,
	PHP_MINIT(librpip), 
	NULL, // name of the MSHUTDOWN function or NULL if not applicable
	PHP_RINIT(librpip), 
	NULL, // name of the RSHUTDOWN function or NULL if not applicable
	PHP_MINFO(librpip), 
	PHP_LIBRPIP_VERSION,
	STANDARD_MODULE_PROPERTIES
};
 
ZEND_GET_MODULE(librpip)

ZEND_DECLARE_MODULE_GLOBALS(librpip)

PHP_MINFO_FUNCTION(librpip) {
	char string[100]={0};
	
	php_info_print_table_start();
	php_info_print_table_row(2, "librpip support", "enabled");
	php_info_print_table_row(2, "version", PHP_LIBRPIP_VERSION);

	char fstring[80]={0};
	get_features_info(fstring, sizeof(fstring), LIBRPIP_G(featureset));
	snprintf(string,sizeof(string),"%s (0x%x)", fstring, LIBRPIP_G(featureset));		
	php_info_print_table_row(2, "feature set", string);	
		
	get_variable_str("BoardDesc", 9, string, sizeof(string), 1);	
	php_info_print_table_row(2, "detected board", string);	
		
	get_variable_str("Version", 7, string, sizeof(string), 1);
	php_info_print_table_row(2, "librpip library version", string);	
	php_info_print_table_end();
}

PHP_MINIT_FUNCTION(librpip) {
	REGISTER_LONG_CONSTANT("LIBRPIP_GPIO_FLAG_FNC_IN", 		0x001, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("LIBRPIP_GPIO_FLAG_FNC_OUT", 		0x002, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("LIBRPIP_GPIO_FLAG_PUD_OFF", 		0x004, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("LIBRPIP_GPIO_FLAG_PUD_DOWN", 		0x008, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("LIBRPIP_GPIO_FLAG_PUD_UP", 		0x010, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("LIBRPIP_GPIO_FLAG_ED_OFF", 		0x020, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("LIBRPIP_GPIO_FLAG_ED_RISE", 		0x040, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("LIBRPIP_GPIO_FLAG_ED_FALL", 		0x080, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("LIBRPIP_GPIO_FLAG_ED_HIGH", 		0x100, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("LIBRPIP_GPIO_FLAG_ED_LOW", 		0x200, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("LIBRPIP_GPIO_FLAG_ED_ARISE", 		0x400, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("LIBRPIP_GPIO_FLAG_ED_AFALL", 		0x800, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("LIBRPIP_I2C_FLAG_PEC", 			0x1, CONST_CS | CONST_PERSISTENT); 
	REGISTER_LONG_CONSTANT("LIBRPIP_PWM_STATUS_OFF", 		0x0, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_PWM_STATUS_ON", 		0x1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_PWM_FLAG_POLARITY_NORMAL", 	0x1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_PWM_FLAG_POLARITY_INVERTED", 	0x2, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_SPI_MODE_0", 			0x0, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_SPI_MODE_1", 			0x1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_SPI_MODE_2", 			0x2, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_SPI_MODE_3", 			0x3, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_SPI_FLAG_CS_HIGH", 		0x04, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_SPI_FLAG_NO_CS", 		0x40, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_1200", 		1200, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_2400", 		2400, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_4800", 		4800, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_9600", 		9600, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_19200", 		19200, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_38400", 		38400, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_57600", 		57600, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_115200", 		115200, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_230400", 		230400, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_460800", 		460800, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_500000", 		500000, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_576000", 		576000, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_921600", 		921600, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_1000000", 		1000000, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_1152000", 		1152000, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_1500000", 		1500000, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_2000000", 		2000000, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_2500000", 		2500000, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_3000000", 		3000000, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_3500000", 		3500000, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_BAUD_4000000", 		4000000, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_SIZE_5", 			5, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_SIZE_6", 			6, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_SIZE_7", 			7, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_SIZE_8", 			8, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_PARITY_OFF", 		0, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_PARITY_ODD", 		1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_PARITY_EVEN", 		2, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_STOPBITS_1", 		1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_STOPBITS_2", 		2, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_MODE_BINARY", 		0, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("LIBRPIP_UART_MODE_ASCII	", 		1, CONST_CS | CONST_PERSISTENT);	
    	return SUCCESS;  
}

PHP_RINIT_FUNCTION(librpip) {
	LIBRPIP_G(featureset) = get_variable_uint("FeatureSet", 10, 1);
	LIBRPIP_G(boardid) = get_variable_uint("BoardID", 7, 1);
	LIBRPIP_G(validpins) = get_variable_uint("GpioGetValidPins", 16, 0);	
    	return SUCCESS;    
    
}

// functions
PHP_FUNCTION(librpip_FeatureSet) {
	RETURN_LONG(LIBRPIP_G(featureset));
}

PHP_FUNCTION(librpip_GetBoardID) {
	RETURN_LONG(LIBRPIP_G(boardid));
}

PHP_FUNCTION(librpip_GetBoardName) {
	char boardname[60]={0};
	get_variable_str("BoardDesc", 9, boardname, sizeof(boardname), 0);
	RETURN_STRING(boardname, 1);
}

PHP_FUNCTION(librpip_Version) {
	char version[40]={0};
	get_variable_str("Version", 7, version, sizeof(version), 0);
	RETURN_STRING(version, 1);
}

PHP_FUNCTION(librpip_GpioConfigPinRead) {
	if(ZEND_NUM_ARGS() != 1) WRONG_PARAM_COUNT;
	
	long pin;
	long flags;
	zval* results;
	zval* flag;
	char* val;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &pin) == FAILURE) {
    		RETURN_NULL();
	}
	

	char params[40]={0};
	char result[40]={0};	
	
	snprintf(params,sizeof(params),"%u",pin);

	if(!run_function_read('G', "GpioConfigPinRead", 17, params, strlen(params), result, strlen(result))) 
		RETURN_NULL();	
	
	val = strtok(result, " ");
	flags=get_response_uint();
	
	MAKE_STD_ZVAL(results);	
	array_init(results);
	
	
	MAKE_STD_ZVAL(flag);	
	array_init(flag);
	
	add_assoc_long(flag, "raw", flags);
	
	if(flags & LIBRPIP_GPIO_FLAG_FNC_IN) 		add_assoc_string(flag, "function", "in", 1);
	else if(flags & LIBRPIP_GPIO_FLAG_FNC_OUT) 	add_assoc_string(flag, "function", "out", 1);
	
	add_assoc_string(flag, "pullupdown", "unknown", 1);	
	
	if(flags & LIBRPIP_GPIO_FLAG_ED_OFF) 		add_assoc_string(flag, "eventdetect", "off", 1);
	else if(flags & LIBRPIP_GPIO_FLAG_ED_RISE) 	add_assoc_string(flag, "eventdetect", "rise", 1);
	else if(flags & LIBRPIP_GPIO_FLAG_ED_FALL) 	add_assoc_string(flag, "eventdetect", "fall", 1);
	else if(flags & LIBRPIP_GPIO_FLAG_ED_HIGH) 	add_assoc_string(flag, "eventdetect", "high", 1);
	else if(flags & LIBRPIP_GPIO_FLAG_ED_LOW) 	add_assoc_string(flag, "eventdetect", "low", 1);
	else if(flags & LIBRPIP_GPIO_FLAG_ED_ARISE) 	add_assoc_string(flag, "eventdetect", "async rise", 1);
	else if(flags & LIBRPIP_GPIO_FLAG_ED_AFALL) 	add_assoc_string(flag, "eventdetect", "async fall", 1);
	
	add_assoc_zval(results, "flags", flag);

	RETURN_ZVAL(results, 1, 1);	
}

PHP_FUNCTION(librpip_GpioConfigPinWrite) {

	if(ZEND_NUM_ARGS() != 2) WRONG_PARAM_COUNT;
	
	long pin;
	long flags;
	

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &pin, &flags) == FAILURE) {
    		RETURN_FALSE;
	}

	char params[40]={0};
	
	snprintf(params,sizeof(params),"%u %u",pin,flags);

	if(!run_function_write('G', "GpioConfigPinWrite", 18, params, strlen(params))) 
		RETURN_FALSE;	
		
	RETURN_TRUE;	
}

PHP_FUNCTION(librpip_GpioPinRead) {
	if(ZEND_NUM_ARGS() != 1) WRONG_PARAM_COUNT;
	
	long pin;

	char* val;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &pin) == FAILURE) {
    		RETURN_NULL();
	}
	

	char params[40]={0};
	char result[40]={0};	
	
	snprintf(params,sizeof(params),"%u",pin);

	if(!run_function_read('G', "GpioPinRead", 11, params, strlen(params), result, strlen(result))) 
		RETURN_NULL();	
	
	val = strtok(result, " ");

	RETURN_LONG(get_response_uint());	
}

PHP_FUNCTION(librpip_GpioPinWrite) {

	if(ZEND_NUM_ARGS() != 2) WRONG_PARAM_COUNT;
	
	long pin;
	long value;
	

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &pin, &value) == FAILURE) {
    		RETURN_FALSE;
	}

	char params[40]={0};
	
	snprintf(params,sizeof(params),"%u %u",pin,value);

	if(!run_function_write('G', "GpioPinWrite", 12, params, strlen(params))) 
		RETURN_FALSE;	
		
	RETURN_TRUE;	
}

PHP_FUNCTION(librpip_GpioPinToggle) {

	if(ZEND_NUM_ARGS() != 1) WRONG_PARAM_COUNT;
	
	long pin;
	

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &pin) == FAILURE) {
    		RETURN_FALSE;
	}

	char params[40]={0};
	
	snprintf(params,sizeof(params),"%u",pin);

	if(!run_function_write('G', "GpioPinToggle", 13, params, strlen(params))) 
		RETURN_FALSE;	
		
	RETURN_TRUE;	
}

PHP_FUNCTION(librpip_GpioPinPulse) {

	if(ZEND_NUM_ARGS() != 2) WRONG_PARAM_COUNT;
	
	long pin;
	long length;
	

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &pin, &length) == FAILURE) {
    		RETURN_FALSE;
	}

	char params[40]={0};
	
	snprintf(params,sizeof(params),"%u %u",pin,length);

	if(!run_function_write('G', "GpioPinPulse", 12, params, strlen(params))) 
		RETURN_FALSE;	
		
	RETURN_TRUE;	
}

PHP_FUNCTION(librpip_GpioPinEvent) {
	if(ZEND_NUM_ARGS() != 1) WRONG_PARAM_COUNT;
	
	long pin;

	char* val;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &pin) == FAILURE) {
    		RETURN_NULL();
	}
	

	char params[40]={0};
	char result[40]={0};	
	
	snprintf(params,sizeof(params),"%u",pin);

	if(!run_function_read('G', "GpioPinEventTest", 16, params, strlen(params), result, strlen(result))) 
		RETURN_NULL();	
	
	val = strtok(result, " ");

	if(get_response_uint()) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;	
	}
}

PHP_FUNCTION(librpip_GpioPinEventWait) {

	if(ZEND_NUM_ARGS() != 2) WRONG_PARAM_COUNT;
	
	long pin;
	long timeout;
	

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &pin, &timeout) == FAILURE) {
    		RETURN_FALSE;
	}

	char params[40]={0};
	
	snprintf(params,sizeof(params),"%u %u",pin,timeout);

	if(!run_function_write('G', "GpioPinEventWait", 16, params, strlen(params))) 
		RETURN_FALSE;	
		
	RETURN_TRUE;	
}

PHP_FUNCTION(librpip_GpioGetValidPins) {
	RETURN_LONG(LIBRPIP_G(validpins));
}

PHP_FUNCTION(librpip_I2cConfigRead) {
	if(ZEND_NUM_ARGS() != 1) WRONG_PARAM_COUNT;
	
	long id;
	long flags;
	zval* results;
	zval* flag;
	char* val;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &id) == FAILURE) {
    		RETURN_NULL();
	}
	

	char params[40]={0};
	char result[40]={0};	
	
	snprintf(params,sizeof(params),"%u",id);

	if(!run_function_read('I', "I2cConfigRead", 13, params, strlen(params), result, strlen(result))) 
		RETURN_NULL();	
	
	val = strtok(result, " ");
	flags=get_response_uint();
	
	MAKE_STD_ZVAL(results);	
	array_init(results);
	
	MAKE_STD_ZVAL(flag);	
	array_init(flag);
	
	add_assoc_long(flag, "raw", flags);
	add_assoc_bool(flag, "pec", (flags & LIBRPIP_I2C_FLAG_PEC) ? 1 : 0);
	
	add_assoc_zval(results, "flags", flag);	

	RETURN_ZVAL(results, 1, 1);	
}

PHP_FUNCTION(librpip_I2cConfigWrite) {

	if(ZEND_NUM_ARGS() != 2) WRONG_PARAM_COUNT;
	
	long id;
	long flags;
	

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &id, &flags) == FAILURE) {
    		RETURN_FALSE;
	}

	char params[40]={0};
	
	snprintf(params,sizeof(params),"%u %u",id,flags);

	if(!run_function_write('I', "I2cConfigWrite", 14, params, strlen(params))) 
		RETURN_FALSE;	
		
	RETURN_TRUE;	
}

PHP_FUNCTION(librpip_PwmConfigRead) {
	if(ZEND_NUM_ARGS() != 1) WRONG_PARAM_COUNT;
	
	long id;
	long flags;
	zval* results;
	zval* flag;
	char* val;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &id) == FAILURE) {
    		RETURN_NULL();
	}
	
	char params[40]={0};
	char result[80]={0};	
	
	snprintf(params,sizeof(params),"%u",id);

	if(!run_function_read('P', "PwmConfigRead", 13, params, strlen(params), result, strlen(result))) 
		RETURN_NULL();	
	
	val = strtok(result, " ");
	
	MAKE_STD_ZVAL(results);	
	array_init(results);
	add_assoc_long(results, "pin", get_response_uint());
	add_assoc_long(results, "period", get_response_uint());
	add_assoc_long(results, "duty_cycle", get_response_uint());
	
	flags=get_response_uint();
	
	MAKE_STD_ZVAL(flag);	
	array_init(flag);
	add_assoc_long(flag, "raw", flags);
	if(flags & LIBRPIP_PWM_FLAG_POLARITY_NORMAL) 		add_assoc_string(flag, "polarity", "normal", 1);
	else if(flags & LIBRPIP_PWM_FLAG_POLARITY_INVERTED) 	add_assoc_string(flag, "polarity", "inverted", 1);
	
	add_assoc_zval(results, "flags", flag);	

	RETURN_ZVAL(results, 1, 1);	
}

PHP_FUNCTION(librpip_PwmConfigWrite) {

	if(ZEND_NUM_ARGS() != 4) WRONG_PARAM_COUNT;
	
	long id;
	long period;
	long duty_cycle;
	long flags;		
	

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &id, &period, &duty_cycle, &flags) == FAILURE) {
    		RETURN_FALSE;
	}

	char params[40]={0};
	
	snprintf(params,sizeof(params),"%u %u %u %u",id, period, duty_cycle, flags);

	if(!run_function_write('P', "PwmConfigWrite", 14, params, strlen(params))) 
		RETURN_FALSE;	
		
	RETURN_TRUE;	
}

PHP_FUNCTION(librpip_PwmStatusRead) {
	if(ZEND_NUM_ARGS() != 1) WRONG_PARAM_COUNT;
	
	long id;
	zval* results;
	char* val;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &id) == FAILURE) {
    		RETURN_NULL();
	}
	
	char params[40]={0};
	char result[40]={0};	
	
	snprintf(params,sizeof(params),"%u",id);

	if(!run_function_read('P', "PwmStatusRead", 13, params, strlen(params), result, strlen(result))) 
		RETURN_NULL();	
	
	val = strtok(result, " ");
	
	MAKE_STD_ZVAL(results);	
	array_init(results);
	add_assoc_long(results, "status", get_response_uint());

	RETURN_ZVAL(results, 1, 1);	
}

PHP_FUNCTION(librpip_PwmStatusWrite) {

	if(ZEND_NUM_ARGS() != 2) WRONG_PARAM_COUNT;
	
	long id;
	long status;		
	

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &id, &status) == FAILURE) {
    		RETURN_FALSE;
	}

	char params[40]={0};
	
	snprintf(params,sizeof(params),"%u %u",id, status);

	if(!run_function_write('P', "PwmStatusWrite", 14, params, strlen(params))) 
		RETURN_FALSE;	
		
	RETURN_TRUE;	
}

PHP_FUNCTION(librpip_PwmDutyPercentWrite) {

	if(ZEND_NUM_ARGS() != 2) WRONG_PARAM_COUNT;
	
	long id;
	double duty;		
	

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ld", &id, &duty) == FAILURE) {
    		RETURN_FALSE;
	}

	char params[40]={0};
	
	snprintf(params,sizeof(params),"%u %.3f",id, duty);

	if(!run_function_write('P', "PwmDutyPercentWrite", 19, params, strlen(params))) 
		RETURN_FALSE;	
		
	RETURN_TRUE;	
}

PHP_FUNCTION(librpip_ServoConfigRead) {
	if(ZEND_NUM_ARGS() != 1) WRONG_PARAM_COUNT;
	
	long id;
	zval* results;
	char* val;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &id) == FAILURE) {
    		RETURN_NULL();
	}
	
	char params[40]={0};
	char result[80]={0};	
	
	snprintf(params,sizeof(params),"%u",id);

	if(!run_function_read('P', "ServoConfigRead", 15, params, strlen(params), result, strlen(result))) 
		RETURN_NULL();	
	
	val = strtok(result, " ");
	
	MAKE_STD_ZVAL(results);	
	array_init(results);
	add_assoc_long(results, "range", get_response_uint());
	add_assoc_long(results, "pmin", get_response_uint());
	add_assoc_long(results, "pmax", get_response_uint());

	RETURN_ZVAL(results, 1, 1);	
}

PHP_FUNCTION(librpip_ServoConfigWrite) {

	if(ZEND_NUM_ARGS() != 4) WRONG_PARAM_COUNT;
	
	long id;
	long range;
	long pmin;
	long pmax;		
	

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &id, &range, &pmin, &pmax) == FAILURE) {
    		RETURN_FALSE;
	}

	char params[40]={0};
	
	snprintf(params,sizeof(params),"%u %u %u %u",id, range, pmin, pmax);

	if(!run_function_write('P', "ServoConfigWrite", 16, params, strlen(params))) 
		RETURN_FALSE;	
		
	RETURN_TRUE;	
}

PHP_FUNCTION(librpip_ServoPositionWrite) {

	if(ZEND_NUM_ARGS() != 2) WRONG_PARAM_COUNT;
	
	long id;
	double angle;		
	

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ld", &id, &angle) == FAILURE) {
    		RETURN_FALSE;
	}

	char params[40]={0};
	
	snprintf(params,sizeof(params),"%u %.3f",id, angle);

	if(!run_function_write('P', "ServoPositionWrite", 18, params, strlen(params))) 
		RETURN_FALSE;	
		
	RETURN_TRUE;	
}

PHP_FUNCTION(librpip_SpiConfigRead) {
	if(ZEND_NUM_ARGS() != 2) WRONG_PARAM_COUNT;
	
	long id;
	long cs;	
	long flags;
	zval* results;
	zval* flag;
	char* val;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &id, &cs) == FAILURE) {
    		RETURN_NULL();
	}
	
	char params[40]={0};
	char result[80]={0};	
	
	snprintf(params,sizeof(params),"%u %u",id,cs);

	if(!run_function_read('S', "SpiConfigRead", 13, params, strlen(params), result, strlen(result))) 
		RETURN_NULL();	
	
	val = strtok(result, " ");
	
	MAKE_STD_ZVAL(results);	
	array_init(results);
	add_assoc_long(results, "spi_mode", get_response_uint());
	add_assoc_long(results, "lsb_first", get_response_uint());
	add_assoc_long(results, "bits_per_word", get_response_uint());
	add_assoc_long(results, "max_speed", get_response_uint());
	
	flags=get_response_uint();
	
	MAKE_STD_ZVAL(flag);	
	array_init(flag);
	add_assoc_long(flag, "raw", flags);
	add_assoc_bool(flag, "cshigh", (flags & LIBRPIP_SPI_FLAG_CS_HIGH) ? 1 : 0);
	add_assoc_bool(flag, "nocs", (flags & LIBRPIP_SPI_FLAG_NO_CS) ? 1 : 0);	
	
	add_assoc_zval(results, "flags", flag);	

	RETURN_ZVAL(results, 1, 1);	
}
	
PHP_FUNCTION(librpip_SpiConfigWrite) {

	if(ZEND_NUM_ARGS() != 5) WRONG_PARAM_COUNT;
	
	long id;
	long cs;
	long spi_mode;
	long max_speed;
	long spi_flags;
	

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lllll", &id, &cs, &spi_mode, &max_speed, &spi_flags) == FAILURE) {
    		RETURN_FALSE;
	}

	char params[40]={0};
	
	snprintf(params,sizeof(params),"%u %u %u %u %u",id, cs, spi_mode, max_speed, spi_flags);

	if(!run_function_write('S', "SpiConfigWrite", 14, params, strlen(params))) 
		RETURN_FALSE;	
		
	RETURN_TRUE;	
}

PHP_FUNCTION(librpip_UartConfigRead) {
	if(ZEND_NUM_ARGS() != 1) WRONG_PARAM_COUNT;
	
	long id;	
	zval* results;
	char* val;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &id) == FAILURE) {
    		RETURN_NULL();
	}
	
	char params[40]={0};
	char result[80]={0};	
	
	snprintf(params,sizeof(params),"%u",id);

	if(!run_function_read('U', "UartConfigRead", 14, params, strlen(params), result, strlen(result))) 
		RETURN_NULL();	
	
	val = strtok(result, " ");
	
	MAKE_STD_ZVAL(results);	
	array_init(results);
	add_assoc_long(results, "baud", get_response_uint());
	add_assoc_long(results, "csize", get_response_uint());
	add_assoc_long(results, "parity", get_response_uint());
	add_assoc_long(results, "stop", get_response_uint());
	add_assoc_long(results, "mode", get_response_uint());	


	RETURN_ZVAL(results, 1, 1);	
}
	
PHP_FUNCTION(librpip_UartConfigWrite) {

	if(ZEND_NUM_ARGS() != 5) WRONG_PARAM_COUNT;
	
	long id;
	long baud;
	long csize;
	long parity;
	long stop;
	long mode;	
	

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llllll", &id, &baud, &csize, &parity, &stop, &mode) == FAILURE) {
    		RETURN_FALSE;
	}

	char params[80]={0};
	
	snprintf(params,sizeof(params),"%u %u %u %u %u %u",id, baud, csize, parity, stop, mode);

	if(!run_function_write('U', "UartConfigWrite", 15, params, strlen(params))) 
		RETURN_FALSE;	
		
	RETURN_TRUE;	
}
	

//internal functions
uint32_t get_features_info(char* str, int len, uint32_t fs) {

	char gpio[8]={0};
	char i2c0[8]={0};
	char i2c1[8]={0};
	char pwm0[8]={0};
	char pwm1[8]={0};
	char spi0[10]={0};
	char spi1[10]={0};		
	char uart0[9]={0};
	char uart1[9]={0};
	
	if(fs>0) {
		if(fs & LIBRPIP_FEATURE_GPIO) strcpy(gpio,"GPIO, ");
		if(fs & LIBRPIP_FEATURE_I2C0) strcpy(i2c0,"I2C0, ");
		if(fs & LIBRPIP_FEATURE_I2C1) strcpy(i2c1,"I2C1, ");	
		if(fs & LIBRPIP_FEATURE_PWM0) strcpy(pwm0,"PWM0, ");
		if(fs & LIBRPIP_FEATURE_PWM1) strcpy(pwm1,"PWM1, ");
		if(fs & LIBRPIP_FEATURE_SPI0) strcpy(spi0,"SPI0, ");
		if(fs & LIBRPIP_FEATURE_SPI1) strcpy(spi1,"SPI1, ");	
		if(fs & LIBRPIP_FEATURE_UART0) strcpy(uart0,"UART0, ");
		if(fs & LIBRPIP_FEATURE_UART1) strcpy(uart1,"UART1, ");
	
		snprintf(str,len,"%s%s%s%s%s%s%s%s%s",gpio,i2c0,i2c1,pwm0,pwm1,spi0,spi1,uart0,uart1);
	} else {
		snprintf(str,len,"librpip is not initialised");	
	}
	return strlen(str);
						
}

uint32_t get_variable_uint(char* variable, int cmd_len, int init) {

	char cmd[50]={0};
	char resp[300]={0};
	char* val;
	
	uint32_t value=0;
	snprintf(cmd,50,"V %s",variable);
	if(do_socket_comms(cmd, strlen(cmd), resp, 300) > 0) {
		switch(resp[0]) {
			case 'Y':
				val=&resp[2];
				value=atoi(val);
				break;
			case 'N':
				val=&resp[2];
				if(init) php_error(E_WARNING, "Sockrpip call unsuccessful during module init. Message was '%s'", val);
				else 	php_error(E_WARNING, "Sockrpip call unsuccessful during %s(). Message was '%s'", get_active_function_name(TSRMLS_C), val);	
				break;	
			case 'S':
				val=&resp[2];
				if(init) php_error(E_WARNING, "Sockrpip socket error during module init. Error was '%s'", val);
				else 	php_error(E_WARNING, "Sockrpip socket error during %s(). Error was '%s'", get_active_function_name(TSRMLS_C), val);
				break;			
			case 'X':
				val=&resp[2];
				if(init) php_error(E_WARNING, "Sockrpip command error during module init. Error was '%s'", val);
				else 	php_error(E_WARNING, "Sockrpip command error during %s(). Error was '%s'", get_active_function_name(TSRMLS_C), val);
				break;
			default:	
				if(init) php_error(E_WARNING, "Unable to get variable due to unknown error in module init. (%s)",resp);
				else 	php_error(E_WARNING, "Unable to get variable due to unknown error in %s(). (%s)", get_active_function_name(TSRMLS_C),resp);					
				break;					
		}
	}
	return value;
}

uint32_t get_variable_str(char* variable, int cmd_len, char* str, int str_len, int init) {

	char cmd[50]={0};
	char resp[300]={0};
	char* val;
	

	snprintf(cmd,sizeof(cmd),"V %s",variable);
	if(do_socket_comms(cmd, strlen(cmd), resp, 300) > 0) {
		switch(resp[0]) {
			case 'Y':
				val=&resp[2];
				if(strlen(val) < str_len-1) strcpy(str,val);
				else {
					strncpy(str,val,str_len-1);
					val=str+(str_len-1);
					*val='\0';
				}
				break;
			case 'N':
				val=&resp[2];
				if(init) php_error(E_WARNING, "Sockrpip responded with an error during module init. Error was '%s'", val);
				else 	php_error(E_WARNING, "Sockrpip responded with an error during %s(). Error was '%s'", get_active_function_name(TSRMLS_C), val);	
				break;	
			case 'S':
				val=&resp[2];
				if(init) php_error(E_WARNING, "Sockrpip socket error during module init. Error was '%s'", val);
				else 	php_error(E_WARNING, "Sockrpip socket error during %s(). Error was '%s'", get_active_function_name(TSRMLS_C), val);
				break;			
			case 'X':
				val=&resp[2];
				if(init) php_error(E_WARNING, "Sockrpip command error during module init. Error was '%s'", val);
				else 	php_error(E_WARNING, "Sockrpip command error during %s(). Error was '%s'", get_active_function_name(TSRMLS_C), val);
				break;
			default:	
				if(init) php_error(E_WARNING, "Unable to get variable due to unknown error in module init. (%s)",resp);
				else 	php_error(E_WARNING, "Unable to get variable due to unknown error in %s(). (%s)", get_active_function_name(TSRMLS_C),resp);					
				break;					
		}
	}
	return strlen(val);
}

uint32_t run_function_write(char class, char* func, int func_len, char* func_params, int func_params_len) {

	char cmd[100]={0};
	char resp[300]={0};
	char* code;
	uint32_t result=0;

	snprintf(cmd,sizeof(cmd),"%c %s %s",class,func,func_params);
	if(do_socket_comms(cmd, strlen(cmd), resp, 300) > 0) {
		switch(resp[0]) {
			case 'Y':
				result=1;
				break;
			case 'N':
				code=&resp[2];
				php_error(E_WARNING, "Sockrpip call unsuccessful during %s(). Message was '%s'", get_active_function_name(TSRMLS_C), code);	
				break;	
			case 'S':
				code=&resp[2];
				php_error(E_WARNING, "Sockrpip socket error during %s(). Error was '%s'", get_active_function_name(TSRMLS_C), code);
				break;			
			case 'X':
				code=&resp[2];
				php_error(E_WARNING, "Sockrpip command error during %s(). Error was '%s'", get_active_function_name(TSRMLS_C), code);
				break;
			default:	
				php_error(E_WARNING, "Unable to run function %s() due to unknown error. (%s)", get_active_function_name(TSRMLS_C),resp);					
				break;					
		}
	} else {
		php_error(E_WARNING, "Unable to run function %s(). No response from sockrpip.", get_active_function_name(TSRMLS_C));
	}
	return result;
}

uint32_t run_function_read(char class, char* func, int func_len, char* func_params, int func_params_len, char* resp, int resp_len) {

	char cmd[100]={0};
	char tresp[300]={0};
	char* code;
	uint32_t result=0;

	snprintf(cmd,sizeof(cmd),"%c %s %s",class,func,func_params);
	if(do_socket_comms(cmd, strlen(cmd), tresp, sizeof(tresp)) > 0) {
		switch(tresp[0]) {
			case 'Y':
				result=1;
				if(strlen(tresp) < resp_len-1) strcpy(resp,tresp);
				else {
					strncpy(resp,tresp,resp_len-1);
					code=resp+(resp_len-1);
					*code='\0';
				}
				break;
			case 'N':
				code=&tresp[2];
				php_error(E_WARNING, "Sockrpip call unsuccessful during %s(). Message was '%s'", get_active_function_name(TSRMLS_C), code);	
				break;	
			case 'S':
				code=&tresp[2];
				php_error(E_WARNING, "Sockrpip socket error during %s(). Error was '%s'", get_active_function_name(TSRMLS_C), code);
				break;			
			case 'X':
				code=&tresp[2];
				php_error(E_WARNING, "Sockrpip command error during %s(). Error was '%s'", get_active_function_name(TSRMLS_C), code);
				break;
			default:	
				php_error(E_WARNING, "Unable to run fucuntion due to unknown error in %s(). (%s)", get_active_function_name(TSRMLS_C),tresp);					
				break;					
		}
	} else {
		php_error(E_WARNING, "Unable to run function in %s(). No response from sockrpip.", get_active_function_name(TSRMLS_C));
	}
	return result;
}

uint32_t do_socket_comms(char* cmd, int cmd_len, char* response, int response_len) {
	char *socket_path = "/var/lib/sockrpip/socket";
	struct sockaddr_un addr;
	int fd,rc;
	
	if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		snprintf(response,response_len,"S Unable to open Socket");
		return strlen(response);
	}	
	
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		snprintf(response,response_len,"S Unable to connect to socket");
		return strlen(response);
	}
	
	if (write(fd, cmd, cmd_len) != cmd_len) {
		if (cmd_len == 0) {
			snprintf(response,response_len,"S Unable to write to socket");
			return strlen(response);
		}
	}
	
	rc=read(fd,response,response_len-1);
	
	if(rc > 0) {
		response[rc]='\0';
	}
	return rc;
}

uint32_t get_response_uint(void) {

	char *val;
	
	val=strtok(NULL, " ");
	if(val) {
		return atoi(val);
	} 
	
	php_error(E_WARNING, "Sockrpip command error during %s(). Unexpected parameter return count.", get_active_function_name(TSRMLS_C));
	return 0;
}
