#include <stdio.h>
#include "lox.h"
#include "interpreter/env.h"
#include <string>
#include <sstream>
#include <fstream>
#include <time.h>

static Value ClockFunc(Interpreter& interpreter, std::vector<Value>& args)
{
	return Value((int)time(nullptr));
}

int main(int argc, char** argv)
{
	std::shared_ptr<Environment> env = std::make_shared<Environment>();
	env->DefineFunction("time", ClockFunc, 0);

	if (argc > 1)
	{
		std::stringstream buf;
		std::ifstream file(argv[1]);
		if (!file)
		{
			printf("Failed to open %s\n", argv[1]);
			return 1;
		}
		buf << file.rdbuf();
		const std::string& contents = buf.str();
		lox_run(env, contents.c_str(), contents.size());
	}
	else
	{
		char lineBuf[255];
		while (true)
		{
			printf("> ");
			const char* line = fgets(lineBuf, 255, stdin);
            if (!line) continue;
			printf("%s\n", line);
			lox_run(env, line, strlen(line));
		}
		return 0;
	}
}
