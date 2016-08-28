#ifndef _LANGUAGES_
#define _LANGUAGES_

#include <ds2sdk.h>

struct _language
{
	u16 **buffer;
	int buffer_count;
};

extern void Language_CleanBuffers(struct _language * lang);
extern int Language_InitBuffers(struct _language * lang, int num_of_buffers);
extern int Language_FileReadStr(const char *filename, u16 * destbuf, const char *section, const char *key, const char *strappend);
extern int Language_FileReadInt(const char *filename, u16 * destbuf, const char *section, const char *key, int append1, int append2);
extern int Language_FileReadFloat(const char *filename, u16 * destbuf, const char *section, const char *key, float append1);

#ifdef __cplusplus
}
#endif

#endif
