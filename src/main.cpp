#include <stdio.h>
#include "lox.h"
#include <string>

int main()
{
	char lineBuf[255];
	while (true)
	{
		printf("> ");
		const char* line = fgets(lineBuf, 255, stdin);
		printf("%s\n", line);
		lox_run(line, strlen(line));
	}
	return 0;
}
