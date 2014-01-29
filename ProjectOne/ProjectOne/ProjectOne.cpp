// ProjectOne.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ProjectOne.h"

// Methods
FILE *openFile(char *fileName);
Record *loadRecord(unsigned int index, FILE *fp);
AnalyzedRecord *analyzeRecord(Record *record);
// Variables
int resultTime = 0;
// Constants
unsigned int RECORD_SIZE = FIRST_LINE_LENGTH + (RECORD_LENGTH * DATA_LINE_LENGTH);


int _tmain(int argc, char *argv[])
{
	FILE *fp;
	// Require that the file name is passed as the only parameter
	if (argc < 2) {
		printf("Specify the file name as a parameter for this program to work.\n");
		return 0;
	}
	// Lets start by opening the file
	fp = openFile(argv[1]);
	// Now we have the file pointer - time to start reading data
	return 0;
}

// Opens the data file
FILE *openFile(char *fileName) {
	return fopen(fileName, "rb");
}

Record *loadRecord(unsigned int index, FILE *fp) {
	unsigned int i = 0, j = 0;
	unsigned int startingIndex = RECORD_SIZE * index; 
	// Seek the spot we want
	if (fseek(fp, startingIndex, SEEK_SET)) printf("fseek(...) failed.\n");
	// Malloc our new record
	Record *record = (Record *) malloc(sizeof(Record));
	// Read first line
	for (i = 0; i < FIRST_LINE_LENGTH; i++) (record->firstLine)[i] = (char) fgetc(fp);
	// Read all the lines there after
	for (i = 0; i < RECORD_LENGTH; i++) {
		for (j = 0; j < DATA_LINE_LENGTH; j++) {
			(record->dataLines)[i][j] = (char) fgetc(fp);
		}
	}
	// Finish up
	return record;
}

AnalyzedRecord *analyzeRecord(Record *record) {
	unsigned int i = 0;
	AnalyzedRecord *analyzedRecord = (AnalyzedRecord *) malloc(sizeof(AnalyzedRecord));
	// Scan the first line stuff
	sscanf(record->firstLine, "%d%f", &(analyzedRecord->blkhdrTicks), &(analyzedRecord->resultTime));
	// Scan the data lines
	for (i = 0; i < RECORD_LENGTH; i++) {
		sscanf(record->dataLines[i], "%d%f", &((analyzedRecord->dataLines[i]).status), &((analyzedRecord->dataLines[i]).data));
	}
	// Finish up
	return analyzedRecord;
}