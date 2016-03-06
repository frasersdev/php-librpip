#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <librpip.h>



static void daemonise(void);
uint32_t setup_socket(int* fd);
uint32_t do_command(int* fd);
uint32_t get_variable(int* cl, char* cmdstr);
uint32_t run_gpio_function(int* cl, char* cmdstr);
uint32_t run_i2c_function(int* cl, char* cmdstr);
uint32_t run_pwm_function(int* cl, char* cmdstr);
uint32_t run_spi_function(int* cl, char* cmdstr);

uint32_t get_param_uint(uint32_t* error);
float get_param_float(uint32_t* error);
void get_error_response(char* desc, int len);
void get_syntax_response(char* desc, int len, int val);
uint32_t feature_set;

int main(int argc, char *argv[]) {


	int fd;
	daemonise();
	if(setup_socket(&fd)) {
	
		feature_set = librpipInit(LIBRPIP_BOARD_DETECT, 0, 0);
		
		while (1) {
			do_command(&fd);
		}
	}
	return 0;
}

static void daemonise(void)
{
	pid_t pid;


	pid = fork();

	if (pid < 0)
		exit(EXIT_FAILURE);

	if (pid > 0)
		exit(EXIT_SUCCESS);

	if (setsid() < 0)
		exit(EXIT_FAILURE);

	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);


	pid = fork();

	if (pid < 0)
		exit(EXIT_FAILURE);

	if (pid > 0)
		exit(EXIT_SUCCESS);


	umask(0);

	chdir("/var/lib/sockrpip");

    	/* Close all open file descriptors */
	int x;
	for (x = sysconf(_SC_OPEN_MAX); x>0; x--) {
		close (x);
	}

	openlog("sockrpip", LOG_PID, LOG_DAEMON);
}

uint32_t setup_socket(int* fd) {

	char* socket_path = "/var/lib/sockrpip/socket";
	struct sockaddr_un addr;

	if ( (*fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		syslog (LOG_ERR, "Socket Creation Error\n");
		return 0;
	}
	
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

	unlink(socket_path);

	if (bind(*fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		syslog (LOG_ERR, "Socket Bind Error\n");
		return 0;
	}

	if (listen(*fd, 5) == -1) {
		syslog (LOG_ERR, "Socket Listen Error\n");
		return 0;
	}
	
	chmod(socket_path, S_IRWXU | S_IRWXG);
	return 1;	
}

uint32_t do_command(int* fd) {

	int cl,rc;
	char buf[255]={0};
	char msg[255]={0};

	if((cl = accept(*fd, NULL, NULL)) == -1) {
		return 0;
	}	
	rc=read(cl,buf,sizeof(buf)-1);
	
	if(rc > 2 ) {
		buf[rc]='\0';
		sprintf(msg,"Client sent request '%s'\n", buf);
		syslog (LOG_INFO, msg);
		switch(buf[0]) {
			case 'V':
				get_variable(&cl,&buf[2]);
				break;
			case 'G':
				run_gpio_function(&cl,&buf[2]);
				break;	
			case 'I':
				run_i2c_function(&cl,&buf[2]);
				break;								
			case 'P':
				run_pwm_function(&cl,&buf[2]);
				break;
			case 'S':
				run_spi_function(&cl,&buf[2]);
				break;					
			default:
				sprintf(msg,"Unknown CMD: %c\n", buf[0]);
				syslog(LOG_WARNING, msg);
		}
		close(cl);
	}
	
	else if(rc > 0) {
		buf[rc]='\0';
		sprintf(msg,"Invalid Request %s\n",buf);
		syslog(LOG_WARNING, msg);
		close(cl);
		return 0;
	}	
	
	else if(rc == 0) {
		syslog (LOG_NOTICE, "Client Dropped Connection\n");
		close(cl);
		return 0;
	}	
    
	else if(rc == -1) {
		syslog (LOG_ERR, "Socket Read Error\n");
		close(cl);
		return 0;
	}

	return 1;
}

uint32_t get_variable(int* cl, char* cmdstr) {

	char *var;
	char buf[255];
	uint32_t valid;

	valid=0;	
	
	var = strtok(cmdstr, " ");

	if(!strncmp("BoardID",var,7)) {
		valid=1;
		sprintf(buf,"Y %u\n",librpipGetBoardID());	
	}
	else if(!strncmp("BoardDesc",var,9)) {
		valid=1;
		char desc[100];
		librpipBoardGetDescription(&desc[0],sizeof(desc));
		sprintf(buf,"Y %s\n",desc);	
	} 				
	else if(!strncmp("FeatureSet",var,10)) {
		valid=1;
		sprintf(buf,"Y %u\n",feature_set);
	} 
	else if(!strncmp("Version",var,7)) {
		valid=1;
		char ver[20];
		librpipVersionStr(&ver[0],sizeof(ver));
		sprintf(buf,"Y %s\n",ver);	
	} 
	else if(!strncmp("GpioGetValidPins",var,16)) {
		valid=1;
		sprintf(buf,"Y %u\n",librpipGpioGetValidPins());	
	} 	
	
	if(!valid) {
		sprintf(buf,"X Unknown Variable %s\n",var);
	}
	
	write(*cl,buf,strlen(buf));
	return 1;	
}



uint32_t run_gpio_function(int* cl, char* cmdstr) {
	char *func;
	char buf[350];
	uint32_t valid;
	uint32_t param_error;

	valid=0;
	param_error=0;	
	
	func = strtok(cmdstr, " ");
	
	if(!strncmp("GpioConfigPinRead",func,17)) {
		valid=1;
		uint32_t pin = get_param_uint(&param_error);
		uint32_t flags;
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),1);
		} else {
			if(librpipGpioConfigPinRead(pin, &flags)) 
				sprintf(buf,"Y %u", flags);	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}	
	}	
	else if(!strncmp("GpioConfigPinWrite",func,18)) {
		valid=1;
		uint32_t pin 	= get_param_uint(&param_error);
		uint32_t flags	= get_param_uint(&param_error);
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),2);
		} else {				
			if(librpipGpioConfigPinWrite(pin, flags)) 
				sprintf(buf,"Y");	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}		
	}
	if(!strncmp("GpioPinRead",func,11)) {
		valid=1;
		uint32_t pin = get_param_uint(&param_error);
		uint32_t value;
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),1);
		} else {
			if(librpipGpioPinRead(pin, &value)) 
				sprintf(buf,"Y %u", value);	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}	
	}	
	else if(!strncmp("GpioPinWrite",func,12)) {
		valid=1;
		uint32_t pin 	= get_param_uint(&param_error);
		uint32_t value	= get_param_uint(&param_error);
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),2);
		} else {				
			if(librpipGpioConfigPinWrite(pin, value)) 
				sprintf(buf,"Y");	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}		
	}
	else if(!strncmp("GpioPinToggle",func,13)) {
		valid=1;
		uint32_t pin 	= get_param_uint(&param_error);
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),1);
		} else {				
			if(librpipGpioPinToggle(pin)) 
				sprintf(buf,"Y");	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}		
	}		
	else if(!strncmp("GpioPinPulse",func,12)) {
		valid=1;
		uint32_t pin 	= get_param_uint(&param_error);
		uint32_t length	= get_param_uint(&param_error);
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),2);
		} else {				
			if(librpipGpioPinPulse(pin, length)) 
				sprintf(buf,"Y");	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}		
	}
	if(!strncmp("GpioPinEventTest",func,16)) {
		valid=1;
		uint32_t pin = get_param_uint(&param_error);
		uint32_t event;
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),1);
		} else {
			if(librpipGpioPinEvent(pin, &event)) 
				sprintf(buf,"Y %u", event);	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}	
	}	
	else if(!strncmp("GpioPinEventWait",func,16)) {
		valid=1;
		uint32_t pin 	= get_param_uint(&param_error);
		uint32_t timeout	= get_param_uint(&param_error);
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),2);
		} else {				
			if(librpipGpioPinEventWait(pin, timeout)) 
				sprintf(buf,"Y");	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}		
	}			
	if(!valid) {
		sprintf(buf,"X Unknown GPIO Function %s", func);
	}
	
	write(*cl,buf,strlen(buf));
	return 1;
}

uint32_t run_i2c_function(int* cl, char* cmdstr) {
	char *func;
	char buf[350];
	uint32_t valid;
	uint32_t param_error;

	valid=0;
	param_error=0;	
	
	func = strtok(cmdstr, " ");
	
	if(!strncmp("I2cConfigRead",func,13)) {
		valid=1;
		uint32_t id = get_param_uint(&param_error);
		uint32_t flags;
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),1);
		} else {
			if(librpipI2cConfigRead(id, &flags)) 
				sprintf(buf,"Y %u", flags);	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}	
	}	
	else if(!strncmp("I2cConfigWrite",func,14)) {
		valid=1;
		uint32_t id 	= get_param_uint(&param_error);
		uint32_t flags	= get_param_uint(&param_error);
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),2);
		} else {				
			if(librpipI2cConfigWrite(id, flags)) 
				sprintf(buf,"Y");	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}		
	}		
	if(!valid) {
		sprintf(buf,"X Unknown I2C Function %s", func);
	}
	
	write(*cl,buf,strlen(buf));
	return 1;
}	
	

uint32_t run_pwm_function(int* cl, char* cmdstr) {
	char *func;
	char buf[350];
	uint32_t valid;
	uint32_t param_error;

	valid=0;
	param_error=0;	
	
	func = strtok(cmdstr, " ");
	
	if(!strncmp("PwmConfigRead",func,13)) {
		valid=1;
		uint32_t id = get_param_uint(&param_error);
		uint32_t pin;
		uint32_t period;
		uint32_t duty_cycle;
		uint32_t flags;
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),1);
		} else {
			if(librpipPwmConfigRead(id, &pin, &period, &duty_cycle, &flags)) 
				sprintf(buf,"Y %u %u %u %u",pin, period, duty_cycle, flags);	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}	
	}	
	else if(!strncmp("PwmConfigWrite",func,14)) {
		valid=1;
		uint32_t id 	= get_param_uint(&param_error);
		uint32_t period	= get_param_uint(&param_error);
		uint32_t duty_cycle= get_param_uint(&param_error);
		uint32_t flags	= get_param_uint(&param_error);
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),4);
		} else {				
			if(librpipPwmConfigWrite(id, period, duty_cycle, flags)) 
				sprintf(buf,"Y");	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}		
	}
	else if(!strncmp("PwmStatusRead",func,13)) {
		valid=1;
		uint32_t id	= get_param_uint(&param_error);
		uint32_t status;
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),1);
		} else {
			if(librpipPwmStatusRead(id, &status)) 
				sprintf(buf,"Y %u",status);	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}	
	}
	else if(!strncmp("PwmStatusWrite",func,14)) {
		valid=1;
		uint32_t id 	= get_param_uint(&param_error);
		uint32_t status = get_param_uint(&param_error);
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),2);
		} else {		
			if(librpipPwmStatusWrite(id,status)) 
				sprintf(buf,"Y");	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}	
	}	
	else if(!strncmp("PwmDutyPercentWrite",func,19)) {
		valid=1;
		uint32_t id	= get_param_uint(&param_error);
		float duty	= get_param_float(&param_error);

		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),2);
		} else {		
			if(librpipPwmDutyPercentWrite(id,duty)) 
				sprintf(buf,"Y");	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}
	}		
	else if(!strncmp("ServoConfigRead",func,15)) {
		valid=1;
		uint32_t id	= get_param_uint(&param_error);
		uint32_t range;
		uint32_t pmin;
		uint32_t pmax;

		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),1);
		} else {		
			if(librpipServoConfigRead(id, &range, &pmin, &pmax)) 
				sprintf(buf,"Y %u %u %u", range, pmin, pmax);	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}	
	}	
	else if(!strncmp("ServoConfigWrite",func,16)) {
		valid=1;
		uint32_t id 	= get_param_uint(&param_error);
		uint32_t range 	= get_param_uint(&param_error);;
		uint32_t pmin 	= get_param_uint(&param_error);
		uint32_t pmax	= get_param_uint(&param_error);
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),4);
		} else {		
			if(librpipServoConfigWrite(id, range, pmin, pmax)) 
				sprintf(buf,"Y");	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}		
	}	
	else if(!strncmp("ServoPositionWrite",func,18)) {
		valid=1;
		uint32_t id 	= get_param_uint(&param_error);
		float angle 	= get_param_float(&param_error);
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),2);
		} else {		
			if(librpipServoPositionWrite(id,angle)) 
				sprintf(buf,"Y");	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}		
	}

	if(!valid) {
		sprintf(buf,"X Unknown PWM Function %s", func);
	}
	
	write(*cl,buf,strlen(buf));
	return 1;	
}

uint32_t run_spi_function(int* cl, char* cmdstr) {
	char *func;
	char buf[350];
	uint32_t valid;
	uint32_t param_error;

	valid=0;
	param_error=0;	
	
	func = strtok(cmdstr, " ");

	if(!strncmp("SpiConfigRead",func,13)) {
		valid=1;
		uint32_t id = get_param_uint(&param_error);
		uint32_t cs = get_param_uint(&param_error);
		uint32_t spi_mode;
		uint32_t lsb_first;
		uint32_t bits_per_word; 
		uint32_t max_speed; 
		uint32_t spi_flags;
		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),2);
		} else {
			if(librpipSpiConfigRead(id, cs, &spi_mode, &lsb_first, &bits_per_word, &max_speed, &spi_flags)) 
				sprintf(buf,"Y %u %u %u %u %u", spi_mode, lsb_first, bits_per_word, max_speed, spi_flags);	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}	
	}	
	else if(!strncmp("SpiConfigWrite",func,14)) {
		valid=1;
		uint32_t id 		= get_param_uint(&param_error);
		uint32_t cs		= get_param_uint(&param_error);
		uint32_t spi_mode	= get_param_uint(&param_error); 
		uint32_t max_speed	= get_param_uint(&param_error); 
		uint32_t spi_flags	= get_param_uint(&param_error);
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),5);
		} else {				
			if( librpipSpiConfigWrite(id, cs, spi_mode, max_speed, spi_flags)) 
				sprintf(buf,"Y");	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}		
	}		
	if(!valid) {
		sprintf(buf,"X Unknown SPI Function %s", func);
	}
	
	write(*cl,buf,strlen(buf));
	return 1;
}

void get_error_response(char* desc, int len) {

	char errordesc[300];
	librpipErrorGetDescription(&errordesc[0], sizeof(errordesc));
	switch(librpipErrorGetSeverity()) {
		case LIBRPIP_ERROR_MSG_INFO:
			snprintf(desc, len, "N Info: 0x%x %s",librpipErrorGetCode(),errordesc);
			break;			
		case LIBRPIP_ERROR_MSG_WARNING:
			snprintf(desc, len, "N Warning: 0x%x %s",librpipErrorGetCode(),errordesc);
			break;		
		case LIBRPIP_ERROR_MSG_ERROR:
			snprintf(desc, len, "N Error: 0x%x %s",librpipErrorGetCode(),errordesc);
			break;
	}
}

void get_syntax_response(char* desc, int len, int val) {

	snprintf(desc, len, "X Parameter Count Mismatch. Expected %u parameters",val);

}

uint32_t get_param_uint(uint32_t* error) {

	char *val;
	
	val=strtok(NULL, " ");
	if(val) {
		return atoi(val);
	} 
	
	*error=1;
	return 0;
}

float get_param_float(uint32_t* error) {

	char *val;
	
	val=strtok(NULL, " ");
	if(val) {
		return atof(val);
	} 
	
	*error=1;
	return 0;
}