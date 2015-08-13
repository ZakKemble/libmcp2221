/*
 * Project: MCP2221 HID Library
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2015 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/mcp2221-hid-library/
 */

#ifndef WIN_H_
#define WIN_H_

#ifdef _WIN32

#define NTDDI_VERSION NTDDI_WINXP
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0600
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#endif

#endif /* WIN_H_ */
