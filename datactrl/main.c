#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "csv.h"
#include "app.h"
#include "tools.h"




typedef struct __attribute__((packed)) {
	uint16_t id;
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





/////////////////////////////////////////////////////////////////////////////
static int printUsage(char *argv[]){
	printf("Usage: %s (scan|add|remove|update) [args unique to command type]\n", argv[0]);
	return 1;
}

//////////////////////////////////////////////////////////////////////////////
// Read input data in the form "34 6A 22 5F" into input buffer (capped at size)
//
//	No format checking is performed... 
//
//	Returns the number of bytes in input, or 0 for error/invalid input.
static int readInputBytes(char *inp, unsigned char *input, unsigned int size){
	
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


	


/////////////////////////////////////////////////////////////////////////////
static int scan(int argc, char *argv[]){
	
	tbstate_t tbstate = {0};
	char toolName[MAX_TOOL_NAME+1] = "";

	toolEvent_t te;
	
	int len = readInputBytes(argv[0], (unsigned char*)&te, sizeof(te));
	if(len != sizeof(toolEvent_t)){
		ERROR("Invalid toolEvent packet received, len = %i\n", len);
		return 1;
	}

	tbstate.id = te.tbID;
	
	tool_t *tools = toolsLoad(te.tbID);
	if(!tools)
		return 1;
	
	//Make the updates to the tools inventory
	if(te.dir != 0xFF){
		if(toolsState(tools, te.toolID, te.dir))
			ERROR("Error updating the tool %u:%u state: %s\n", te.tbID, te.toolID, te.dir ? "IN" : "OUT");
		else{
			if(toolsWrite(tools, te.tbID))
				ERROR("Error saving database file for toolbox %u\n", te.tbID);
		}
	}
	
	//Now its time to generate the write back to the scanner so it can display tool state
	tool_t *t = tools;
	while(t){
		tbstate.toolTotal++;
		
		if(!t->state){
			tbstate.toolOut++;
			strncpy(toolName, t->name, sizeof(toolName));
		}
		
		t = t->next;
	}

	toolsFree(tools);

	INFO("Toolbox ID: %u, Tools total: %u, Number out: %u. Toolname: \"%s\"\n", tbstate.id, tbstate.toolTotal, tbstate.toolOut, toolName);
	
	FILE *fp = fopen(TBSTATE_FILE, "w");
	for(int i=0;i<sizeof(tbstate_t);i++)
		fprintf(fp, "%02x", ((char*)&tbstate)[i]);
	
	for(int i=0;i<strlen(toolName);i++)
		fprintf(fp, "%02x", toolName[i]);
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
static int addTool(int argc, char *argv[]){
	
	if(argc != 3){
		ERROR("usage: - add <toolbox id> <tool id> <tool name>\n");
		return 1;
	}
	
	long tbid = atol(argv[0]);
	long toolid = atol(argv[1]);
	char *toolname = argv[2];
	
	
	tool_t *tools = toolsLoad(tbid);
	if(!tools)
		return 1;
	
	
	if(toolsAdd(tools, toolid, 0, toolname)){	//Default to out since they are added at the computer, not the toolbox
		ERROR("Error adding tool to chain\n");
		toolsFree(tools);
		return 1;
	}
		
	
	int res = toolsWrite(tools, tbid);
	toolsFree(tools);
	if(res)
		return 1;
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
static int removeTool(int argc, char *argv[]){
	
	if(argc != 2){
		ERROR("usage: - remove <toolbox id> <tool id>\n");
		return 1;
	}
	long tbid = atol(argv[0]);
	long toolid = atol(argv[1]);
	
	tool_t *tools = toolsLoad(tbid);
	if(!tools)
		return 1;

	if(toolsRemove(&tools, toolid)){
		ERROR("Error removing tool %li from inventory\n", toolid);
		toolsFree(tools);
		return 1;
	}
	
	int res = toolsWrite(tools, tbid);
	toolsFree(tools);
	if(res)
		return 1;
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
static int updateTool(int argc, char *argv[]){
	
	if(argc != 3 && argc != 4){
		ERROR("usage: - update <toolbox id> <tool id> <tool name> [OUT|IN]\n");
		return 1;
	}
	long tbid = atol(argv[0]);
	long toolid = atol(argv[1]);
	char *toolname = argv[2];
	uint8_t state = 0xFF;
	if(argc == 4){
		if(!strcmp("IN", argv[3]))
			state = 1;
		else if(!strcmp("OUT", argv[3]))
			state = 0;
		else{
			ERROR("Invalid state\n");
			return 1;
		}
	}
	
	tool_t *tools = toolsLoad(tbid);
	if(!tools)
		return 1;

	if(toolsUpdate(tools, toolid, state, toolname)){
		ERROR("Error updating tool %li inventory\n", toolid);
		toolsFree(tools);
		return 1;
	}
	
	int res = toolsWrite(tools, tbid);
	toolsFree(tools);
	if(res)
		return 1;
	
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
static int listTools(int argc, char *argv[]){
	
	if(argc != 1){
		ERROR("usage: - list <toolbox id>\n");
		return 1;
	}
	
	long tbid = atol(argv[0]);
	
	tool_t *tools = toolsLoad(tbid);
	if(!tools)
		return 1;
	
	
	printf("------ Tools in toolbox %li ------\n", tbid);
	while(tools){
		printf("%8u | %3s | %s\n", tools->id, tools->state ? "IN" : "OUT", tools->name);
		tools = tools->next;
	}
	
	return 0;	
	
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[]){
	
	if(argc < 2)
		return printUsage(argv);
	
	
	if(!strcmp("scan", argv[1])){
		if(argc != 3)
			return printUsage(argv);

		return scan(argc-2, &argv[2]);
	}
	else if(!strcmp("add", argv[1])){
		return addTool(argc-2, &argv[2]);
	}
	else if(!strcmp("remove", argv[1])){
		return removeTool(argc-2, &argv[2]);
	}
	else if(!strcmp("update", argv[1])){
		return updateTool(argc-2, &argv[2]);
	}
	else if(!strcmp("list", argv[1])){
		return listTools(argc-2, &argv[2]);
	}
	else{
		ERROR("Invalid command: %s\n", argv[1]);
		return printUsage(argv);
	}
	
	return 1;
}