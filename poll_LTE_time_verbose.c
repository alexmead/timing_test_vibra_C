#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#define __USE_XOPEN
#define _GNU_SOURCE
#include <time.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


#define MODEMDEVICE "/dev/ttyAT"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE; 
char const gettime[]="AT+CCLK?\r";
char const timeDelimiter='"';
char const modemOK[]="OK";

int main()
{
	int fd,res, failCount=0;
	struct termios options;
	char oldbuf[255];
	char buf[255];
	struct timeval  tvInit, tvEnd, tvLinux, tvRadio;
	struct tm tmRadio;
	gettimeofday(&tvInit, NULL);
	

	// Open serial device - O_NOCTTY: No control mode (ctrl+C etc...)
	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY ); 
	
	struct termios termios;

	tcgetattr(fd, &termios);
	termios.c_cc[VTIME] = 10; /* Set timeout of 1 seconds */
	tcsetattr(fd, TCSANOW, &termios);

	res = write(fd,gettime,sizeof(gettime));
	res = write(fd,gettime,sizeof(gettime));
	usleep(5000);
	res = read(fd,oldbuf,255);
	// Add null character at the end
	oldbuf[res]=0;
	
	for(int j=0;j<100;j++){

		// Query the modem time
		res = write(fd,gettime,sizeof(gettime));
		usleep(1000);
		// Printing the Loop-number counter for ID purposes.
		printf("Loop number: %d\n", j);
		res = read(fd,buf,255);
		buf[res] = 0;

		// Check modem time query for success.
		char* okPointer = strstr(buf,modemOK); //Check if "OK" exists
		if(!okPointer){
			printf("Failed to read time\n");
			failCount++;
			//return -1;
		}
		else
		{
			//printf(buf);

			// True if new time buffer different than old time buffer, process to get offset from
			// 		OS time.
			if(strcmp(oldbuf,buf)){

				// Find the first quotation mark in the buffer, which marks the beginning of the time mark
				char* timePointer = strchr(buf,timeDelimiter);
				if(timePointer)
				{
					// Parse the actual time string which is passed back from the AT command.
					printf("%s\n", timePointer+1);
					char * valueLeftover = strptime(timePointer+1, "%y/%m/%d,%H:%M:%S", &tmRadio);

//					// check results of time string parsing.
//					if (valueLeftover == NULL){
//						// Error parsing AT command time string
//						printf("Error with the strptime(.), NULL.\n");
//					}else{
//						// Print remaining string, should be "-32"
//						printf("Should be '-32\"', it is: %s\n", valueLeftover);
//					}

					// Convert the tm struct to timeval struct
					tvRadio.tv_sec = mktime(&tmRadio);
					tvRadio.tv_usec = 0;

//					if (1)
//					{
//						printf("tmRadio.tm_sec: %d\n", tmRadio.tm_sec);
//						printf("tmRadio.tm_min: %d\n", tmRadio.tm_min);
//						printf("tmRadio.tm_hour: %d\n", tmRadio.tm_hour);
//
//						printf("tvRadio.tv_sec: %d\n", tvRadio.tv_sec);
//						printf("tvRadio.tv_usec: %d\n", tvRadio.tv_usec);
//
//					}else{
//						printf("Error in strptime(.) \n");
//						return -1;
//					}
//
//					 tvRadio.tv_sec = mktime(&tmRadio);
//					 tvRadio.tv_usec = 0;

					// Query OS time for comparison. This is calculated with the NTPD program.
					gettimeofday(&tvLinux, NULL);

					// Print both AT derived cellular time and OS time derived through NTPD
					printf ("Linux Time = %f seconds\n",
					(double)tvLinux.tv_usec/ 1000000 +
					(double)tvLinux.tv_sec);
					printf ("Modem Time = %f seconds\n",
					(double)tvRadio.tv_usec/ 1000000 +
					(double)tvRadio.tv_sec);

					//printf("Error in millis: %d", (int)())
				}
				//return -1;

			}
			strcpy(oldbuf,buf);
		}

		
	}

	gettimeofday(&tvEnd, NULL);
	printf ("Time Elapsed = %f seconds\n",
         (double) (tvEnd.tv_usec - tvInit.tv_usec) / 1000000 +
         (double) (tvEnd.tv_sec - tvInit.tv_sec));
	printf("Number of failed AT queries: %d\n", failCount);
	return 0;
}
