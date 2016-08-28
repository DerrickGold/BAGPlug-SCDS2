#include "ds2sdk.h"
#include "fb_api.h"
#include "fileops.h"
#include "interface.h"
#include "settings.h"
#include "language.h"
#include "menu_files.h"
#include "sd_card.h"
#include "draw.h"

struct _clip_board ClipBoard;


void _Convert_Size(struct _file_size * fileinfo)
{
	float currentSize = fileinfo->size;
	
	if(fileinfo->size > KB)
	{	
		fileinfo->group = 1;
		currentSize = fileinfo->size / KB;
	}
	if(fileinfo->size > MB)
	{
		fileinfo->group = 2;
		currentSize = fileinfo->size / MB;
	}
	if(fileinfo->size > GB)
	{
		fileinfo->group = 3;
		currentSize = fileinfo->size / GB;
	}
	fileinfo->size = currentSize;
	
}


void File_GetSize(const char *file, struct _file_size * fileinfo)
{
	fileinfo->size = SD_getFileSize(file);
	_Convert_Size(fileinfo);
}

void _copy_clear(struct _copy_info * copy)
{
    if(copy->Source_File)
        fclose(copy->Source_File);
        
    if(copy->Dest_File)
        fclose(copy->Dest_File);
        
    copy->Source_File = NULL;
    copy->Dest_File = NULL;
    
    copy->file_size = 0;
    copy->read_size = 0;
    copy->bytes_read = 0;
    copy->done = 0;
    copy->err = 0;
    copy->init = 0;
    copy->cut = 0;
    memset(copy->SourcePath, 0, sizeof(copy->SourcePath));
	if(copy->read_buf)
	{
		free(copy->read_buf);
		copy->read_buf = NULL;
	}
}


void _copy_init(const char *destination, const char *source, struct _copy_info * copy)
{
     _copy_clear(copy);
	copy->Source_File = fopen(source, "rb");
	if(!copy->Source_File)
		copy->err = 0;
		
	copy->Dest_File = fopen(destination, "wb");
	if(!copy->Dest_File)
		copy->err = -1;
		
	//get file size of file
	copy->read_buf = calloc(Main_Settings.copy_buf_len + 1, sizeof(u8));
	if(!copy->read_buf)
		copy->err = -2;
			
    if(copy->err == 0)
    {
        strcpy(copy->SourcePath, source);
        copy->file_size = SD_getFileSize(source);
        copy->init = 1;
    }
}


void File_Copy(const char *destination, const char *source, struct _copy_info *copy)
{
    _copy_init(destination, source, copy);
}



void File_Cut(const char *destination, const char *source, struct _copy_info * copy)
{
    _copy_init(destination, source, copy);
	copy->cut = 1;
}


/*==================================================================
Updates a file copy/cut
==================================================================*/
int File_Update(struct _copy_info * copy)
{
	if(copy->init == 0)
		return -1;

	if(copy->err)
	{
		if(copy->Dest_File)
			fclose(copy->Dest_File);
		if(copy->Source_File)
			fclose(copy->Source_File);
			
		if(copy->read_buf)
		{
			free(copy->read_buf);
			copy->read_buf = NULL;
		}		
		return -1;
	}
	//copying isn't done
	if(copy->done == 0)
	{
		copy->read_size = fread(copy->read_buf, 1, Main_Settings.copy_buf_len - 1, copy->Source_File);
		fwrite(copy->read_buf, 1, copy->read_size, copy->Dest_File);
		copy->bytes_read += copy->read_size;
		
		//all the bytes have been copied
		if(copy->bytes_read >= copy->file_size)
		{
			fclose(copy->Source_File);
			fclose(copy->Dest_File);
			copy->done = 1;
			copy->init = 0;
            
            if(copy->cut == 1)
            {
                remove(copy->SourcePath);
                copy->cut = 0;
            }
			if(copy->read_buf)
			{
				free(copy->read_buf);
				copy->read_buf = NULL;
			}
			return 1;
		}
	}
	return 0;
}



int _directory_check(const char *path)
{
	DIR* dir = opendir(path);
	if(!dir)
		return 0;
	else
	{
		closedir(dir);
		return 1;
	}
	return -1;
}

int _directory_create(const char *path)
{
	mkdir(path, 777);
	return 1;
}

int Directory_Clear(struct _dir_info * folder)
{
	memset(&folder->st, 0, sizeof(struct stat));
	folder->current_file = NULL;
	folder->dir = NULL;
    memset(folder->CurDir, 0, sizeof(folder->CurDir));
	memset(folder->DestPath, 0, sizeof(folder->DestPath));
    folder->file_name = NULL;

    folder->action = 0;
    folder->Folder_Count = 0;
    folder->File_Count = 0;
    folder->Dir_Levels = 0;
    folder->curDirPos = 0;
    
    if(folder->DirPos)
    {
        free(folder->DirPos);
        folder->DirPos = NULL;
    }
	folder->isCopying = 0;
     _copy_clear(&folder->copy);
    folder->total_bytes = 0;
	
	return 1;
}

int _directory_walk_init(const char *directory, const char *dest, int action, struct _dir_info * folder)
{
	Directory_Clear(folder);
	strcpy(folder->CurDir, directory);
    strcat(folder->CurDir, "/");
	folder->action = action;
	if(dest != NULL)
	{
		strcpy(folder->DestPath, dest);
		strcat(folder->DestPath, "/");
		if(folder->action != DIRWALK_COUNT && folder->action != DIRWALK_DEL)
		{
			if(!_directory_check(dest))
				_directory_create(dest);
		}
	}
	folder ->curDirPos = 1;
	folder -> DirPos = realloc(folder->DirPos, (folder->Dir_Levels + 1) * sizeof(int));
	memset(folder->DirPos, 0, sizeof(folder->DirPos));
	
	folder->dir = opendir(folder->CurDir);
	folder->isCopying = 0;
	return 1;
}

void _directory_delete(char *path, int action)
{
    if(action == DIRWALK_DEL || action == DIRWALK_CUT)
    {
        //strip trailing characters from folder path
        int len = strlen(path);
        while(path[len] == '/' || path[len] == '\0')
            path[len--]= '\0';
            
        rmdir(path);
    }
}

int _directory_walk(struct _dir_info * folder)
{
	if(folder->isCopying == 1)
		goto FILECOPY;
	
	folder->current_file = readdir_ex(folder->dir, &folder->st);
 
	if(folder->current_file == NULL)
	{ //End of folder has been reached
    
        //close the current directory
        closedir(folder->dir);
        //delete the folder if needed
		if(folder->Dir_Levels > 0)
             _directory_delete(folder->CurDir, folder->action);

       
       if(folder->Dir_Levels > 0)
            FB_DirBack(folder->CurDir);//go back one dir
            
		folder->Dir_Levels--;
		if(folder->Dir_Levels <= -1)
		{
            //delete the current folder if necessary
            _directory_delete(folder->CurDir, folder->action);
            
			free(folder->DirPos);
			return 1;
		}
        if(folder->action == DIRWALK_CUT || folder->action == DIRWALK_DEL)
            folder->DirPos[folder->Dir_Levels] = 0;//reset the position in the dir
        //if moving files, then set the destination directory back one
		if(folder->action == DIRWALK_COPY || folder->action == DIRWALK_CUT)
			FB_DirBack(folder->DestPath);

			
		folder->curDirPos = 0;
		folder->DirPos = realloc(folder->DirPos, (folder->Dir_Levels + 1) * sizeof(int));
					
		folder->dir = opendir(folder->CurDir);//open new dir
		if(folder->dir == NULL)
		{
			printf("folder error!\n %s\n", folder->CurDir);
			while(1);
		}
	}
	else if(folder->curDirPos > folder->DirPos[folder->Dir_Levels])
	{
		folder->file_name = folder->current_file->d_name;
		
		if(S_ISDIR(folder->st.st_mode) && strcasecmp(folder->file_name, ".") && strcasecmp(folder->file_name, ".."))//check if folder
		{
			
			//if folder we need to scan it as well
			folder->DirPos[folder->Dir_Levels] = folder->curDirPos;
			folder->Dir_Levels++;
			folder->DirPos = realloc(folder->DirPos, (folder->Dir_Levels + 1) * sizeof(int));
			folder->curDirPos = 0;
			folder->DirPos[folder->Dir_Levels] = 0;
			
			folder->Folder_Count++;
			strcat(folder->CurDir, folder->file_name);
			strcat(folder->CurDir,"/\0");
			
			closedir(folder->dir);//close old dir
			folder->dir = opendir(folder->CurDir);//open new dir
			if(folder->dir == NULL)
			{
				printf("folder error!\n %s\n", folder->CurDir);
				while(1);
			}
			if(folder->action != DIRWALK_COUNT && folder->action != DIRWALK_DEL)
			{
					//create folder in destination directory
					strcat(folder->DestPath, folder->file_name);
					if(!_directory_check(folder->DestPath))
						_directory_create(folder->DestPath);
					strcat(folder->DestPath, "/\0");
			}	
		}
		else if(!S_ISDIR(folder->st.st_mode))
		{
			//get source file
			char SourceFile[MAX_PATH];
			strcpy(SourceFile, folder->CurDir);
			strcat(SourceFile,folder->file_name);
			
			//if a file, then we need to copy it
			if(folder->action == DIRWALK_COPY || folder-> action == DIRWALK_CUT)
			{		
				//get path for destination file
				char DestinationFile[MAX_PATH];
				strcpy(DestinationFile, folder->DestPath);
				strcat(DestinationFile, folder->file_name);
		
                if(folder->action == DIRWALK_COPY)
                    File_Copy(DestinationFile, SourceFile, &folder->copy);
                else if(folder->action == DIRWALK_CUT)
                    File_Cut(DestinationFile, SourceFile, &folder->copy);
                    
				folder->isCopying = 1;
			}
            else if(folder->action == DIRWALK_DEL)
                remove(SourceFile);
            else if(folder->action == DIRWALK_COUNT)
                folder->total_bytes += SD_getFileSize(SourceFile);
		
			folder->File_Count++;
		}
	}
	folder->curDirPos++;
	
    return 0;
    
	FILECOPY:
		folder->isCopying -= File_Update(&folder->copy);
	return 0;
}
		

		
void ClipBoard_Clear(struct _clip_board * cb)
{
    memset(cb->File_Path, 0, sizeof(cb->File_Path));
    cb->Flags = 0; 
}


void ClipBoard_Copy(const char *source, struct _clip_board * cb)
{
    ClipBoard_Clear(cb);
    strcpy(cb->File_Path, source);
    strcat(cb->File_Path, "\0");
    
    SET_FLAG(cb->Flags, CB_FULL);
    
    struct stat st;
    lstat(source, &st);
   if(S_ISDIR(st.st_mode))
        SET_FLAG(cb->Flags, CB_IS_DIR);
    
    SET_FLAG(cb->Flags, CB_COPY);
}

void ClipBoard_Cut(const char *source, struct _clip_board * cb)
{
    ClipBoard_Copy(source, cb);
    RESET_FLAG(cb->Flags, CB_COPY);  
    
    SET_FLAG(cb->Flags, CB_CUT);  
}


int ClipBoard_OverwriteMsg(int type)
{
	typedef enum{
		__WINDOW_TITLE,
		__MESSAGE0,
		__MESSAGE1,
	}overwritemsgs;
	
	struct _language msgs;
	memset(&msgs, 0, sizeof(struct _language));
	if(Language_InitBuffers(&msgs, 3) != 1)
		return 0;
	
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__WINDOW_TITLE], "Clip Board", "overwrite_window_title", NULL);
	switch(type)
	{
		case 0://file
			Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__MESSAGE0], "Clip Board", "file_overwrite_msg0", NULL);
			Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__MESSAGE0], "Clip Board", "file_overwrite_msg1", NULL);
		break;
		case 1://folder
			Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__MESSAGE0], "Clip Board", "folder_overwrite_msg0", NULL);
			Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__MESSAGE0], "Clip Board", "folder_overwrite_msg1", NULL);	
		break;
	}
	
	int choice = Unicode_Notification_Window( msgs.buffer[__WINDOW_TITLE], (u16**)&msgs.buffer[__MESSAGE0], 2, 2);
	Language_CleanBuffers(&msgs);
	return choice;
}

int ClipBoard_Paste(const char *current_dir, struct _clip_board * cb)
{
    //if clip board is empty, then just exit
    if(!GET_FLAG(cb->Flags, CB_FULL))
        return -1;

    //default paste in current directory
	int rtrn = 0;
    char SourceName[MAX_PATH];
	memset(SourceName, 0, sizeof(SourceName));
    FB_GetFileFromPath(SourceName, cb->File_Path);
    
	char DestFile[MAX_PATH];
	memset(DestFile, 0, sizeof(DestFile));
	sprintf(DestFile, "%s%s", current_dir, SourceName);

    int len = strlen(DestFile);
    while(DestFile[len] == '/' || DestFile[len] == '\0')
        DestFile[len--] = '\0';
		
	//check if someone is pasting the file in the same as the source
	if(!strcasecmp(DestFile, cb->File_Path))
	{
		ClipBoard_Clear(cb);
		return -1;
	}
        
	//if a directory is copied/cut
	if(GET_FLAG(cb->Flags, CB_IS_DIR))
	{
		//if directory exists
        if(_directory_check(DestFile) == 1)
        {
			int force = ClipBoard_OverwriteMsg(1);

            if(!force)//user selected no
                return 0;
        }
        else
            _directory_create(DestFile);
            
        Render_DirGfx(down_screen_addr);
        Flip_Screen_Fun(DOWN_SCREEN, 0);
        int action = 0;
        if(GET_FLAG(cb->Flags, CB_CUT))
            action = DIRWALK_CUT;
        else  if(GET_FLAG(cb->Flags, CB_COPY))
            action = DIRWALK_COPY;
            
        FolderOperation_Display(&ClipBoard, DestFile, action);   
	}
	else
	{
		struct stat st;
		if(!lstat(DestFile, &st))
		{
			int force = ClipBoard_OverwriteMsg(1);
            if(!force)//user selected no
                return 0;
		}
        Render_DirGfx(down_screen_addr);
        Flip_Screen_Fun(DOWN_SCREEN, 0);
		FileOperation_Display(&ClipBoard, DestFile);	
	}
    ClipBoard_Clear(cb);
	return rtrn;
}

void ClibBoard_DeleteItem(const char *Name, const char *Source,struct _clip_board * cb)
{
	typedef enum{
		__WINDOW_TITLE,
		__MESSAGE0,
	}overwritemsgs;
	
	struct _language msgs;
	memset(&msgs, 0, sizeof(struct _language));
	if(Language_InitBuffers(&msgs, 2) != 1)
		return;
		
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__WINDOW_TITLE], "Clip Board", "file_action_title2", NULL);
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__MESSAGE0], "Clip Board", "delete_msg1", NULL);

    int force = Unicode_Notification_Window(msgs.buffer[__WINDOW_TITLE], (u16**)&msgs.buffer[__MESSAGE0], 1, 2);
	Language_CleanBuffers(&msgs);
    if(!force)
        return;
 
    Render_DirGfx(down_screen_addr);
    Flip_Screen_Fun(DOWN_SCREEN, 0);
    
    struct stat st;
    lstat(Source, &st);
    if(S_ISDIR(st.st_mode)) 
    {
        ClipBoard_Copy(Source, cb);
        FolderOperation_Display(cb, NULL, DIRWALK_DEL); 
    }
    else
        remove(Source); 
        
    ClipBoard_Clear(cb);
}
    


