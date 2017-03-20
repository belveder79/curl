//#include <cmath>
#include <string.h>
#include <time.h>

int main(int argc, char* argv[])
{
	char* argin = "curl -v --data username=hololens&password=xoig0uP7 https://controldev.mission-j.com/oauth2/sign_in --cacert cacert.pem --cookie-jar cookie.txt";

	mainwrapped(0, argin);

	char* argin2 = "curl -X GET --header 'Accept: application/json' https://controldev.mission-j.com/notes-api/v1/vocabulary/plants -b cookie.txt --cacert cacert.pem";

	mainwrapped(0, argin2);

	while (1);

	return 0;
}