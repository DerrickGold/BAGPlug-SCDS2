#ifndef _FB_API_
#define _FB_API_


#ifdef __cplusplus
extern "C" {
#endif

#include "ds2sdk.h"
#include "read_txt.h"


#define utf8_to_unicode16 _FAT_utf8_to_unicode16

#define MAX_FILES_PER_DIR 1024
#define MAX_DIRS_PER_DIR 64

#define MAX_FILENAME_LEN 256
#define DIR_NAME_LEN 512
#define EXT_NAME_LEN 10
#define MAX_EXT 50

//Filebrowser Core Data
typedef enum
{
	FBFlag_ShowAllFiles = (1 << 0),
	FBFlag_HideAllFiles = (1 << 1),
	FBFlag_HideFolders = (1 << 2),
	FBFlag_HideAllFolders = (1 << 3),
	FBFlag_isNewDir = ( 1 << 4),
}FBFlags;

struct FILEINFO
{
    char *Name;
	char *CurrentDir;
};


typedef struct __fb
{
	struct _txtFile HidDirList;
	struct _txtFile HidFileList;
	struct FILEINFO *File;
    struct FILEINFO * Dir;
	int Max_Files, Max_Dirs;
	s16 File_Count, Dir_Count;
	char Current_Dir [MAX_PATH];
	char Old_Dir[MAX_PATH];
	char *Supported_Ext[MAX_EXT];
	int types;
	bool Flags;
	char Hide[6];
}FileBrowser;


extern FileBrowser * FB_Init ( int max_files, int max_dirs);
extern void FB_Free (FileBrowser *FB);

extern void FB_ReadHidDirList(const char * file, FileBrowser * _fb_);
extern void FB_ReadHidFileList(const char * file, FileBrowser * _fb_);

extern void FB_SetHiddenFolders(FileBrowser * _fb_, char * str, int on);
extern void FB_ShowAllTypes(FileBrowser * _fb_, int on);
extern void FB_HideAllFiles(FileBrowser * _fb_, int on);
extern void FB_HideAllFolders(FileBrowser * _fb_, int on);


extern void QSort_Alpha(void *array, int left, int right);
extern s16 FB_ScanDir( const char *directory, FileBrowser *_fb_, u8 Num_Types, char *exts[], int folders);
extern u8 FB_SortAlpha(FileBrowser * FB);
extern u8 FB_GetExt(const char *filename, char *ext_buf, int bufsize);
extern  int FB_FileExists(const char *path);
extern int FB_StripExt(char *input);
extern int FB_DirBack(char *path);
extern int FB_GetFilePath(char *pathbuf, const char *path);
extern int FB_GetFileFromPath(char *pathbuf, const char *path);
extern void FB_SetSupportedExt(FileBrowser * _fb_, int num, char *exts[]);
extern void FB_ClearTypes(FileBrowser * _fb_);
extern int FB_FixFilePath(char * path);
extern int FB_isNewDir(FileBrowser *_fb_);


//Filebrowser Interface data
#define MAX_ESCAPES 8
typedef enum
{
	FB_ESCAPE1 = 50,
	FB_ESCAPE2 = 55,
	FB_ESCAPE3 = 60,
	FB_ESCAPE4 = 65,
	FB_ESCAPE5 = 70,
	FB_ESCAPE6 = 75,
	FB_ESCAPE7 = 80,
	FB_ESCAPE8 = 85,
	FB_LOAD = 98,
	FB_LOAD_SKIP = 97,
}filebrowserload;

extern int escape_table[MAX_ESCAPES];

typedef struct __fbguistate
{
	int File_Selected ,OldFile, FilesPerScreen, List_Scroll, Cursor_Scroll;
	char Folder[MAX_PATH];
}UI_State;

typedef enum
{
	FBPad_Open,
	FBPad_PrevDir,
	FBPad_NextFile,
	FBPad_PrevFile,
    FBPad_NextPage,
    FBPad_PrevPage,
	FBPad_Stylus,
	FBPad_Escape,
	
	FBPad_EndInput,//end of input array
}FBInput;


typedef struct __fbgui
{
	FileBrowser * FB_Dir;
	UI_State State;
	int (*Render_FB)(struct __fbgui * gui);
	int (*FBInput[FBPad_EndInput])(void);
	int Flags;
    int Paging_Val;
}UI_FB;

extern int _fbuiDefault_NextFile(void);
extern int _fbuiDefault_PrevFile(void);
extern int _fbuiDefault_NextPage(void);
extern int _fbuiDefault_PrevPage(void);
extern int _fbuiDefault_Open(void);
extern int _fbuiDefault_PrevDir(void);
extern int _fbuiDefault_Stylus(void);
extern int _fbuiDefault_Escape(void);


extern UI_FB * FBUI_Init(int max_files, int (*renderFun)(UI_FB * gui));
extern void FBUI_Free( UI_FB * gui);
extern int FBUI_Run (UI_FB * gui, char * file_load, int types, char * file_types[]);
extern void FBUI_ShowFiles(UI_FB * gui, int num, char *file_list[]);
extern void FBUI_RegisterKeyAction(UI_FB * gui, int action, int (*input)(void));
extern void FBUI_SetDefaultInput(UI_FB * gui);
extern void FBUI_SetPageSkip(UI_FB * gui, int page_skip);
extern int FBUI_ifPagingScroll(UI_FB * gui);
extern void FBUI_SetFilesPerScreen(UI_FB * gui, int file_count);
extern void FBUI_Refresh(UI_FB * gui);
extern int FBUI_RenderCore(UI_FB * gui);

#ifdef __cplusplus
}
#endif

#endif

