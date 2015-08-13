/*
 * Project: MCP2221 HID Library
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2015 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/mcp2221-hid-library/
 */

#include <stdio.h>
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

	// Configure GPIO 0, 2 and 3 as OUTPUT LOW
	gpioConf.conf[0].gpios		= MCP2221_GPIO0 | MCP2221_GPIO2 | MCP2221_GPIO3;
	gpioConf.conf[0].mode		= MCP2221_GPIO_MODE_GPIO;
	gpioConf.conf[0].direction	= MCP2221_GPIO_DIR_OUTPUT;
	gpioConf.conf[0].value		= MCP2221_GPIO_VALUE_LOW;

	// Configure GPIO 1 as alternate function 2, which is interrupt input
	gpioConf.conf[1].gpios		= MCP2221_GPIO1;
	gpioConf.conf[1].mode		= MCP2221_GPIO_MODE_ALT2;

	// Apply config
	mcp2221_setGPIOConf(myDev, &gpioConf);
	puts("done");

	printf("Setting interrupt trigger mode and clearing interrupt... ");
	mcp2221_setInterrupt(myDev, MCP2221_INT_TRIG_FALLING, 1);
	puts("done");

	int triggerCount = 0;
	mcp2221_error res;

	while(1)
	{
		int interrupt;
		res = mcp2221_readInterrupt(myDev, &interrupt);
		if(res != MCP2221_SUCCESS)
			break;

		if(interrupt)
		{
			triggerCount++;
			printf("Interrupt triggered! %d\n", triggerCount);
			res = mcp2221_clearInterrupt(myDev);
			if(res != MCP2221_SUCCESS)
				break;
		}
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