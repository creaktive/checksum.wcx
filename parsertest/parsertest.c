#include <stdio.h>
#include "parser.h"

int main(void)
{
	sum_node *x, *p;

	if ((x = sum_parse("parsertest.txt")) == NULL)
	{
		perror("parse");
		return -1;
	}

	for (p = x; p != NULL; p = p->next)
	{
		printf("%-4s(%-40s) <%s>\n", (p->type == MD5) ? "MD5" : "SHA1", p->checksum, p->filename);
	}

	return 0;
}
