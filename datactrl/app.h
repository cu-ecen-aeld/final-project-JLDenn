
#ifndef APP_H
#define APP_H

#include <stdio.h>

#ifdef LOCAL
#define DB_ROOT			"../database/"
#define TBSTATE_FILE	"../database/tbstate"
#else
#define DB_ROOT			"/root/database/"
#define TBSTATE_FILE	"/root/tbstate"
#endif


//#define DEBUG

#ifdef DEBUG
#define INFO(fmt, ...) \
        printf("%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#else
#define INFO(a, ...) {}
#endif

#define ERROR(fmt, ...) \
		printf("%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#endif