#include "Util.h"

#include <time.h>

using std::string;

string timestamp()
{
	time_t now = time(NULL);
	tm tm;
    localtime_s(&tm, &now);
	char buffer[32];
	strftime(buffer, 32, "%m-%d-%Y %H:%M:%S", &tm);
	return string(buffer);
}
