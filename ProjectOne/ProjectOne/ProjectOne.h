// Constants
#define FIRST_LINE_LENGTH 18
#define DATA_LINE_LENGTH 20
#define RECORD_LENGTH 64
// Data structures
typedef struct {
	char firstLine[FIRST_LINE_LENGTH];
	char dataLines[RECORD_LENGTH][DATA_LINE_LENGTH];
} Record;
typedef struct {
	int status;
	float data;
} DataLine;
typedef struct {
	Record *record;
	int resultTime;
	unsigned long blkhdrTicks;
	DataLine dataLines[RECORD_LENGTH];
} AnalyzedRecord;