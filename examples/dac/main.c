/*
 * Project: MCP2221 HID Library
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2015 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/mcp2221-hid-library/
 */

#ifndef _WIN32
	#define _BSD_SOURCE
	#include <unistd.h>
	#define Sleep(ms) usleep(ms * 1000)
#endif

#include <stdio.h>
#include "../../libmcp2221/win/win.h"
#include "../../libmcp2221/libmcp2221.h"
#include "../../libmcp2221/hidapi.h"

int main(void)
{
	puts("Starting!");

	mcp2221_init();
	
	// Get list of MCP2221s
	printf("Looking for devices... ");
	int count = mcp2221_find(MCP2221_DEFAULT_VID, MCP2221_DEFAULT_PID, NULL, NULL, NULL);
	printf("found %d devices\n", count);

	// Open whatever device was found first
	printf("Opening device... ");
	mcp2221_t* myDev = mcp2221_open();

	if(!myDev)
	{
		mcp2221_exit();
		puts("No MCP2221s found");
		getchar();
		return 0;
	}
	puts("done");
	
	// Configure GPIOs
	printf("Configuring GPIOs... ");
	mcp2221_gpioconfset_t gpioConf = mcp2221_GPIOConfInit();

	// Check GP DESIGNATION TABLE in the datasheet for what the dedicated and alternate functions are for each GPIO pin

	// Configure GPIO 0, 1 and 3 as OUTPUT LOW
	gpioConf.conf[0].gpios		= MCP2221_GPIO0 | MCP2221_GPIO1 | MCP2221_GPIO3;
	gpioConf.conf[0].mode		= MCP2221_GPIO_MODE_GPIO;
	gpioConf.conf[0].direction	= MCP2221_GPIO_DIR_OUTPUT;
	gpioConf.conf[0].value		= MCP2221_GPIO_VALUE_LOW;

	// Configure GPIO 2 as alternate function 1, which is DAC
	gpioConf.conf[1].gpios		= MCP2221_GPIO2;
	gpioConf.conf[1].mode		= MCP2221_GPIO_MODE_ALT1;

	// Apply config
	mcp2221_setGPIOConf(myDev, &gpioConf);
	puts("done");

	mcp2221_error res;
	
	int dacValue = 0;
	int valueAdd = 1;

	puts("Outputting a triangle wave of a few Hz...");

	while(1)
	{
		res = mcp2221_setDAC(myDev, MCP2221_DAC_REF_VDD, dacValue);
		if(res != MCP2221_SUCCESS)
			break;

		dacValue += valueAdd;
		if(dacValue >= MCP2221_DAC_MAX)
			valueAdd = -1;
		else if(dacValue <= 0)
			valueAdd = 1;
	}

	switch(res)
	{
		case MCP2221_SUCCESS:
			puts("No error");
			break;
		case MCP2221_ERROR:
			puts("General error");
			break;
		case MCP2221_INVALID_ARG:
			puts("Invalid argument, probably null pointer");
			break;
		case MCP2221_ERROR_HID:
			printf("USB HID Error: %ls\n", hid_error(myDev->handle));
			break;
		default:
			printf("Unknown error %d\n", res);
			break;
	}
	
	mcp2221_exit();
	
	return 0;
}