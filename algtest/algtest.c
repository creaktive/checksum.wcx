/****************************************************************************
	This file is part of MD5/SHA1 checksum generator/checker plugin for
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
#include "sha1.h"

int main(void)
{
	struct MD5Context md5c;
	SHA_State sha1s;
	unsigned char keybuf[20], testbuf[64];
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

	static char *md5[]={
		"d41d8cd98f00b204e9800998ecf8427e",
		"0cc175b9c0f1b6a831c399e269772661",
		"900150983cd24fb0d6963f7d28e17f72",
		"f96b697d7cb7938d525a2f31aaf161d0",
		"c3fcd3d76192e4007dfb496cca67e13b",
		"d174ab98d277d9f5a5611c2c9f419d9f",
		"57edf4a22be3c955ac49da2e2107b67a",
		};

	static char *sha1[]={
		"da39a3ee5e6b4b0d3255bfef95601890afd80709",
		"86f7e437faa5a7fce15d1ddcb9eaeaea377667b8",
		"a9993e364706816aba3e25717850c26c9cd0d89d",
		"c12252ceda8be8994d5fa0290a47231c1d16aae3",
		"32d10c7b8cf96570ca04ce37f2a19d84240d3a89",
		"761c457bf73b14d27e9e9265c46f4b4dda11f940",
		"50abf5706a150990a08b2c5ea40fa0e585554732",
		};

	printf("testing MD5...\n");
	for (i = 0; i < sizeof(test) / sizeof(char *); i++)
	{
		MD5Init(&md5c);
		MD5Update(&md5c, test[i], strlen(test[i]));
		MD5Final(keybuf, &md5c);

		for (j = 0, p = testbuf; j < 16; j++, p += 2)
			sprintf(p, "%02x", keybuf[j]);

		printf("test %d %s!\n",
			i + 1,
			strncmp(md5[i], testbuf, sizeof(testbuf)) ? "failed" : "ok");
	}

	printf("\ntesting SHA1...\n");
	for (i = 0; i < sizeof(test) / sizeof(char *); i++)
	{
		SHA_Init(&sha1s);
		SHA_Bytes(&sha1s, test[i], strlen(test[i]));
		SHA_Final(&sha1s, keybuf);

		for (j = 0, p = testbuf; j < 20; j++, p += 2)
			sprintf(p, "%02x", keybuf[j]);

		printf("test %d %s!\n",
			i + 1,
			strncmp(sha1[i], testbuf, sizeof(testbuf)) ? "failed" : "ok");
	}

	return 0;
}
