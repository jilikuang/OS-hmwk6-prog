#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "file_loc.h"


#define FILE_LOC_DEBUG

#ifdef FILE_LOC_DEBUG
#define log	printf
#else
#define log(...)
#endif

/* The user space utility, file_loc, should take exactly one argument which is the path to a file or directory. 
*  It should then print out the GPS coordinates / accuracy and data age of the specified file or directory, 
*  and also format a Google Maps URL which can be pasted into a web browser to view the location.
*  Example: https://www.google.com/maps/@40.8167651,-73.96536,15z
*/

int main(int argc, char **argv)
{
	char *pathname;
	char *g_url = "https://www.google.com/maps/@";
	struct gps_location loc;

	if (argc == 2) {
		pathname = argv[1];
	} else{
		log("Enter a filepath. Try again! \n");
		return -1;
	}

	log("GPS read from file: %s \n", pathname);

	if(get_gps_location(pathname, &loc) < 0){
		log("__NR_set_gps_location failed\n");
		return -1;
	}

	log("__NR_set_gps_location success\n");

	log("latitude: %f longitude: %f accuracy: %f \n",
	loc.latitude, loc.longitude, loc.accuracy);

	log("Google Maps URL: %s%f,%f,15z \n", g_url, loc.latitude, loc.longitude);

	return 0;
}
