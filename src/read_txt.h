#ifndef _READTXT_
#define _READTXT_

#ifdef __cplusplus
extern "C" {
#endif
#define MAX_KEYS 32

struct _txtFile
{
	char ** buffer;
	int line_count;
};

extern int TxtFile_Read(const char * filepath, struct _txtFile * txtfile);
extern void TxtFile_Clean(struct _txtFile * txtfile);
extern inline int TxtFile_cmp(const char *text, const struct _txtFile * txtfile);
extern inline int TxtFile_getLineCount(const struct _txtFile * txtfile);
extern void TxtFile_addLine(struct _txtFile *txt, const char *file);
extern inline void TxtFile_writeAddLine(const char *file, const char *text);
extern void TxtFile_removeLine(struct _txtFile *txt, int line_num);
extern void TxtFile_Combine(struct _txtFile * destTxt, struct _txtFile * srcTxt);
extern int TxtFile_Write(const char * file, struct _txtFile * txtfile, char *mode);
extern void TxtFile_ReadCombine(const char *destFile, struct _txtFile * destTxt, const char *sourceFile, struct _txtFile * srcTxt);

//ini file handling
struct _ini_file
{
	struct _txtFile textFile;
	
	int Key_Count;
	char *Keys[MAX_KEYS];
	
	int *Settings;
	char Path[MAX_PATH];
	char ReadPath[MAX_PATH];	
};

extern void ini_File_Init(struct _ini_file * ini, int key_count);
extern void ini_File_Clean(struct _ini_file * ini);
extern int ini_Read_File(const char *gamepath, struct _ini_file * ini);
extern void ini_Read_Settings(struct _ini_file * ini, const char *section);
extern int ini_Match_Key(int line_num, struct _ini_file * ini);
extern void ini_Write_File(struct _ini_file * ini);

#ifdef __cplusplus
}
#endif

#endif
