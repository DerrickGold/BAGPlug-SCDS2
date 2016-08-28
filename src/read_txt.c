#include <libBAG.h>
#include "read_txt.h"
#include "minIni.h"
#include "fb_api.h"

int TxtFile_Read(const char *filepath, struct _txtFile * txtfile)
{
	TxtFile_Clean(txtfile);
	FILE * file = fopen(filepath, "rb");
	if(file == NULL)
		return -1;

	txtfile->line_count = 0;
	
	char tempbuf[MAX_PATH];
	int count = 0;
	
	int err = 0;
	while(!feof(file))
	{
		txtfile->buffer = (char**)realloc(txtfile->buffer, (count + 1) * sizeof(char*));
		if(!txtfile->buffer)
		{
			err = -2;
			break;
		}
		
		memset(tempbuf, 0, sizeof(tempbuf));
		char * fget_test = fgets(tempbuf, MAX_PATH, file);
		
		if(fget_test != NULL && tempbuf[0] != '\n' && tempbuf[0] != '\0')//skip blank lines
		{
			//strip trailing characters
			int z = strlen(tempbuf);
			while(z > 0 && (tempbuf[z] == '\0' || tempbuf[z] == '\n' || tempbuf[z] == '\r' || tempbuf[z] == ' '))
				tempbuf[z--] = '\0';
				
			txtfile->buffer[count] = calloc(strlen(tempbuf) + 1, sizeof(char));
			if(!txtfile->buffer[count])
			{
				err = -3;
				break;
			}
				
			strcpy(txtfile->buffer[count], tempbuf);
			count++;
		}
		BAG_UpdateAudio();
	}
	txtfile->line_count = count;
	fclose(file);
	
	if(!err)
		return 1;
	
	return err;
}

void TxtFile_Clean(struct _txtFile * txtfile)
{
	if(!TxtFile_getLineCount(txtfile))
		return;
		
	int i = 0;
	for(i = 0; i < txtfile->line_count; i++)
	{
		if(txtfile->buffer[i])
		{
			free(txtfile->buffer[i]);
			txtfile->buffer[i] = NULL;
		}
	}
	if(txtfile->buffer)
	{
		free(txtfile->buffer);
		txtfile->buffer = NULL;
	}
	txtfile->line_count = 0;
}

inline int TxtFile_cmp(const char *text, const struct _txtFile * txtfile)
{
	if(!txtfile->line_count)
		return -1;
		
	int i = 0, test = 0;
	while( i < txtfile->line_count && (test = strcasecmp(text, txtfile->buffer[i++])) != 0);
	
	if(test == 0)
		return i - 1;
	
	return -1;
}

inline int TxtFile_getLineCount(const struct _txtFile * txtfile)
{
	return txtfile->line_count;
}

inline void TxtFile_writeAddLine(const char *file, const char *text)
{
	u8 isNewFile = 0;
	FILE *txtfile = fopen(file, "rb");
	if(!txtfile)
		isNewFile = 1;
	else
		fclose(txtfile);
		
	txtfile = fopen(file, "a+");
	if(!txtfile)
		return;
		
	//write file to end of list
	fseek(txtfile, 0, SEEK_END);
	if(isNewFile == 0)
		fprintf(txtfile, "\n%s", text);
	else
		fprintf(txtfile, "%s", text);
	
	fclose(txtfile);
}

inline void TxtFile_addLine(struct _txtFile *txt, const char *file)
{
	
	txt->buffer = (char**)realloc(txt->buffer, (txt->line_count + 2) * sizeof(char*));
	if(txt->buffer == NULL)
	{
		txt->line_count--;
		return;
	}
	
	txt->buffer[txt->line_count] = calloc(strlen(file) +1, sizeof(char));
	if(txt->buffer[txt->line_count] == NULL)
	{
		txt->line_count--;
		txt->buffer = (char**)realloc(txt->buffer, (txt->line_count + 1) * sizeof(char*));
		return;
	}

	strcpy(txt->buffer[txt->line_count], file);
	txt->line_count++;
}

inline void TxtFile_removeLine(struct _txtFile *txt, int line_num)
{
	int i = 0;
	for( i = line_num; i < TxtFile_getLineCount(txt) - 1; i++)
	{
		if(txt->buffer[i] && txt->buffer[i + 1])
		{
			free(txt->buffer[i]);
			txt->buffer[i] = NULL;
			txt->buffer[i] = calloc(strlen(txt->buffer[i + 1]) + 1, sizeof(char));
			strcpy(txt->buffer[i], txt->buffer[i + 1]);
		}
	}

	txt->line_count--;
	memset(txt->buffer[i], 0, sizeof(txt->buffer[i]));
	free(txt->buffer[i]);
	txt->buffer = (char**)realloc(txt->buffer, (txt->line_count + 1) * sizeof(char*));
	
}


inline int TxtFile_Write(const char *file, struct _txtFile * txtfile, char * mode)
{
	if(TxtFile_getLineCount(txtfile) == 0)
		return 0;
		
	FILE * dest = fopen(file, mode);
	if(!dest)
		return -1;
		
	int i = 0;
	for(i = 0; i < TxtFile_getLineCount(txtfile); i++)
	{
		if(txtfile->buffer[i] == NULL)
			break;
		if(i < TxtFile_getLineCount(txtfile) - 1)
			fprintf(dest, "%s\n", txtfile->buffer[i]);
		else
			fprintf(dest, "%s", txtfile->buffer[i]);
	}

	fclose(dest);
	return 1;
}

void TxtFile_Combine(struct _txtFile * destTxt, struct _txtFile * srcTxt)
{
	//get the new line count of combined text
	int final_count = destTxt->line_count + srcTxt->line_count;
	
	//realloc space
	destTxt->buffer = (char**)realloc(destTxt->buffer, (final_count) * sizeof(char*));
	if(destTxt == NULL)
		return;
			
	int i = destTxt->line_count;
	int source_line = 0;
	for( i = destTxt->line_count; i < final_count; i++)
	{
		int size = strlen(srcTxt->buffer[source_line]) + 1;
		destTxt->buffer[i] = calloc(size, sizeof(char));
		if(!destTxt->buffer[i])
			return;
			
		//memcpy(destTxt->buffer[i], srcTxt->buffer[source_line], size);
		strcpy(destTxt->buffer[i], srcTxt->buffer[source_line]);
		source_line++;
	}
	
	destTxt->line_count = final_count;
	
}

void TxtFile_ReadCombine(const char *destFile, struct _txtFile * destTxt, const char *sourceFile, struct _txtFile * srcTxt)
{
	//read destination file first
	if(TxtFile_Read(destFile, destTxt) != 1)
		return;

	//read source
	if(TxtFile_Read(sourceFile, srcTxt) != 1)
		return;

	//then combine
	TxtFile_Combine(destTxt, srcTxt);
}


//Ini files====================================================
void ini_File_Init(struct _ini_file * ini, int key_count)
{
	ini->Key_Count = key_count;
	ini->Settings = calloc(ini->Key_Count + 1, sizeof(int));
}

void ini_File_Clean(struct _ini_file * ini)
{
	TxtFile_Clean(&ini->textFile);
	
	if(ini->Settings)
	{
		free(ini->Settings);
		ini->Settings = NULL;
	}
	
	memset(ini->Path, 0, sizeof(ini->Path));
	memset(ini->ReadPath, 0, sizeof(ini->ReadPath));
}

int ini_Read_File(const char *gamepath, struct _ini_file * ini)
{
	strcpy(ini->Path, gamepath);
	int rtrn = TxtFile_Read(ini->Path, &ini->textFile); 
	strcpy(ini->ReadPath, ini->Path);
	return rtrn;
}

void ini_Read_Settings(struct _ini_file * ini, const char *section)
{
	int k = 0;
	for(k = 0; k < ini->Key_Count; k++)
		ini->Settings[k] = ini_getl(section , ini->Keys[k], 0, ini->ReadPath);
}

int ini_Match_Key(int line_num, struct _ini_file * ini)
{
	int i = 0;
	for(i = 0; i < ini->Key_Count; i++)
	{
		if(!strncasecmp(ini->textFile.buffer[line_num], ini->Keys[i], strlen(ini->Keys[i])))
			return i;	
	}
	return -1;
}

void ini_Write_File(struct _ini_file * ini)
{
	FILE * leFile = fopen(ini->Path, "wb");
	if(!leFile)
		return;
		
	int i = 0;
	for( i = 0; i < TxtFile_getLineCount(&ini->textFile); i++)
	{
		int val = ini_Match_Key(i,  ini);
		if(val > -1)//settings file detected
			fprintf(leFile, "%s=%d\n", ini->Keys[val], ini->Settings[val]);
		else
			fprintf(leFile, "%s\n", ini->textFile.buffer[i]);
	
	}
	fclose(leFile);
}



