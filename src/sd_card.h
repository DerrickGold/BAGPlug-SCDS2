#ifndef _DSTWO_SD_CARD_
#define _DSTWO_SD_CARD_

#include "file_allocation_table.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void SD_getSpaceInfo(u64 *used,  u64 *total, u64 *free);
extern off_t SD_getFileSize(const char *file);
extern unsigned int SD_convertSize(u64 size, u8 *unit);

#ifdef __cplusplus
}
#endif


#endif
