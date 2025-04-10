#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct __attribute__((packed)) {
	uint16_t tbID;
	uint32_t toolTotal;
	uint32_t toolOut;
	char toolName[];		//Max toolName = 16 bytes (no NULL)
}tbstate_t;
#define MAX_TOOL_NAME	16

int main(int argc, char *argv[]){
	
	//Default output
	tbstate_t state = {.tbID = 90, .toolTotal = 100, .toolOut = 3};
	
	char toolName[17] = "DEFAULT";
	int len = strlen(toolName);
	
	if(argc == 2){
		char *inp = argv[1];
//		char input[128];
		len = 0;
		while(inp){
			
			unsigned int inc;
			if(sscanf(inp, "%x", &inc) != 1)
				break;

			
//			input[len++] = inc;
			len++;
			if(len == MAX_TOOL_NAME)
				break;
			
			inp += 2;	//advance past the 2 character hex value
			
			//Advance past the space (if one exists, and we aren't at the NULL)
			while(*inp && *inp == ' ')
				inp++;
		}
		
		snprintf(toolName, sizeof(toolName), "l:%i", len);
	}
	
	for(int i=0;i<sizeof(tbstate_t);i++)
		printf("%02x", ((char*)&state)[i]);
	
	for(int i=0;i<strlen(toolName);i++)
		printf("%02x", toolName[i]);
	
	return 0;
}