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
 
#ifndef SOCKRPIP_H
#define SOCKRPIP_H 


static void daemonise(void);
uint32_t setup_socket(int* fd);
uint32_t do_command(int* fd);
uint32_t get_variable(int* cl, char* cmdstr);
uint32_t run_gpio_function(int* cl, char* cmdstr);
uint32_t run_i2c_function(int* cl, char* cmdstr);
uint32_t run_pwm_function(int* cl, char* cmdstr);
uint32_t run_spi_function(int* cl, char* cmdstr);
uint32_t run_uart_function(int* cl, char* cmdstr);

uint32_t get_param_uint(uint32_t* error);
float get_param_float(uint32_t* error);
void get_error_response(char* desc, int len);
void get_syntax_response(char* desc, int len, int val);
uint32_t feature_set;

#endif