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


#include "parser.h"

sum_node *sum_parse(const char *list)
{
	FILE *f;
	char buf[512];
	char *line, *sum, *name;
	int i, j, len, type;
	char *p, *q;
	sum_node *head = NULL, *last = NULL, *newone;

	if ((f = fopen(list, "rt")) == NULL)
		return NULL;

	memset(buf, '\0', sizeof(buf));
	while (fgets(buf, sizeof(buf), f) != NULL)
	{
		line	= NULL;
		sum		= NULL;
		name	= NULL;
		type	= -1;

		for (i = 0; i < sizeof(buf); i++)
			if (!isspace(buf[i]))
			{
				line = buf + i;
				break;
			}

		if ((line != NULL) && (len = strlen(line)))
		{
			for (i = strlen(buf) - 1; i >= 0; i--)
				if (isspace(buf[i]))
					buf[i] = '\0';
				else
					break;

			if (len > (32 + 1))
			{
				for (i = 0; i <= len - 32; i++)
				{
					for (j = 0; i+j < len; j++)
						if (!isxdigit(line[i+j]))
							break;

					if ((!i || !isalnum(line[i-1])) && !isalnum(line[i+j]))
					{
						type = j;
						if (type == MD5 || type == SHA1)
						{
							sum = line + i;
							break;
						}
					}
				}

				if (sum != NULL)
				{
					// classical checksum scheme:
					// filename after checksum
					for (p = sum + type; p < line + len; p++)
						if (!isspace(*p) && (*p != '*'))
						//if (*p!=0x20 && (*p != '*'))
						{
							name = p;
							break;
						}

					// filename before checksum?
					if ((name == NULL) || (*name == '\0'))
					{
						for (p = sum - 1, q = NULL; p >= line; p--)
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
						newone = (sum_node *) malloc(sizeof(sum_node));

						newone->type = type;

						len = strlen(name) + 1;
						newone->filename = (char *) malloc(len);

						memset(newone->filename, '\0', len);
						strncpy(newone->filename, name, len);

						memset(newone->checksum, '\0', sizeof(newone->checksum));
						for (i = 0; i < type; i++)
							newone->checksum[i] = tolower(sum[i]);

						newone->next = NULL;

						if (last == NULL)
							head = newone;
						else
							last->next = newone;
						last = newone;
					}
				}
			}
		}

		memset(buf, '\0', sizeof(buf));
	}

	fclose(f);

	return head;
}

void sum_free(sum_node *head)
{
	sum_node *p, *q;
	for (p = head; p != NULL; p = q)
	{
		q = p->next;
		free(p->filename);
		free(p);
	}
}
