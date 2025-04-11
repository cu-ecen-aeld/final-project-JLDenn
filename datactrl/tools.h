#ifndef TOOLS_H
#define TOOLS_H

#define TOOL_STATE_OUT		0
#define TOOL_STATE_IN		1

typedef struct _tool{
	uint32_t id;
	uint8_t state;
	char name[128];	//null terminated tool name
	
	struct _tool *next;
}tool_t;


tool_t* toolsLoad(uint16_t tbid);
void toolsFree(tool_t *tools);

int toolsAdd(tool_t *tools, uint32_t toolid, uint8_t state, char *toolname);

int toolsWrite(tool_t *tools, uint16_t tbid);

int toolsRemove(tool_t **tools, uint32_t toolid);
int toolsUpdate(tool_t *tools, uint32_t toolid, uint8_t state, char *toolname);

int toolsState(tool_t *tools, uint32_t toolid, uint8_t state);

#endif