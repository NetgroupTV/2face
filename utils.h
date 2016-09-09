#include <cstdint> // include this header for uint64_t
#include <cstring>
#include <time.h>       /* time_t, struct tm, difftime, time, mktime */
#include <unistd.h>
#include "city.h"
#ifdef __SSE4_2__
#include "citycrc.h"
#endif

#ifdef _DEBUG
//#define verprintf(...)   {printf("In file %s, function %s(), line %d: ",__FILE__,__FUNCTION__,__LINE__); printf(__VA_ARGS__);}
#define verprintf(...) {printf("In file %s, function %s(), line %d: ",__FILE__,__FUNCTION__,__LINE__); printf(__VA_ARGS__);}
#else
#define verprintf(...)   {;}
#endif 


