#include <libBAG.h>
#include "fb_api.h"
#include "read_txt.h"


int escape_table[MAX_ESCAPES]={FB_ESCAPE1, FB_ESCAPE2, FB_ESCAPE3, FB_ESCAPE4, FB_ESCAPE5, FB_ESCAPE6, FB_ESCAPE7, FB_ESCAPE8};
/*===============================================================
Filebrowser Core
===============================================================*/

//Hidden folder list
void FB_ReadHidDirList(const char * file, FileBrowser * _fb_)
{
	TxtFile_Read(file, &_fb_->HidDirList);
}	


void FB_CleanHidDirList(FileBrowser * _fb_)
{
	TxtFile_Clean(&_fb_->HidDirList);
}


inline int FB_isFolderHidden(const char * folder, FileBrowser * _fb_)
{
	if(TxtFile_getLineCount(&_fb_->HidDirList) == 0)
		return 0;
		
	char tempbuf[MAX_PATH];
	memset(tempbuf, 0, sizeof(tempbuf));
	sprintf(tempbuf, "%s%s", _fb_->Current_Dir, folder);
	return (TxtFile_cmp(tempbuf,&_fb_->HidDirList) == -1) ? 0 : 1;
}



//hidden file list
void FB_ReadHidFileList(const char * file, FileBrowser * _fb_)
{
	TxtFile_Read(file, &_fb_->HidFileList);
}	


void FB_CleanHidFileList(FileBrowser * _fb_)
{
	TxtFile_Clean(&_fb_->HidFileList);
}


inline int FB_isFileHidden(const char *file, FileBrowser * _fb_)
{
	if(TxtFile_getLineCount(&_fb_->HidFileList) == 0)
		return 0;
		
	char tempbuf[MAX_PATH];
	memset(tempbuf, 0, sizeof(tempbuf));
	sprintf(tempbuf, "%s%s", _fb_->Current_Dir, file);
	return (TxtFile_cmp(tempbuf,&_fb_->HidFileList) == -1) ? 0: 1;
}



void FB_Reset(FileBrowser * _fb_)
{
	if(_fb_->File)
    {
        int i = 0;
        for(i = 0; i < _fb_->File_Count; i++)
        {
            if(_fb_->File[i].Name)
            {
                free(_fb_->File[i].Name);
                _fb_->File[i].Name = NULL;
            }
        }
		free(_fb_->File);	
        _fb_->File = NULL;
    }

   
	if(_fb_->Dir)
    {
        int i = 0;
        for(i = 0; i < _fb_->Dir_Count; i++)
        {
            if(_fb_->Dir[i].Name)
            {
                free(_fb_->Dir[i].Name);
                _fb_->Dir[i].Name = NULL;
            }
        }
		free(_fb_->Dir);	
        _fb_->Dir = NULL;
    }
	memset(_fb_->Current_Dir, 0, sizeof(_fb_->Current_Dir));
    _fb_->Dir_Count = 0;
    _fb_->File_Count = 0;
}
    
inline FileBrowser * FB_Init ( int max_files, int max_dirs)
{
	FileBrowser *_fb_= NULL;
	_fb_ = (FileBrowser *)calloc(sizeof(FileBrowser),sizeof(u8));	
	if(!_fb_)
		goto err;
	
	_fb_->Max_Files = max_files;
    _fb_->Max_Dirs = max_dirs;
    
	_fb_->Dir = calloc(sizeof(struct FILEINFO), _fb_->Max_Dirs);
	_fb_->File = calloc(sizeof(struct FILEINFO), _fb_->Max_Files);
	
    _fb_->Dir_Count = 0;
    _fb_->File_Count = 0;
	
	return _fb_;
	
	err:
		return NULL;
}

void FB_Free (FileBrowser *_fb_)
{
	FB_Reset( _fb_);
	FB_ClearTypes( _fb_);
	FB_CleanHidDirList(_fb_);
	FB_CleanHidFileList( _fb_);
	if(_fb_)
		free((FileBrowser*)_fb_);
	
	_fb_ = NULL;
}



void FB_SetHiddenFolders(FileBrowser * _fb_, char * str, int on)
{
	if(on)
		SET_FLAG(_fb_->Flags, FBFlag_HideFolders);
	else
		RESET_FLAG(_fb_->Flags, FBFlag_HideFolders);
		
	memset(_fb_->Hide, 0, sizeof(_fb_->Hide));
	if(str != NULL)
		strcpy(_fb_->Hide, str);
}

void FB_HideAllFolders(FileBrowser * _fb_, int on)
{
	RESET_FLAG(_fb_->Flags, FBFlag_HideAllFolders);
	if(on)
		SET_FLAG(_fb_->Flags, FBFlag_HideAllFolders);
}

void FB_ShowAllTypes(FileBrowser * _fb_, int on)
{
	if(on)
		SET_FLAG(_fb_->Flags, FBFlag_ShowAllFiles);
	else
		RESET_FLAG(_fb_->Flags, FBFlag_ShowAllFiles);
}



void FB_HideAllFiles(FileBrowser * _fb_, int on)
{
	if(on)
		SET_FLAG(_fb_->Flags, FBFlag_HideAllFiles);
	else
		RESET_FLAG(_fb_->Flags, FBFlag_HideAllFiles);
}



void FB_ClearTypes(FileBrowser * _fb_)
{
	int i = 0;
	//clear previous supported files
	for(i = 0; i < MAX_EXT; i++)
	{
		if(_fb_-> Supported_Ext[i])
		{
			memset(_fb_-> Supported_Ext[i], 0, sizeof(_fb_-> Supported_Ext[i]));
			free(_fb_-> Supported_Ext[i]);
			_fb_-> Supported_Ext[i] = NULL;
		}
	}
}



void FB_SetSupportedExt(FileBrowser * _fb_, int num, char *exts[])
{
	FB_ClearTypes(_fb_);
	int i = 0;
	//copy new list of files
	for(i = 0; i < num; i++)
	{
		_fb_-> Supported_Ext[i] = calloc(sizeof(exts[i]), sizeof(char));
		strcpy(_fb_-> Supported_Ext[i], exts[i]);
	}
	_fb_->types = num;
}


//Converts upper case characters to lower case
inline char FB_GetSmall(char letter)
{
	if((letter >= 'A')&&(letter <= 'Z')) 
		return (letter-'A'+'a');
	
   return letter;
}   

inline u8 FB_GetExt(const char *filename, char * ext_buf, int bufsize)
{
	// go to end of name
	int len = strlen(filename);
	int i = len;
	while( filename[i] != '.' && i > 0)
		i--;
		
	int j = i + 1;
	i = j;
	do 
	{
        if( (i - j) >= bufsize)
			return 0;		
        ext_buf[ i - j ] = FB_GetSmall((u8)filename[ i ]);
	}while( i++ < len);
	
	return 1;
}

inline int FB_StripExt(char *input)
{
	int len = strlen(input);
	int count = 0;
	while(input[len] != '.' && len > 0)
	{
		input[len--] = '\0';
		count++;
	}
		
	input[len] = '\0';
	return count;
}

//go back one directory
inline int FB_DirBack(char *path)
{
	int len = strlen(path);
	while(path[len] == '\0' || path[len] == '/')
		path[len--] = '\0';
	int i = len;//subtract the \0 and the first /	
	while(path[i] != '/' && i > 0)
		path[i--]='\0';
	strcat(path, "/");
	return 1;
}


inline int FB_FixFilePath(char * path)
{
    int len = strlen(path);

    while((path[len] == '/' || path[len] == '\n' || path[len] == '\0' || path[len] == '\r') && len >= 0)
        path[len--] = '\0';
        
    //check if path is a directory
    FILE *test = fopen(path, "rb");
    if(!test)
        strcat(path, "/\0");
    else
        fclose(test);
		
	return 1;
}

inline int FB_FileExists(const char *path)
{
		struct stat st;
		return (lstat(path, &st) < 0) ? 0 : 1;
}

inline int FB_GetFilePath(char *pathbuf, const char *path)
{
	int test=0, isfile = 1;
	s16 length=strlen(path);
    
    FILE * testfile = fopen(path, "rb");
    if(!testfile)
        isfile = 0;
    else
        fclose(testfile);
        
	if(length>0)
	{
		test=1;
        if(isfile)
        {
            while(path[length]!='/')length--;
            strncpy(pathbuf,path, length);
        }
        else
        {
            strncpy(pathbuf,path, length);
            if(pathbuf[length]!= '/' || pathbuf[length - 1] != '/')
                strcat(pathbuf, "/");
        }
	}
	return test;
}

inline int FB_GetFileFromPath(char *pathbuf, const char *path)
{
	int test=0, size = 0;
	s16 length= size = strlen(path);
	if(length>=0)
	{
		test=1;
		while(path[length]!='/'  && length >= 0)length--;
		if(length >= 0)
			strncpy(pathbuf,&path[length + 1], size - length + 1);
		else
			test = 0;
	}
	return test;
}

int FB_ifNewDir(FileBrowser *_fb_, const char *curPath)
{
	if(strcasecmp(_fb_->Old_Dir, curPath))
	{
		strcpy(_fb_->Old_Dir, curPath);
		strcpy(_fb_->Current_Dir, curPath);
		
		SET_FLAG(_fb_->Flags, FBFlag_isNewDir);
		return 1;
	}
	RESET_FLAG(_fb_->Flags, FBFlag_isNewDir);
	return 0;
}

int FB_UpdateNewDir(FileBrowser *_fb_)
{
	/*should be updated at the end of every loop*/
	RESET_FLAG(_fb_->Flags, FBFlag_isNewDir);
	return 0;
}

int FB_isNewDir(FileBrowser *_fb_)
{
	return GET_FLAG(_fb_->Flags, FBFlag_isNewDir);
}

inline u8 ScanExt(char *exts[], char *filename, int num_exts)
{
	if(!num_exts)
		return 1;
		
	char Ext_Buf[EXT_NAME_LEN];
	memset(Ext_Buf, 0, sizeof(Ext_Buf));
	if(!FB_GetExt(filename, Ext_Buf, EXT_NAME_LEN))
		return 0;

	int i = 0, test = 0;
	while((test = strcasecmp(exts[i], Ext_Buf)) != 0 && (++i) < num_exts);
	if(i < num_exts)
		return 1;
		
	return 0;
}

inline u8 checkHiddenDirChars(char *folder)
{
	int len = strlen(folder);
	//check for . in beginning
	if(folder[0] == '.')
		return 1;
	//or ~ at the end
	if(folder[len - 1]  == '~')
		return 1;

	return 0;
}

inline s8 _fb_addDir(FileBrowser *_fb_, char *filename)
{
	_fb_->Dir = realloc(_fb_->Dir, sizeof(struct FILEINFO) * (_fb_->Dir_Count + 1));
	if(_fb_->Dir == NULL)
		return 0;
	
	_fb_->Dir[_fb_->Dir_Count].Name = calloc(strlen(filename)+1, sizeof(char));
	if(_fb_->Dir[_fb_->Dir_Count].Name == NULL)
		return -1;
	
	strcpy(_fb_->Dir[_fb_->Dir_Count].Name, filename);	
	_fb_->Dir_Count++;
	return 1;
}

inline s8 _fb_addFile(FileBrowser *_fb_, char *filename)
{
	_fb_->File = realloc(_fb_->File, sizeof(struct FILEINFO) * (_fb_->File_Count + 1));
	if(_fb_->File == NULL)
		return 0;
		
	_fb_->File[_fb_->File_Count].Name = calloc(strlen(filename) + 1, sizeof(char));
	if(_fb_->File[_fb_->File_Count].Name == NULL)
		return -1;
		
	strcpy(_fb_->File[_fb_->File_Count].Name, filename);	
	_fb_->File_Count++;
	return 1;
}

inline s16 FB_ScanDir( const char *directory, FileBrowser *_fb_,u8 Num_Types, char *exts[], int folders)
{
	//check if a new directory has been entered before resetting everything
    FB_Reset(_fb_);
	struct stat st;
	//reset file list
	strcpy(_fb_->Current_Dir, directory);
	DIR* dir = opendir(directory);
	char *file_name = NULL;
	//check if directory exists
	if(!dir)
		return -1;
		
	//if to show all files
	if(GET_FLAG(_fb_->Flags, FBFlag_ShowAllFiles))	
		Num_Types = 0;
		
	//random variables
	s16 Same_Ext = 0;
	//collect all extentions givent to compare to
	//scan through the directory and grab the files with the same extentions
	dirent *current_file = NULL;
	while ((current_file = readdir_ex(dir, &st)) != NULL) 
	{
		file_name = current_file->d_name;
		//current file is actually a folder
		if(S_ISDIR(st.st_mode) && folders && _fb_->Dir_Count < _fb_->Max_Dirs)
		{ //is a folder
			//check common characters for hidden folders
			if(strcmp(file_name, ".."))
			{
				if(checkHiddenDirChars(file_name))
					goto SKIPFOLDER;
			}
			
			//check if folder matches the list
			if(FB_isFolderHidden(file_name, _fb_) == 1)
				goto SKIPFOLDER;
			
			int test = _fb_addDir(_fb_, file_name);
			if(test != 1)
				return test;
			
			SKIPFOLDER:;		
		}
		//if current file is not a directory
		else if(!(st.st_mode & S_IFDIR) && _fb_->File_Count < _fb_->Max_Files && !GET_FLAG(_fb_->Flags, FBFlag_HideAllFiles)) 
		{			
			if(FB_isFileHidden(file_name, _fb_) == 1)
				goto SKIPFILE;
					
			Same_Ext = ScanExt(exts, file_name, Num_Types);		
			//the file does have an extention we asked for
			if(Same_Ext <= 0)
				goto SKIPFILE;

			int test = _fb_addFile(_fb_, file_name);
			if(test != 1)
				return test;
				
			SKIPFILE:;
		}//end of file actions
		
		else if(_fb_->File_Count >= _fb_->Max_Files )
			break;
			
		BAG_UpdateAudio();
	}
	closedir(dir);
	return _fb_->File_Count + _fb_->Dir_Count;
}

//sorts folders on top in alphabetical order, then files underneath in alphabetical order
inline int sort_function(const struct FILEINFO *dest_str_ptr, const struct FILEINFO *src_str_ptr)
{
  char *dest_str = dest_str_ptr->Name;
  char *src_str = src_str_ptr->Name;
 
  if(src_str[0] == '.')
    return 1;

  if(dest_str[0] == '.')
    return -1;

  return strcasecmp(dest_str, src_str);
}

inline int my_array_partion(struct FILEINFO *array, int left, int right)
{
    struct FILEINFO  pivot = *((struct FILEINFO *)array + left);

    while(left < right)
    {
        while(sort_function((void*)((struct FILEINFO *)array+left), (void*)((struct FILEINFO *)array+right)) < 0) {
            right--;
        }

        if(right== left) break;
        *((struct FILEINFO *)array + left) = *((struct FILEINFO *)array + right);
        *((struct FILEINFO *)array + right) = pivot;

        if(left < right)
        {
            left++;
            if(right== left) break;
        }

        while(sort_function((void*)((struct FILEINFO *)array+right), (void*)((struct FILEINFO *)array+left)) > 0) {
            left++;
        }

        if(left== right) break;
        *((struct FILEINFO *)array + right) = *((struct FILEINFO *)array + left);
        *((struct FILEINFO *)array + left) = pivot;
        right--;
    }

    return left;
}

inline void QSort_Alpha(void *array, int left, int right)
{
    if(left < right)
    {
        int mid= my_array_partion(array, left, right);
        QSort_Alpha(array, left, mid-1);
        QSort_Alpha(array, mid+1, right);
    }
}


/*===============================================================
Filebrowser interface
Graphical Interface for filebrowser handling
===============================================================*/

UI_FB * FBUI_Init(int max_files, int (*renderFun)(UI_FB * gui))
{
	UI_FB * gui = NULL;
	
	gui = calloc(sizeof(UI_FB), 1);
	if(!gui)
		goto err;
		
	gui -> FB_Dir = FB_Init (max_files, MAX_DIRS_PER_DIR);
	if(!gui -> FB_Dir)
	{
		free(gui);
		goto err;
	}

	gui->Render_FB = renderFun;
	return gui;
	
	err:
		return NULL;
}


void FBUI_Free( UI_FB * gui)
{
	if(gui)
	{
		FB_Free (gui->FB_Dir);
		free(gui);
		gui = NULL;
	}
}


void FBUI_ShowFiles(UI_FB * gui, int types, char *file_types[])
{
	FB_SetSupportedExt(gui->FB_Dir, types, file_types);
}

//file browser initialization==========================================
int _fbuiDefault_NULL(void)
{
	return 0;
}

void FBUI_RegisterKeyAction(UI_FB * gui, int action, int (*input)(void))
{
	if(input != NULL)
		gui->FBInput[action] = input;
	else
		gui->FBInput[action] = &_fbuiDefault_NULL;
}

//default control setup
int _fbuiDefault_NextFile(void)
{
	if(Pad.Newpress.Down)
		return 1;
	else if(Pad.Held.Down)
		return 2;
		
	return 0;
}

int _fbuiDefault_PrevFile(void)
{
	if(Pad.Newpress.Up)
		return 1;
	else if(Pad.Held.Up)
		return 2;
		
	return 0;
}	

int _fbuiDefault_NextPage(void)
{
	if(Pad.Newpress.Right)
		return 1;
	else if(Pad.Held.Right)
		return 2;
		
	return 0;
}

int _fbuiDefault_PrevPage(void)
{
	if(Pad.Newpress.Left)
		return 1;
	else if(Pad.Held.Left)
		return 2;
		
	return 0;
}	


int  _fbuiDefault_Open(void)
{
	return Pad.Newpress.A;
}		

int  _fbuiDefault_PrevDir(void)
{
	return Pad.Newpress.B;
}		

int _fbuiDefault_Stylus(void)
{
	if(Stylus.Newpress)
		return 1;
	else if(Stylus.Held)
		return 2;
		
	return 0;
}

//keys that exit filebrowser loop
int _fbuiDefault_Escape(void)
{
	if(Pad.Newpress.Y)
		return FB_ESCAPE1;
	else if(Pad.Newpress.X)
		return FB_ESCAPE2;
	else if(Pad.Newpress.Start)
		return FB_ESCAPE3;
	else if(Pad.Newpress.Select)
		return FB_ESCAPE4;
		
	return 0;
}

void FBUI_SetDefaultInput(UI_FB * gui)
{
	//register keys
	FBUI_RegisterKeyAction(gui, FBPad_NextFile, _fbuiDefault_NextFile);
	FBUI_RegisterKeyAction(gui, FBPad_PrevFile, _fbuiDefault_PrevFile);
	FBUI_RegisterKeyAction(gui, FBPad_NextPage, _fbuiDefault_NextPage);
	FBUI_RegisterKeyAction(gui, FBPad_PrevPage, _fbuiDefault_PrevPage);
	FBUI_RegisterKeyAction(gui, FBPad_Open,  _fbuiDefault_Open);
	FBUI_RegisterKeyAction(gui, FBPad_PrevDir,  _fbuiDefault_PrevDir);
	FBUI_RegisterKeyAction(gui, FBPad_Stylus,  _fbuiDefault_Stylus);
	FBUI_RegisterKeyAction(gui, FBPad_Escape,  _fbuiDefault_Escape);
}

int _fbuiCheck_Escape(int input)
{
	int i = 0;
	for(i = 0; i < MAX_ESCAPES; i++)
		if(input == escape_table[i])
			return escape_table[i];
			
	return 0;
}


//file browser gui core==========================================
void FBUI_OpenDir(const char *folder, UI_FB * gui)
{
	char path[MAX_PATH];
	memset(path, 0, MAX_PATH);
	
	int open = 0;

	if(folder != NULL && !strcmp(folder, ".."))
	{
		FB_DirBack(gui->FB_Dir->Current_Dir);
		sprintf(path,"%s", gui->FB_Dir->Current_Dir);
	}
	else
	{
		if(folder != NULL)
			sprintf(path,"%s%s", gui->FB_Dir->Current_Dir, folder);
		else
			sprintf(path,"%s", gui->FB_Dir->Current_Dir);
	}
		
	FB_FixFilePath(path);		
	
	if(FB_ifNewDir(gui->FB_Dir, path))	
		open++;
	if(open)
	{
		if(FB_ScanDir(path,gui->FB_Dir , gui->FB_Dir->types, gui->FB_Dir->Supported_Ext, 1)<0)
			return;

		if(gui->FB_Dir->Dir_Count > 0)
		{
			if(strcasecmp(path, "/"))
				QSort_Alpha(gui->FB_Dir->Dir, 1, gui->FB_Dir->Dir_Count-1);
			else
				QSort_Alpha(gui->FB_Dir->Dir, 0, gui->FB_Dir->Dir_Count-1);
		}
		if(gui->FB_Dir->File_Count > 0)
			QSort_Alpha(gui->FB_Dir->File, 0, gui->FB_Dir->File_Count-1);
	}
	
	//gui->Render_FB(gui->FB_Dir, &gui->State.File_Selected);
	FBUI_RenderCore(gui);
	FB_UpdateNewDir(gui->FB_Dir);
	BAG_UpdateIN();
	BAG_ResetDblClick();
}

//refresh filebrowser ui with new settings
void FBUI_Refresh(UI_FB * gui)
{
    memset(gui->FB_Dir->Old_Dir, 0, sizeof(gui->FB_Dir->Old_Dir));
	gui->State.OldFile = -1;
	gui->Render_FB(gui);
}

void FBUI_SetPageSkip(UI_FB * gui, int page_skip)
{
    gui->Paging_Val = page_skip;
}

int FBUI_ifPagingScroll(UI_FB * gui)
{
	UI_State * state = &gui->State;
	if(abs((state->OldFile) - abs(state->File_Selected)) > 1)
		return 1;

	return 0;
}

void FBUI_SetFilesPerScreen(UI_FB * gui, int file_count)
{
	gui->State.FilesPerScreen = file_count;
}

//filebrowser loop
int FBUI_Run (UI_FB * gui, char *file_load, int types, char *file_types[])
{
	if(gui == NULL)
		return 0;
		
	//set up supported file types
	FBUI_ShowFiles( gui, types, file_types);
	FBUI_OpenDir(gui->State.Folder, gui);
	memset(gui->State.Folder, 0, sizeof(gui->State.Folder));
	
	//declare some pointers to shorten writing
	FileBrowser * FB = gui->FB_Dir;
	int * File_Num = (int*)&gui->State.File_Selected;	
	//shorten up input functions
	typedef int (*keyFunc) (void);
	keyFunc NextFile = gui->FBInput[FBPad_NextFile];
	keyFunc PrevFile = gui->FBInput[FBPad_PrevFile];
	keyFunc NextPage = gui->FBInput[FBPad_NextPage];
	keyFunc PrevPage = gui->FBInput[FBPad_PrevPage];
	keyFunc Open = gui->FBInput[FBPad_Open];
	keyFunc PrevDir = gui->FBInput[FBPad_PrevDir];
	keyFunc Stylus =  gui->FBInput[FBPad_Stylus];
	keyFunc Escape =  gui->FBInput[FBPad_Escape];
	
	BAG_UpdateIN();
	BAG_ResetDblClick();
    
    //number of browser entries
    int file_skip = 1;
	u8 fb_run = 1;
	int load = 0, padTimer = 0, escape_check = 0;
	gui->Render_FB(gui);
	while(fb_run)
	{
        int Entries = FB->File_Count + FB->Dir_Count;
		escape_check = 0;
		
        int next_file = NextFile(),
			prev_file = PrevFile(),
            next_page = NextPage(),
            prev_page = PrevPage(),
			stylus = Stylus(),
			open = Open(),
			prev_dir = PrevDir();

		escape_check += Escape();
		escape_check += _fbuiCheck_Escape(next_file);
		escape_check += _fbuiCheck_Escape(prev_file);
		escape_check += _fbuiCheck_Escape(next_page);
		escape_check += _fbuiCheck_Escape(prev_page);
		escape_check += _fbuiCheck_Escape(stylus);
		escape_check += _fbuiCheck_Escape(open);
		escape_check += _fbuiCheck_Escape(prev_dir);		



		if(!escape_check)
		{
            if(next_page == 1 || prev_page == 1)
                file_skip = gui->Paging_Val;
        
			if(next_file == 2  || prev_file == 2 || next_page == 2 || prev_page == 2)
				if(padTimer<20)padTimer++;

			//select next file
			if((next_file == 1 || next_page == 1) || ( (next_file == 2 || next_page == 2) && padTimer>=20))
			{
				//check file if it is in range of the list then check if the file selected variable wasn't externally modified in a control function
				if(*File_Num < Entries - 1 && *File_Num == gui->State.OldFile)
					(*File_Num)+= file_skip;
			}
		
			//dpad up scrolling
			if( (prev_file == 1 || prev_page == 1) || ((prev_file == 2 || prev_page == 2)&& padTimer>=20))
			{
				//check file if it is in range of the list then check if the file selected variable wasn't externally modified in a control function
				if(*File_Num > 0 && *File_Num == gui->State.OldFile)
					(*File_Num)-= file_skip;
			}	
			
			if(!prev_file && !next_file && !prev_page && !next_page)
            {
                file_skip = 1;
				padTimer = 0;
            }
				
			//Update display when new file selected
			if(gui->State.OldFile != *File_Num || stylus)
			{
				escape_check = FBUI_RenderCore(gui);
				//gui->Render_FB(FB, File_Num);
				gui->State.OldFile = *File_Num;
			}
			

			//open dir
			if((open || stylus == 1) && *File_Num < FB->Dir_Count)
			{
				int tempFile = *File_Num;
				*File_Num = 0;//reset file selected to first file
                gui->State.OldFile=-1;
				FBUI_OpenDir(FB->Dir[tempFile].Name, gui);	
				BAG_UpdateIN();
				stylus = 0;
				open = 0;
			}
			//open file
			if ((open || stylus == 1))
			{
				BAG_UpdateIN();
				stylus = 0;
				open = 0;
				//get path of file to load and copy it to the file_load string
				char TempBuf[MAX_PATH];
                memset(TempBuf, 0, sizeof(TempBuf));
                //check if there is a / at the end of the path
                int z = strlen(FB->Current_Dir);
                int tempFile = *File_Num - FB->Dir_Count;
                
                if(FB->Current_Dir[z] != '/' && FB->Current_Dir[z - 1] != '/')
                    sprintf(TempBuf, "%s/%s",FB->Current_Dir ,FB->File[tempFile].Name);
                else
                    sprintf(TempBuf, "%s%s",FB->Current_Dir ,FB->File[tempFile].Name);
                    
				strcpy(file_load, TempBuf);
				
				//exit and return value
				load = FB_LOAD;
				fb_run = 0;
			}
			
			//go back 1 dir or exit filebrowser
			if(prev_dir == 1)
			{
				if(strcmp(FB->Current_Dir, "/"))
				{
					*File_Num = 0;
					FBUI_OpenDir("..", gui);
				}
			}
		}
		else
			fb_run = 0;

		BAG_Update();
	}
	
	if(escape_check != 0)
		load = escape_check;
	
		return load;
}

int FBUI_RenderCore(UI_FB * gui)
{	
	FileBrowser * FB = gui->FB_Dir;
    int Entries = FB->File_Count + FB->Dir_Count;
    int Files_Screen = gui->State.FilesPerScreen;
	int *Old_File = &gui->State.OldFile;
	int *Cursor_Scroll = &gui->State.Cursor_Scroll;
	int *List_Scroll = &gui->State.List_Scroll;
	int *File_Selected = &gui->State.File_Selected;
	
	if(FB_isNewDir(FB))
	{
		(*Cursor_Scroll) = 0;
		(*List_Scroll) = 0;
		(*Old_File) = 0;
		(*File_Selected) = 0;
	}
    if(Entries < Files_Screen)
        Files_Screen = Entries;
        
    int list_diff = 1, cursor_diff = 1;
    while(*File_Selected > Entries - 1)
        (*File_Selected)--;
        
    while(*File_Selected < 0)
        (*File_Selected)++;
		
	//check if input was paging
    if(FBUI_ifPagingScroll(gui))
    {
        list_diff =  Files_Screen;
		cursor_diff = 0;
    }
        
	//scrolling down the list
	if((*Old_File) < *File_Selected && *File_Selected <=  Entries -1)
	{
		(*Cursor_Scroll)+=cursor_diff;
		if((*Cursor_Scroll) >  Files_Screen - 1 || list_diff > 1)
		{
			if(list_diff == 1)
                (*Cursor_Scroll) =  Files_Screen -1;
                
			(*List_Scroll) +=list_diff; 
            if((*List_Scroll)  +  Files_Screen -1  >= Entries)
            {
                (*List_Scroll)  = Entries -  Files_Screen;
                (*Cursor_Scroll) =( Files_Screen -1) - ((Entries-1) - *File_Selected) ;
            }
		}
	}
		
	//scrolling down the list
	if((*Old_File) > *File_Selected && *File_Selected >=0)
	{
		(*Cursor_Scroll)-=cursor_diff;
		if((*Cursor_Scroll) < 0 || list_diff > 1)
		{
			if(list_diff == 1)
                (*Cursor_Scroll) = 0;
			(*List_Scroll) -= list_diff;
			if((*List_Scroll)  <= 0)
            {
                (*Cursor_Scroll) = *File_Selected;
				(*List_Scroll)  = 0;	
            }
		}
	}
	
	//draw the interface
	int rtrn = gui->Render_FB(gui);
	(*Old_File) = *File_Selected;
	return rtrn;
}

