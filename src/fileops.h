#ifndef _FILE_OPERATIONS_
#define _FILE_OPERATIONS_

#include <ds2sdk.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "fb_api.h"

#define KB 1024
#define MB 1048576
#define GB 1073741824
typedef enum
{
	SIZE_B,
	SIZE_KB,
	SIZE_MB,
	SIZE_GB,
	SIZE_END,
}FILESIZES;

struct _file_size
{
	float size;
	int group;
};


typedef enum
{
	CB_IS_DIR = (1<<0),//if selected object is a directory
	CB_FULL = (1<<1),//flag for a full clib board
	CB_CUT = (1<<2),//flag to notify a cut is in progress
    CB_COPY = (1<<3),
    
}_clip_board_flags;


struct _copy_info
{
	FILE *Source_File,
		   *Dest_File;
      
	//u8 read_buf[COPY_BUF_LEN];
	u8 * read_buf;
	int file_size,
        read_size,
        bytes_read, 
        done,
        err, 
        init,
        cut;
    char SourcePath[MAX_PATH];
	
};


struct _clip_board
{
	char File_Path[MAX_PATH];
	int Flags;
};
extern struct _clip_board ClipBoard;


struct _dir_info
{
	struct stat st;
	dirent *current_file;
	DIR* dir;
    
	char    CurDir[MAX_PATH],
               DestPath[MAX_PATH],
               *file_name;
                
	int action,
         Folder_Count,
		 File_Count,
		 Dir_Levels,
		 curDirPos,
		 *DirPos;
		 
	 int isCopying;
	 struct _copy_info copy;
     
     unsigned long long total_bytes;
};

typedef enum
{
	DIRWALK_COUNT,
	DIRWALK_COPY,
	DIRWALK_CUT,
	DIRWALK_DEL,
}_dir_walking_actions;

extern void ClipBoard_Clear(struct _clip_board * cb);
extern void ClipBoard_Copy(const char *source, struct _clip_board * cb);
extern void ClipBoard_Cut(const char *source, struct _clip_board * cb);
extern int ClipBoard_Paste(const char *current_dir, struct _clip_board * cb);
extern void ClibBoard_DeleteItem(const char *Name, const char *Source, struct _clip_board * cb);

extern int _directory_check(const char *path);
extern int _directory_create(const char *path);

extern int _directory_walk_init(const char *directory, const char *dest, int action, struct _dir_info * folder);
extern int _directory_walk(struct _dir_info * folder);
extern int Directory_Clear(struct _dir_info * folder);
extern unsigned long long _file_getSize(const char *file);
extern void File_GetSize(const char *file, struct _file_size * fileinfo);

extern void File_Copy(const char *destination, const char *source, struct _copy_info *copy);
extern void File_Cut(const char *destination, const char *source, struct _copy_info * copy);
extern int File_Update(struct _copy_info * copy);



#ifdef __cplusplus
}
#endif

#endif
