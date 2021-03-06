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
 
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>


#include <librpip.h>

#include "sockrpip.h"


int main(int argc, char *argv[]) {


	int fd;
	daemonise();
	if(setup_socket(&fd)) {
	
		feature_set = librpipInit(LIBRPIP_BOARD_DETECT, 0, 0);
		
		struct sockrpip_transaction_t* st = malloc((sizeof(struct sockrpip_transaction_t)*SOCKRPIP_TRANS));
		char* buf = malloc(sizeof(char)*SOCKRPIP_BUFFER_SIZE);
		
		init_transactions(st,SOCKRPIP_TRANS);
		
		while (1) {
			do_command(&fd, st, buf, SOCKRPIP_BUFFER_SIZE);
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

	chdir(SOCKRPIP_HOME);

    	/* Close all open file descriptors */
	int x;
	for (x = sysconf(_SC_OPEN_MAX); x>0; x--) {
		close (x);
	}

	openlog("sockrpip", LOG_PID, LOG_DAEMON);
}

uint32_t setup_socket(int* fd) {

	char* socket_path = SOCKRPIP_SOCKET;
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

void init_transactions(struct sockrpip_transaction_t* st, uint16_t n) {

	uint16_t i;
	
	for(i=0;i<n;i++) { 
		strncpy(st[i].name,"",SOCKRPIP_NAME_SIZE);
		st[i].ttl=0;
		st[i].timer=0;
		st[i].configured=0;
		st[i].t=0;		
	}

}



uint32_t do_command(int* fd, struct sockrpip_transaction_t* st, char* buf, int buf_size) {

	int cl,rc;
	char msg[255]={0};

	if((cl = accept(*fd, NULL, NULL)) == -1) {
		return 0;
	}	
	rc=read(cl,buf,buf_size-1);
	
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
			case 'T':
				run_tx_function(&cl,&buf[2],st);
				break;					
			case 'U':
				run_uart_function(&cl,&buf[2]);
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

uint32_t run_tx_function(int* cl, char* cmdstr, struct sockrpip_transaction_t* st) {
	char *func;
	char buf[350];
	uint32_t valid;
	uint32_t param_error;

	valid=0;
	param_error=0;	
	
	func = strtok(cmdstr, " ");

	if(!strncmp("TransactionConfigRead",func,21)) {
		valid=1;
		uint32_t id = get_param_uint(&param_error);
		uint8_t mode;
		uint8_t bpw;
		uint8_t status; 
		uint16_t len; 


		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),1);
		} else {
			if(id < SOCKRPIP_TRANS) {
				if(st[id].configured) {
					librpipTransactionConfigRead(st[id].t, &mode, &bpw, &status, &len);
					sprintf(buf,"Y %u %u %u %u %u %u %u %s", st[id].configured, st[id].ttl, st[id].timer, mode, bpw, status, len, st[id].name);	

				} else {
					sprintf(buf,"Y 0 0 0 0 0 0 0");
				}
			} else {
				get_txerror_response(&buf[0], sizeof(buf), id);
			}
		}	
	} else if(!strncmp("TransactionCreate",func,17)) {
		valid=1;
		uint32_t id = get_param_uint(&param_error);
		uint32_t ttl = get_param_uint(&param_error);
		uint8_t mode = get_param_uint(&param_error);
		uint8_t bpw = get_param_uint(&param_error);

		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),4);
		} else {
			if(id < SOCKRPIP_TRANS) {
				if(st[id].configured) {
					librpipTransactionDestroy(st[id].t);
					get_param_str(st[id].name, SOCKRPIP_NAME_SIZE, &param_error);
					st[id].ttl = ttl;
					st[id].timer = 0;
					st[id].configured =1 ;				
					st[id].t = librpipTransactionCreate(mode, bpw);
					sprintf(buf,"Y");
				} else {
					get_param_str(st[id].name, SOCKRPIP_NAME_SIZE, &param_error);
					st[id].ttl = ttl;
					st[id].timer = 0;
					st[id].configured = 1 ;				
					st[id].t = librpipTransactionCreate(mode, bpw);
					sprintf(buf,"Y");
				}
			} else {
				get_txerror_response(&buf[0], sizeof(buf), id);
			}
		}	
	
	} else if(!strncmp("TransactionMsgAdd",func,17)) {
		valid=1;
		uint32_t id = get_param_uint(&param_error);
		uint8_t dir = get_param_uint(&param_error);
		uint16_t len = get_param_uint(&param_error);		
		char* msgdata = strtok(NULL, " ");  //point at the start of the submitted buffer

		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),3);
		} else if((dir & LIBRPIP_TX_MSG_TX) && !msgdata) {
			get_syntax_response(&buf[0], sizeof(buf),3);
		} else if((dir & LIBRPIP_TX_MSG_TX) && len >= (SOCKRPIP_BUFFER_SIZE/2)) {
			get_txbuf_response(&buf[0], sizeof(buf),SOCKRPIP_BUFFER_SIZE/2);			
		} else {
			if(id < SOCKRPIP_TRANS) {
				if(st[id].configured) {
					if(dir & LIBRPIP_TX_MSG_TX) {
						uint8_t* txbuf = malloc(sizeof(uint8_t)*(1+(SOCKRPIP_BUFFER_SIZE/2)));
						char* val = strtok(msgdata, "|");
						while(val) {
							txbuf[len] = atoi(val);
							val = strtok(NULL, "|");
						}
						if(librpipTransactionMsgAdd(st[id].t, dir, txbuf, len))
							sprintf(buf,"Y");
						else 
							get_error_response(&buf[0], sizeof(buf));
						free(txbuf);
					} else {
						if(librpipTransactionMsgAdd(st[id].t, dir, 0, len))
							sprintf(buf,"Y");
						else 
							get_error_response(&buf[0], sizeof(buf));					
					}
				} else {
					get_txadderror_response(&buf[0], sizeof(buf), id);
				}
			} else {
				get_txerror_response(&buf[0], sizeof(buf), id);
			}
		}	
	}	
	
	if(!valid) {
		sprintf(buf,"X Unknown Transaction Function %s", func);
	}
	
	write(*cl,buf,strlen(buf));
	return 1;
}

uint32_t run_uart_function(int* cl, char* cmdstr) {
	char *func;
	char buf[350];
	uint32_t valid;
	uint32_t param_error;

	valid=0;
	param_error=0;	
	
	func = strtok(cmdstr, " ");

	if(!strncmp("UartConfigRead",func,14)) {
		valid=1;
		uint32_t id = get_param_uint(&param_error);
		uint32_t baud;
		uint32_t csize;
		uint32_t parity; 
		uint32_t stop; 
		uint32_t mode;

		
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),2);
		} else {
			if(librpipUartConfigRead(id, &baud, &csize, &parity, &stop, &mode)) 
				sprintf(buf,"Y %u %u %u %u %u", baud, csize, parity, stop, mode);	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}	
	}	
	else if(!strncmp("UartConfigWrite",func,15)) {
		valid=1;
		uint32_t id 		= get_param_uint(&param_error);
		uint32_t baud		= get_param_uint(&param_error); 
		uint32_t csize		= get_param_uint(&param_error); 
		uint32_t parity		= get_param_uint(&param_error);
		uint32_t stop		= get_param_uint(&param_error); 
		uint32_t mode		= get_param_uint(&param_error);	
			
		if(param_error) {
			get_syntax_response(&buf[0], sizeof(buf),5);
		} else {				
			if( librpipUartConfigWrite(id, baud, csize, parity, stop, mode)) 
				sprintf(buf,"Y");	
			else 
				get_error_response(&buf[0], sizeof(buf));
		}		
	}	
		
	if(!valid) {
		sprintf(buf,"X Unknown UART Function %s", func);
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

void get_txerror_response(char* desc, int len, int val) {

	snprintf(desc, len, "X Invalid transaction id %u.",val);

}

void get_txadderror_response(char* desc, int len, int val) {

	snprintf(desc, len, "X Transaction id %u is not initialised.",val);

}

void get_txbuf_response(char* desc, int len, int val) {

	snprintf(desc, len, "X Message data is larger than buffer %u.",val);

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

uint32_t get_param_str(char* str, uint32_t str_len, uint32_t* error) {

	char *val;
	
	val=strtok(NULL, " ");
	if(val) {
		if(strlen(val) < str_len-1) strcpy(str,val);
		else {
			strncpy(str,val,str_len-1);
			val=str+(str_len-1);
			*val='\0';
			return strlen(str);
		} 
	}
	*error=1;
	return 0;
}