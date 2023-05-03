/* libc includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// stdcxx includes
#include <string>


int main(int, char**)
{
	fprintf(stderr, "--- app_cpp start ---\n");

	char        in      { '\0' };
	std::string request { };
	while (in != ';') {
		read(0, &in, 1);
		request += in;
	}

	std::string result { request };
	result += "--42--;";

	write(1, result.c_str(), result.size());

	fprintf(stderr, "--- app_cpp exit ---\n");
}
