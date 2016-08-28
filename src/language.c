#include "language.h"
#include "minIni.h"


void Language_CleanBuffers(struct _language * lang)
{
	if(lang->buffer == NULL)
		return;
		
	if(lang->buffer_count == 0)
		return;
		
	int i = 0;
	for(i=0; i < lang->buffer_count; i++)
	{
		if(lang->buffer[i])
		{
			free(lang->buffer[i]);
			lang->buffer[i] = NULL;
		}
	}
	
	free(lang->buffer);
	lang->buffer = NULL;
	
	lang->buffer_count = 0;
}

int Language_InitBuffers(struct _language * lang, int num_of_buffers)
{
	lang->buffer_count = num_of_buffers;
	lang->buffer = calloc(lang->buffer_count, sizeof(u16*));
	if(lang->buffer == NULL)
		return 0;

	int i = 0;
	for(i = 0; i < lang->buffer_count; i++)
	{
		lang->buffer[i] = calloc(MAX_PATH, sizeof(u16));
		if(!lang->buffer[i])
			goto CLEANUP;
	
	}
	return 1;
	
	CLEANUP:
		Language_CleanBuffers(lang);
	return 0;
}

//reserved buffers
char tempbuf[MAX_PATH];
char buf2[MAX_PATH];

inline int Language_FileReadStr(const char *filename, u16 *destbuf, const char *section, const char *key, const char *strappend)
{
	char * src = NULL;

	memset(tempbuf, 0, sizeof(tempbuf));
	ini_gets(section,key, "no lang", tempbuf, MAX_PATH, filename);
	if(strappend != NULL && strcasecmp(tempbuf, "no lang"))
	{
		memset(buf2, 0, sizeof(buf2));	
		sprintf(buf2, tempbuf, strappend);
		src = buf2;
	}
	else
		src = tempbuf;
		
	utf8_to_uni16(src, destbuf);
	return 1;
}

int Language_FileReadFloat(const char *filename, u16 *destbuf, const char *section, const char *key, float append1)
{
	char * src = NULL;
	memset(tempbuf, 0, sizeof(tempbuf));
	
	ini_gets(section,key, "no lang", tempbuf, MAX_PATH, filename);
	if(strcasecmp(tempbuf, "no lang"))
	{
		memset(buf2, 0, sizeof(buf2));	
		sprintf(buf2, tempbuf, append1);
		src = buf2;
	}
	else
		src = tempbuf;
		
	utf8_to_uni16(src, destbuf);
	return 1;
}


int Language_FileReadInt(const char *filename, u16 *destbuf, const char *section, const char *key, int append1, int append2)
{
	char * src = NULL;
	memset(tempbuf, 0, sizeof(tempbuf));
	
	ini_gets(section, key, "no lang", tempbuf, MAX_PATH, filename);
	if( strcasecmp(tempbuf, "no lang"))
	{
		memset(buf2, 0, sizeof(buf2));
		sprintf(buf2, tempbuf, append1, append2);
		src = buf2;
	}
	else
		src = tempbuf;
		
	utf8_to_uni16(src, destbuf);
	return 1;
}
