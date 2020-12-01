//#include <cmath>
#include <string.h>
#include <time.h>

int mainwrapped(char* argv);

int main(int argc, char* argv[])
{
	char* argin = "curl -v --data username=a&password=v https://somesecuresite.com --cacert cacert.pem --cookie-jar cookie.txt";

	mainwrapped(argin);

	char* argin2 = "curl -X GET --header 'Accept: application/json' https://somesecuresite.com -b cookie.txt --cacert cacert.pem";

	mainwrapped(argin2);

	while (1);

	return 0;
}
