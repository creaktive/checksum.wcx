/****************************************************************************
	This file is part of MD5 checksum generator/checker plugin for
	Total Commander.
	Copyright (C) 2003  Stanislaw Y. Pusep

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	E-Mail:	stanis@linuxmail.org
	Site:	http://sysdlabs.hypermart.net/
****************************************************************************/


#include <stdio.h>
#include <string.h>
#include "md5.h"

int main(void)
{
	struct MD5Context md5c;
	unsigned char keybuf[16], testbuf[64];
	int i, j;
	char *p;

	static char *test[]={
		"",
		"a",
		"abc",
		"message digest",
		"abcdefghijklmnopqrstuvwxyz",
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
		"12345678901234567890123456789012345678901234567890123456789012345678901234567890",
		};

	static char *ret[]={
		"d41d8cd98f00b204e9800998ecf8427e",
		"0cc175b9c0f1b6a831c399e269772661",
		"900150983cd24fb0d6963f7d28e17f72",
		"f96b697d7cb7938d525a2f31aaf161d0",
		"c3fcd3d76192e4007dfb496cca67e13b",
		"d174ab98d277d9f5a5611c2c9f419d9f",
		"57edf4a22be3c955ac49da2e2107b67a",
		};

	for (i = 0; i < sizeof(test) / sizeof(char *); i++)
	{
		MD5Init(&md5c);
		MD5Update(&md5c, test[i], strlen(test[i]));
		MD5Final(keybuf, &md5c);

		for (j = 0, p = testbuf; j < sizeof(keybuf); j++, p += 2)
			sprintf(p, "%02x", keybuf[j]);

		printf("test %d %s!\n",
			i + 1,
			strncmp(ret[i], testbuf, sizeof(testbuf)) ? "failed" : "ok");
	}

	return 0;
}
