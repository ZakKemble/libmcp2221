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

	mcp2221_error res;
	
	// Here we read the temperature from an LM73 sensor
	
	// NOTE: I2C is not complete yet

	while(1)
	{
		Sleep(500);

		uint8_t buff[2];
		mcp2221_i2c_state_t state = MCP2221_I2C_IDLE;
		
		// Stop any transfers
		mcp2221_i2cState(myDev, &state);
		if(state != MCP2221_I2C_IDLE)
			mcp2221_i2cCancel(myDev);

		// Set divider from 12MHz
		mcp2221_i2cDivider(myDev, 26);

		// Write 1 byte
		puts("Writing...");
		uint8_t tmp = 0;
		mcp2221_i2cWrite(myDev, 0x48, &tmp, 1, MCP2221_I2CRW_NORMAL);
		puts("Write complete");

		// Wait for completion
		while(1)
		{
			if(mcp2221_i2cState(myDev, &state) != MCP2221_SUCCESS)
				puts("ERROR");
			printf("State: %hhu\n", state);
			if(state == MCP2221_I2C_IDLE)
				break;
		}

		// Begin reading 2 bytes
		puts("Reading...");
		mcp2221_i2cRead(myDev, 0x48, 2, MCP2221_I2CRW_NORMAL);
		puts("Read complete");
		
		// Wait for completion
		while(1)
		{
			mcp2221_i2cState(myDev, &state);
			printf("State: %hhu\n", state);
			if(state == MCP2221_I2C_DATAREADY)
				break;
		}
		
		// Get the data
		puts("Getting...");
		mcp2221_i2cGet(myDev, buff, 2);
		puts("Get complete");

		// Show the temperature
		int16_t temperature = (buff[0]<<8) | buff[1];
		temperature >>= 7;
		printf("  Temp: %u\n", temperature);
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