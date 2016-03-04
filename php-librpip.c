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
#include "php-librpip.h"
 

// set {NULL, NULL, NULL} as the last record to mark the end of list
static zend_function_entry librpip_functions[] = {
	PHP_FE(librpip_FeatureSet, NULL)
	PHP_FE(librpip_GetBoardID, NULL)
	PHP_FE(librpip_GetBoardName, NULL)	
	PHP_FE(librpip_Version, NULL)
	PHP_FE(librpip_I2cConfigWrite, NULL)
	PHP_FE(librpip_PwmConfigWrite, NULL)
	PHP_FE(librpip_PwmStatusWrite, NULL)
	PHP_FE(librpip_PwmDutyPercentWrite, NULL)				
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

PHP_MINFO_FUNCTION(librpip)
{
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

PHP_MINIT_FUNCTION(librpip)
{
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
    	return SUCCESS;  
}

PHP_RINIT_FUNCTION(librpip)
{
	LIBRPIP_G(featureset) = get_variable_uint("FeatureSet", 10, 1);
	LIBRPIP_G(boardid) = get_variable_uint("BoardID", 7, 1);
    	return SUCCESS;    
    
}

// functions
PHP_FUNCTION(librpip_FeatureSet)
{
	RETURN_LONG(LIBRPIP_G(featureset));
}

PHP_FUNCTION(librpip_GetBoardID)
{
	RETURN_LONG(LIBRPIP_G(boardid));
}

PHP_FUNCTION(librpip_GetBoardName)
{
	char boardname[60]={0};
	get_variable_str("BoardDesc", 9, boardname, sizeof(boardname), 0);
	RETURN_STRING(boardname, 1);
}

PHP_FUNCTION(librpip_Version)
{
	char version[40]={0};
	get_variable_str("Version", 7, version, sizeof(version), 0);
	RETURN_STRING(version, 1);
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
				php_error(E_WARNING, "Unable to run fucuntion due to unknown error in %s(). (%s)", get_active_function_name(TSRMLS_C),resp);					
				break;					
		}
	}
	return result;
}

uint32_t do_socket_comms(char* cmd, int cmd_len, char* response, int response_len) {
	char *socket_path = "/tmp/librpip-socket";
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
