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


#include "parser.h"

md5_node *md5_parse(const char *list)
{
	FILE *f;
	char buf[512];
	char *line = NULL, *md5 = NULL, *name = NULL;
	int i, j, len;
	char *p, *q;
	md5_node *head = NULL, *last = NULL, *new;

	if ((f = fopen(list, "rt")) == NULL)
		return NULL;

	memset(buf, '\0', sizeof(buf));
	while (fgets(buf, sizeof(buf), f) != NULL)
	{
		for (i = 0; i < sizeof(buf); i++)
			if (!isspace(buf[i]))
			{
				line = buf + i;
				break;
			}

		if ((line != NULL) && strlen(line))
		{
			for (i = strlen(buf) - 1; i >= 0; i--)
				if (isspace(buf[i]))
					buf[i] = '\0';
				else
					break;

			len = strlen(line);
			if (len > (32 + 1))
			{
				for (i = 0; i <= len - 32; i++)
				{
					for (j = 0; j < 32; j++)
						if (!isxdigit(line[i+j]))
							break;

					if (j == 32)
					{
						if (!isalnum(line[i+j]))
							md5 = line + i;
						break;
					}
				}

				if (md5 != NULL)
				{
					// classical md5sum scheme:
					// filename after checksum
					for (p = md5 + 32; p < line + len; p++)
						if (!isspace(*p) && (*p != '*'))
						{
							name = p;
							break;
						}

					// filename before checksum?
					if (name == NULL)
					{
						for (p = md5 - 1, q = NULL; p >= line; p--)
							if (isalnum(*p) ||
								(*p=='.') ||
								(*p=='_') ||
								(*p=='-') ||
								(*p=='/') ||
								(*p=='\\') )
							{
								if (q == NULL)
									q = p + 1;
							}
							else if (q != NULL)
								break;

						name = p + 1;
						*q = '\0';
					}

					if (name != NULL)
					{
						new = (md5_node *) malloc(sizeof(md5_node));

						len = strlen(name) + 1;
						new->filename = (char *) malloc(len);

						memset(new->filename, '\0', len);
						strncpy(new->filename, name, len);

						memset(new->checksum, '\0', sizeof(new->checksum));
						//strncpy(new->checksum, md5, 32);
						for (i = 0; i < 32; i++)
							new->checksum[i] = tolower(md5[i]);

						new->next = NULL;

						if (last == NULL)
							head = new;
						else
							last->next = new;
						last = new;
					}
				}
			}
		}

		line	= NULL;
		md5	= NULL;
		name	= NULL;
		memset(buf, '\0', sizeof(buf));
	}

	fclose(f);

	return head;
}

void md5_free(md5_node *head)
{
	md5_node *p, *q;
	for (p = head; p != NULL; p = q)
	{
		q = p->next;
		free(p->filename);
		free(p);
	}
}
