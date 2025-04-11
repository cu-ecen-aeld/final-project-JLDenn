
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "app.h"
#include "tools.h"
#include "csv.h"

typedef struct {
	int row;
	int col;
	
	uint32_t toolID;
	uint8_t state;
	char name[128];
	
	tool_t *toolHead;
	tool_t **toolNext;
}parseHelp_t;

#define COL_TID		0
#define COL_STATE	1
#define COL_NAME	2


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
	
	
	if(!*ph->toolNext){
		*ph->toolNext = (tool_t*)malloc(sizeof(tool_t));
		if(!*ph->toolNext)
			return;
		memset(*ph->toolNext, 0, sizeof(tool_t));
	}
	
	(*ph->toolNext)->id = ph->toolID;
	(*ph->toolNext)->state = ph->state;
	strncpy((*ph->toolNext)->name, ph->name, sizeof((*ph->toolNext)->name));
	ph->toolNext = &(*ph->toolNext)->next;
	
	
	ph->col = 0;
	ph->row++;
}



/////////////////////////////////////////////////////////////////////////////
tool_t* toolsLoad(uint16_t tbid){
	
	char filename[64];
	snprintf(filename, sizeof(filename), DB_ROOT "%u", tbid);
	
	
	struct csv_parser p;
	if(csv_init(&p, CSV_APPEND_NULL)){
		ERROR("Error initializing csv\n");
		return NULL;
	}

	INFO("Init succeeded\n");

	//Read in the correct database file
	FILE *fp = fopen(filename, "r");
	if(!fp){
		ERROR("Error opening %s\n", filename);
		return NULL;
	}
	
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	INFO("Allocating %li bytes\n", size);
	
	char *csvmem = malloc(size);
	if(!csvmem){
		perror("malloc");
		fclose(fp);
		return NULL;
	}
	
	INFO("Memory allocated\n");
	
	
	long read = fread(csvmem, 1, size, fp);
	fclose(fp);
	if(size != read){
		perror("fread");
		free(csvmem);
		return NULL;
	}
	

	INFO("Parsing\n");

	//Create and initialize the parseHelp_t so we can track the tools total, out and one of the names
	parseHelp_t ph = {.row = 0, .col = 0, .state = 0};
	ph.toolNext = &ph.toolHead;

	//Now that the toolbox inventory is in memory, we'll parse the csv file
	if(size != csv_parse(&p, csvmem, size, cb1, cb2, (void*) &ph)){
		ERROR("Error parsing csv file %s\n", filename);
		return NULL;
	}
	
	INFO("Finishing\n");
	if(csv_fini(&p, cb1, cb2, (void*) &ph)){
		ERROR("Error during csv_fini\n");
	}
	
	return ph.toolHead;
}

/////////////////////////////////////////////////////////////////////////////////
int toolsAdd(tool_t *tools, uint32_t toolid, uint8_t state, char *toolname){
	

	while(tools){
		if(tools->id == toolid){
			ERROR("Tool already exists: %s\n", tools->name);
			return 1;
		}
		
		if(!tools->next){
			tools->next = (tool_t*)malloc(sizeof(tool_t));
			if(!tools->next){
				ERROR("Error allocating tool item\n");
				return 1;
			}
			
			tools->next->id = toolid;
			tools->next->state = state;
			strncpy(tools->next->name, toolname, sizeof(tools->next->name));
			return 0;
		}
		
		tools = tools->next;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////
int toolsRemove(tool_t **tools, uint32_t toolid){
	
	if(!tools)
		return 1;

	tool_t *ptr = *tools;

	//Check if the head (first tool) needs removal.
	if(ptr->id == toolid){
		tool_t *hold = ptr;
		*tools = ptr->next;
		free(hold);
		return 0;
	}

	//Search for the toolid in the rest of the chain
	tool_t *prev = ptr;
	ptr = ptr->next;
	while(ptr){
		if(ptr->id == toolid){
			prev->next = ptr->next;
			free(ptr);
			return 0;
		}
		
		prev = ptr;
		ptr = ptr->next;
	}
	return 1;
	
}

/////////////////////////////////////////////////////////////////////////////////
int toolsUpdate(tool_t *tools, uint32_t toolid, uint8_t state, char *toolname){
	
	while(tools){
		if(tools->id == toolid){
			strncpy(tools->name, toolname, sizeof(tools->name));
			if(state != 0xFF)
				tools->state = state;
			return 0;
		}

		tools = tools->next;
	}
	return 1;
}


/////////////////////////////////////////////////////////////////////////////////
int toolsState(tool_t *tools, uint32_t toolid, uint8_t state){
	while(tools){
		if(tools->id == toolid){
			if(state != 0xFF)
				tools->state = state;
			return 0;
		}

		tools = tools->next;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////
void toolsFree(tool_t *tools){

	while(tools){
		tool_t *next = tools->next;
		free(tools);
		tools = next;
	}	
}

/////////////////////////////////////////////////////////////////////////////////
int toolsWrite(tool_t *tools, uint16_t tbid){
	
	char filename[64];
	snprintf(filename, sizeof(filename), DB_ROOT "%u", tbid);
	FILE *fp = fopen(filename, "w");
	if(!fp){
		ERROR("Error opening file for output: %s\n", filename);
		return 1;
	}
	
	
	while(tools){
		char *toolname = tools->name;
		if(strchr(tools->name, '"')){
			//we must escape the '"'s
			char name[sizeof(tools->name) + 10];		//leave room for the extra quotes
			char *ptr = tools->name;
			int len = 0;
			while(*ptr && len < sizeof(name)-2){		//leave room for both " if they are needed, and the NULL
				if(*ptr == '"')
					name[len++] = '"';
				name[len++] = *ptr++;
			}
			name[len] = '\0';
			toolname = name;
		}

		fprintf(fp, "%u,%s,\"%s\"\n", tools->id, tools->state ? "IN" : "OUT", toolname);
		tools = tools->next;
	}
	
	fclose(fp);
	return 0;
}


