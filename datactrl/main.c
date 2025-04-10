#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "csv.h"



typedef struct __attribute__((packed)) {
	uint16_t tbID;
	uint32_t toolTotal;
	uint32_t toolOut;
	char toolName[];		//Max toolName = 16 bytes (no NULL)
}tbstate_t;
#define MAX_TOOL_NAME	16

typedef struct __attribute__((packed)) {
	uint16_t tbID;
	uint8_t dir;
	uint32_t toolID;
}toolEvent_t;

#define DB_ROOT			"../database/" 	//"/root/database/"
#define TBSTATE_FILE	"../database/tbstate" 		//"/root/tbstate"

typedef struct {
	int row;
	int col;
	tbstate_t *tbState;
	
	uint32_t toolID;
	uint8_t state;
	char name[128];
	char *toolOutName;
}parseHelp_t;

#define COL_TID		0
#define COL_STATE	1
#define COL_NAME	2

#define DEBUG

#ifdef DEBUG
#define INFO(fmt, ...) \
        printf("%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define INFO(a, ...) {}
#endif

//////////////////////////////////////////////////////////////////////////////
// Read input data in the form "34 6A 22 5F" into input buffer (capped at size)
//
//	No format checking is performed... 
//
//	Returns the number of bytes in input, or 0 for error/invalid input.
static int readInputData(char *inp, unsigned char *input, unsigned int size){
	
	if(!inp)
		return 0;
	
	int len = 0;
	while(*inp){
		unsigned int inc;
		if(sscanf(inp, "%x", &inc) != 1)
			break;

		input[len++] = inc;
		if(len == size)
			break;
		
		inp += 2;	//advance past the 2 character hex value
		
		//Advance past the space (if one exists, and we aren't at the NULL)
		while(*inp && *inp == ' ')
			inp++;
	}
	
	return len;
}

//handle a new field
static void cb1(void *field, size_t size, void *arg){
	
	parseHelp_t *ph = arg;
	//printf("Field (%i, %i): %s\n", ph->row, ph->col, (char*)field);	
	
	switch(ph->col){
	case COL_TID:
		ph->toolID = atol(field);
		break;
	case COL_STATE:
		ph->state = 0;
		if(!strcmp(field, "IN"))
			ph->state = 1;
		break;
	case COL_NAME:
		strncpy(ph->name, field, sizeof(ph->name));
		ph->name[sizeof(ph->name)-1] = '\0';
		break;
	}
	
	ph->col++;
}

//Handle end of record (or end of parsing)
static void cb2(int chr, void *arg){
	
	parseHelp_t *ph = arg;
	
	
	ph->tbState->toolTotal++;
	if(!ph->state){
		ph->tbState->toolOut++;
		strncpy(ph->toolOutName, ph->name, MAX_TOOL_NAME);
		ph->toolOutName[MAX_TOOL_NAME] = '\0'; //Ensure we have a NULL character. 
	}
	
	
	ph->col = 0;
	ph->row++;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){
	
	//Output state for our toolbox inventory
	tbstate_t state = {0};
	char toolName[MAX_TOOL_NAME+1] = "";

	unsigned char input[16];
	int len = 0;
	if(argc == 2)
		len = readInputData(argv[1], input, sizeof(input));

	if(len != sizeof(toolEvent_t)){
		INFO("Invalid toolEvent packet received, len = %i\n", len);
		len = 0;
	}
	else{
		toolEvent_t *te = (toolEvent_t*)input;
		state.tbID = te->tbID;
		char filename[64];	//We know this is big enough since we're controlling the paths
		snprintf(filename, sizeof(filename), DB_ROOT "%i", te->tbID);
		
		INFO("Toolbox filename: %s\n", filename);
		
		struct csv_parser p;
		if(csv_init(&p, CSV_APPEND_NULL)){
			printf("Error initializing csv\n");
			return 1;
		}
	
		INFO("Init succeeded\n");
	
		//Read in the correct database file
		FILE *fp = fopen(filename, "r");
		if(!fp){
			printf("Error opening %s\n", filename);
			return 1;
		}
		
		fseek(fp, 0, SEEK_END);
		long size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		
		INFO("Allocating %li bytes\n", size);
		
		char *csvmem = malloc(size);
		if(!csvmem){
			perror("malloc");
			return 1;
		}
		
		INFO("Memory allocated\n");
		
		if(size != fread(csvmem, 1, size, fp)){
			perror("fread");
			return 1;
		}
		fclose(fp);
	
		INFO("Parsing\n");
	
		//Create and initialize the parseHelp_t so we can track the tools total, out and one of the names
		parseHelp_t ph = {.row = 0, .col = 0, .tbState = &state, .toolOutName = toolName};
	
		//Now that the toolbox inventory is in memory, we'll parse the csv file
		if(size != csv_parse(&p, csvmem, size, cb1, cb2, (void*) &ph)){
			printf("Error parsing csv file %s\n", filename);
			return 1;
		}
		
		printf("Finishing\n");
		if(csv_fini(&p, cb1, cb2, (void*) &ph)){
			printf("Error during csv_fini\n");
		}
		
	}

	
	FILE *fp = fopen(TBSTATE_FILE, "w");
	for(int i=0;i<sizeof(tbstate_t);i++)
		fprintf(fp, "%02x", ((char*)&state)[i]);
	
	for(int i=0;i<strlen(toolName);i++)
		fprintf(fp, "%02x", toolName[i]);
	
	return 0;
}