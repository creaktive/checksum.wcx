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


#ifndef _PARSER_H
#define _PARSER_H

#include <ctype.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>

typedef struct md5_node_struct
{
	char *filename;
	char checksum[33];
	struct md5_node_struct *next;
} md5_node;

md5_node *md5_parse(const char *list);
void md5_free(md5_node *head);

#endif
