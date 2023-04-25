//
// Author: A. Delis ad1622@nyu.edu
//

#include <stdio.h>
#include <unistd.h>

#include "myRecordDef.h"

int main (int argc, char** argv) {
	FILE *fpb;
   	MyRecord rec;
   	long lSize;
   	int numOfrecords, i, j;
   
	// check inline parameters
   	if (argc!=2) {
      		printf("Correct syntax is: %s BinaryFile\n", argv[0]);
      		return(1); // exit if no compliance
   		}

	// open BIN file that contains records
   	fpb = fopen (argv[1],"rb");
	if (fpb==NULL) {
      		printf("Cannot open binary file\n");
      		return(1); // exit if not successful file opening
   		}

   	// check number of records stats in BIN file
   	fseek (fpb , 0 , SEEK_END);
   	lSize = ftell (fpb);
   	rewind (fpb);
	// check abobe lib calls: fseek, ftell, rewind
   	numOfrecords = (int) lSize/sizeof(rec);
	// report what is found out of examining the BIN file
   	printf("Records found in file %d \n", numOfrecords);
   	sleep(1);
   
	// for all records found in BIN file print out content in txt	
   	for (i=0; i<numOfrecords ; i++) {
		// get from BIN file the next struct/record
      		fread(&rec, sizeof(rec), 1, fpb);
      		printf("%ld %-20s %-20s ", \
			rec.custid, rec.LastName, rec.FirstName);
		for (j=0;j<NumOfCourses;j++)
			printf("%4.2f ",rec.Marks[j]);
		printf("%4.2f\n",rec.GPA);
   		}

   	fclose (fpb);
}
