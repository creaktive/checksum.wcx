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


#ifndef _PARSER_H
#define _PARSER_H

/* eradicate "Microsoft Visual C++ Runtime Library" */
#ifdef NO_RUNTIME

#undef isspace
#define isspace(c)	(((c>=0x9&&c<=0xd)||(c==0x20))?1:0)
#undef isxdigit
#define isxdigit(c)	(((c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F'))?1:0)
#undef isalnum
#define isalnum(c)	(((c>='0'&&c<='9')||(c>='a'&&c<='z')||(c>='A'&&c<='Z'))?1:0)
#undef tolower
#define tolower(c)	((c>='A'&&c<='Z')?(c+('a'-'A')):c)

#else

#include <ctype.h>

#endif

#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>

enum sumsize
{
	MD5		= 32,
	SHA1	= 40,
};

typedef struct sum_node_struct
{
	char *filename;
	char checksum[64];
	int type;
	struct sum_node_struct *next;
} sum_node;

sum_node *sum_parse(const char *list);
void sum_free(sum_node *head);

#endif
