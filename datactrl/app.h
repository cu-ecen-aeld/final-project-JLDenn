
#ifndef APP_H
#define APP_H

#include <stdio.h>

#ifdef LOCAL
#define DB_ROOT			"../database/"
#else
#define DB_ROOT			"/root/database/"
#endif

#define TBSTATE_FILE	"/tmp/tbstate"


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