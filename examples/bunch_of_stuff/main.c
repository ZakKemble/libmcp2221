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
#include <string.h>
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
	
	// Print out info
	printf("MCP2221 device opened: %s\n", myDev->path);
	printf("  Manufacturer:  %ls\n",	myDev->usbInfo.manufacturer);
	printf("  Product:       %ls\n",	myDev->usbInfo.product);
	printf("  Serial:        %ls\n",	myDev->usbInfo.serial);
	printf("  Fact serial:   %.*s\n",	myDev->usbInfo.factorySerialLen, myDev->usbInfo.factorySerial);
	printf("  VID:           %hx\n",	myDev->usbInfo.vid);
	printf("  PID:           %hx\n",	myDev->usbInfo.pid);
	printf("  Firmware:      %c.%c\n",	myDev->usbInfo.firmware[0], myDev->usbInfo.firmware[1]);
	printf("  Hardware:      %c.%c\n",	myDev->usbInfo.hardware[0], myDev->usbInfo.hardware[1]);
	printf("  Power source:  %s\n",		myDev->usbInfo.powerSource == MCP2221_PWRSRC_SELFPOWERED ? "Self powered" : "Bus powered");
	printf("  Remote wakeup: %s\n",		myDev->usbInfo.remoteWakeup == MCP2221_WAKEUP_ENABLED ? "Enabled" : "Disabled");
	printf("  Current:       %dmA\n",	myDev->usbInfo.milliamps);
	
	puts("");
	puts("");

	// Configure GPIOs
	printf("Configuring GPIOs... ");
	mcp2221_gpioconfset_t gpioConf = mcp2221_GPIOConfInit();

	// Check GP DESIGNATION TABLE in the datasheet for what the dedicated and alternate functions are for each GPIO pin

	// Configure GPIO 0 as OUTPUT HIGH
	gpioConf.conf[0].gpios		= MCP2221_GPIO0; // Multiple GPIOs can be OR'd together to apply the same config to all
	gpioConf.conf[0].mode		= MCP2221_GPIO_MODE_GPIO;
	gpioConf.conf[0].direction	= MCP2221_GPIO_DIR_OUTPUT;
	gpioConf.conf[0].value		= MCP2221_GPIO_VALUE_HIGH;

	// Configure GPIO 1 as alternative function 3 (which is interrupt input for GPIO1)
	gpioConf.conf[1].gpios		= MCP2221_GPIO1;
	gpioConf.conf[1].mode		= MCP2221_GPIO_MODE_ALT3;

	// Configure GPIO 2 as alternate function 2 (DAC output)
	gpioConf.conf[2].gpios		= MCP2221_GPIO2;
	gpioConf.conf[2].mode		= MCP2221_GPIO_MODE_ALT2;

	// Configure GPIO 3 as as alternate function 1 (which is ADC3 for GPIO3)
	gpioConf.conf[3].gpios		= MCP2221_GPIO3;
	gpioConf.conf[3].mode		= MCP2221_GPIO_MODE_ALT1;

	// Apply config
	mcp2221_setGPIOConf(myDev, &gpioConf);
	puts("done");


	printf("Saving different GPIO configuration to flash... ");
	mcp2221_gpioconfset_t confSetSave = mcp2221_GPIOConfInit();
	
	// Set all GPIOs to OUTPUT LOW at startup
	confSetSave.conf[0].gpios		= MCP2221_GPIO0 | MCP2221_GPIO1 | MCP2221_GPIO2 | MCP2221_GPIO3;
	confSetSave.conf[0].mode		= MCP2221_GPIO_MODE_GPIO;
	confSetSave.conf[0].direction	= MCP2221_GPIO_DIR_OUTPUT;
	confSetSave.conf[0].value		= MCP2221_GPIO_VALUE_LOW;

	mcp2221_saveGPIOConf(myDev, &confSetSave);
	puts("done");

	printf("Setting clock out divider and duty cycle... ");
	mcp2221_setClockOut(myDev, MCP2221_CLKDIV_16, MCP2221_CLKDUTY_50);
	puts("done");

	printf("Setting ADC ref voltage to 1024mV... ");
	mcp2221_setADC(myDev, MCP2221_ADC_REF_1024);
	puts("done");

	printf("Setting DAC ref voltage to 2048mV and output value to 10 (should output 640mV)... ");
	mcp2221_setDAC(myDev, MCP2221_DAC_REF_2048, 10);
	puts("done");

	printf("Setting interrupt trigger mode and clearing interrupt... ");
	mcp2221_setInterrupt(myDev, MCP2221_INT_TRIG_FALLING, 1);
	puts("done");

	// Get startup values for DAC (read settings from flash)
	printf("Getting startup defaults for DAC... ");
	mcp2221_dac_ref_t ref;
	int value;
	mcp2221_loadDAC(myDev, &ref, &value);
	puts("done");
	
	int refVoltage;
	switch(ref)
	{
		case MCP2221_DAC_REF_4096:
			refVoltage = 4096;
			break;
		case MCP2221_DAC_REF_2048:
			refVoltage = 2048;
			break;
		case MCP2221_DAC_REF_1024:
			refVoltage = 1024;
			break;
		case MCP2221_DAC_REF_OFF:
			refVoltage = 0;
			break;
		case MCP2221_DAC_REF_VDD:
			refVoltage = 5000; // We'll just assume VDD is 5V
			break;
		default:
			refVoltage = -1;
			break;
	}

	if(refVoltage == -1)
		puts("Unknown DAC startup ref voltage");
	else if(refVoltage == 0)
		puts("DAC startup ref voltage is off");
	else
	{
		printf("DAC startup ref: %dmV\n", refVoltage);
		printf("DAC startup value: %d\n", value);
		printf("DAC startup resulting output voltage: %dmV\n", (int)(((float)refVoltage / (MCP2221_DAC_MAX + 1)) * value));
	}


	if(mcp2221_isConnected(myDev) != MCP2221_SUCCESS)
		puts("Device is not connected!");
	else
	{
		puts("Device is OK");
		
		mcp2221_error res;
	
		while(1)
		{
			Sleep(200);
			puts("~~~~~~~~~~~~~");

			// Get GPIO0 to LOW
			res = mcp2221_setGPIO(myDev, MCP2221_GPIO0, MCP2221_GPIO_VALUE_LOW);
			if(res != MCP2221_SUCCESS)
				break;

			// Read GPIO input values
			mcp2221_gpio_value_t values[MCP2221_GPIO_COUNT];
			res = mcp2221_readGPIO(myDev, values);
			if(res != MCP2221_SUCCESS)
				break;

			for(int i=0;i<MCP2221_GPIO_COUNT;i++)
			{
				char str[8];
				switch(values[i])
				{
					case MCP2221_GPIO_VALUE_HIGH:
						strcpy(str, "HIGH");
						break;
					case MCP2221_GPIO_VALUE_LOW:
						strcpy(str, "LOW");
						break;
					case MCP2221_GPIO_VALUE_INVALID:
						strcpy(str, "INVALID");
						break;
					default:
						strcpy(str, "UNKNOWN");
						break;
				}
				printf("GPIO %d: %s\n", i, str);
			}

			// Read I2C pin values
			mcp2221_i2cpins_t i2cPins;
			res = mcp2221_i2cReadPins(myDev, &i2cPins);
			if(res != MCP2221_SUCCESS)
				break;

			printf("SDA: %hhu\n", i2cPins.SDA);
			printf("SCL: %hhu\n", i2cPins.SCL);

			// Read ADC values
			int adc[MCP2221_ADC_COUNT];
			res = mcp2221_readADC(myDev, adc);
			if(res != MCP2221_SUCCESS)
				break;

			for(int i=0;i<MCP2221_ADC_COUNT;i++)
				printf("ADC %d: %d\n", i, adc[i]);

			// Read interrupt
			int interrupt;
			res = mcp2221_readInterrupt(myDev, &interrupt);
			if(res != MCP2221_SUCCESS)
				break;

			if(interrupt)
			{
				puts("Interrupt triggered!");
				res = mcp2221_clearInterrupt(myDev);
				if(res != MCP2221_SUCCESS)
					break;
			}
			
			Sleep(200);
			
			// Set GPIO0 to HIGH
			res = mcp2221_setGPIO(myDev, MCP2221_GPIO0, MCP2221_GPIO_VALUE_HIGH);
			if(res != MCP2221_SUCCESS)
				break;
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
	}
	
	mcp2221_exit();
	
	return 0;
}