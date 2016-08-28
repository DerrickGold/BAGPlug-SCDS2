//ds2_main.c
#include <libBAG.h>
#include "minIni.h"
#include "draw.h"
#include "fb_api.h"
#include "supportedFiles.h"

#include "interface.h"
#include "music_player.h"
#include "settings.h"
#include "ndsheader.h"
#include "fileops.h"

#include "menu_buttons.h"
#include "menu_files.h"
#include "usermode.h"
#include "language.h"
#include "screen_cap.h"

int FILE_LAUNCH_VAL = 0;
/*========================================================================
File Handling Code
========================================================================*/

//launch a file with args
void File_Args(const char *filedest, int argc,  ...)
{
	FILE* dat = fopen(filedest, "wb");
	if(dat)
	{
		va_list vl;
		va_start(vl, argc);
		int i = 1;
		fprintf(dat, "%s", va_arg(vl, u8* ));
		for(i = 1; i < argc; i++)
			fprintf(dat, "\n%s", va_arg(vl, u8* ));
			
		va_end(vl);
		 
		fclose(dat);
	}
}


void noSupportMsg(const char *ext, bool internal)
{
	typedef enum{
		__WINDOW_TITLE,
		__MESSAGE0,
		__MESSAGE1,
		__MESSAGECOUNT,
	}overwritemsgs;
	
	struct _language msgs;
	memset(&msgs, 0, sizeof(struct _language));
	if(Language_InitBuffers(&msgs, __MESSAGECOUNT) != 1)
		return;
		
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__WINDOW_TITLE], "Error Messages", "window_title", NULL);
	
	if(internal)
	{
		Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__MESSAGE0], "Error Messages", "internal_msg0", NULL);
		Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__MESSAGE1], "Error Messages", "internal_msg1", ext);
	}
	else
	{
		Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__MESSAGE0], "Error Messages", "external_msg0", NULL);
		Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__MESSAGE1], "Error Messages", "external_msg1", ext);	
	}

    Unicode_Notification_Window(msgs.buffer[__WINDOW_TITLE], (u16**)&msgs.buffer[__MESSAGE0], __MESSAGECOUNT - 1, 1);
	Language_CleanBuffers(&msgs);
}

int run_internal(const char *filename, const char * ext, EXTINFO * info, int program_id)
{
	int err = Internal_Programs( filename, ext, info, program_id);
	//no internal program found
	if (!err)
		noSupportMsg(ext, 1);
    
	return err;
}



int run_external(const char *filename, const char *ext, EXTINFO * info, int program_id)
{
	if(program_id >= 0)
	{
		if(!FB_FileExists(info->Entry[program_id].Program))//check file if it exists
			noSupportMsg(ext, 0);

		FILE* dat = fopen(GlobalPaths.LoadFile_dat, "wb");
		int i = 0;
		for ( i = 0; i < info->Entry[program_id].num_args; i++)
		{
			if(!strcasecmp(EXTSCRIPT[EXT_PRGMPATH], info->Entry[program_id].args[i]))
				fprintf(dat, "%s", info->Entry[program_id].Program);
			else if(!strcasecmp(EXTSCRIPT[EXT_FILEPATH], info->Entry[program_id].args[i]))
				fprintf(dat, "\n%s", filename);
			else
				fprintf(dat, "\n%s", info->Entry[program_id].args[i]);
		}
		fclose(dat);
		if(info->Entry[program_id].fill_eos > 0)
			Use_EOS(filename);
		
		Write_ExtLinkDat(info->Entry[program_id].Program, filename);
		return FILE_NDSPLG;
	}
	return 0;
}




int Load_File(char *filename)
{
	BAG_Update();
    FB_FixFilePath(filename);
	int err = 0, program = -1;
    
    //get file extention
    char ext[EXT_NAME_LEN];
    memset(ext, 0, sizeof(ext));
    if(!FB_GetExt(filename, ext, EXT_NAME_LEN))
        return 0;       
    
    //allocate memory for program list
    //EXTINFO *info = calloc(sizeof(EXTINFO), 1);
	EXTINFO *info = NULL;
	info = calloc(sizeof(EXTINFO), 1);
	if(info == NULL)
		return 0;
	//memset(&info, 0, sizeof(EXTINFO));
	
	if(!Init_EXTFile(info))
	{
		free(info);
		return 0;
	}
        
    //read extention details
    if(!Read_ExtFile(filename, ext, info))
	{
		err = 0;
		goto CLEANUP;
	}
		
	//if there is only one set program, then launch the default program
	if(info->num_programs < 2)
	{
		//check for using internal file support
		if(!strcasecmp(EXTSCRIPT[EXT_INTERNAL], info->Entry[0].Program))
			err = run_internal(filename, ext, info, 0);
		else
			err = run_external(filename,  ext, info, 0);
		goto CLEANUP;
	}

	//handle more than one programs
	program = OpenWith_Menu(info);
	if(program >= 0)
	{
		if(!strcasecmp(EXTSCRIPT[EXT_INTERNAL], info->Entry[program].Program))
			err = run_internal(filename, ext, info, program);
		else
			err = run_external(filename,  ext, info, program);
	}
	else
		err = 0;

	CLEANUP:
	if(err != 0)//if a file is being launched, then write last folder
		Write_Last_Folder(filename);
		
	
	Free_EXTFile(info);
	free(info);   
	
	return err;	
}	


/*========================================================================
Main Function
========================================================================*/



UI_FB * Main_Browser = NULL;
void Initialize_Filebrowser(void)
{
	NDS_ROM_LANG = ini_getl((const char*)"General", (const char*)"nds_header_lang", 1, GlobalPaths.Language);

	loadAllTypes();
	Load_DaysText();
	Initialize_Player();

	Main_Browser = FBUI_Init(MAX_FILES_PER_DIR, &Draw_Dir);
	FB_ReadHidDirList(GlobalPaths.HiddenFolders_txt, Main_Browser->FB_Dir);

	
	FBUI_RegisterKeyAction(Main_Browser, FBPad_NextFile, _fbuiDefault_NextFile);
	FBUI_RegisterKeyAction(Main_Browser, FBPad_PrevFile, _fbuiDefault_PrevFile);
	FBUI_RegisterKeyAction(Main_Browser, FBPad_NextPage, _fbuiDefault_NextPage);
	FBUI_RegisterKeyAction(Main_Browser, FBPad_PrevPage, _fbuiDefault_PrevPage);
	FBUI_RegisterKeyAction(Main_Browser, FBPad_Open,  _fbuiDefault_Open);
	FBUI_RegisterKeyAction(Main_Browser, FBPad_PrevDir,  _fbuiDefault_PrevDir);
	FBUI_RegisterKeyAction(Main_Browser, FBPad_Escape,  _fbuiDefault_Escape);
	FBUI_RegisterKeyAction(Main_Browser, FBPad_Stylus, StylusFun);

	FBSETTINGS tempset;
	tempset = Settings_Read_Ini(Main_Browser, &UI_Style, GlobalPaths.Settings_ini, 0);
	memcpy(&Main_Settings, &tempset, sizeof(FBSETTINGS));
		
	//hidden file list
	ReadAllLists();
	RefreshMainLists();
	
	SetUIStyle(&UI_Style, UI_Style.Current_Skin);
    Init_IconCache(&UI_Style);
    
    FBUI_SetPageSkip(Main_Browser, UI_Style.FilesPerScreen);
	FBUI_SetFilesPerScreen(Main_Browser,  UI_Style.FilesPerScreen);
}


		
void Sleep_Mode(void)
{
    if(Lid.Closed)
    {
		int tempCpu = cpuVal_old;
		//enable bg controls for music
		if(tempCpu == SETTINGS_CPU_MUSIC || tempCpu == SETTINGS_CPU_BGPLAY || tempCpu == SETTINGS_CPU_FAVS_BGMUSIC)
			MusicPlayer->BGplay = 1;
			
		Change_Cpu(SETTINGS_CPU_SLEEP, &Main_Settings);
        ds2_setBacklight(0);
		while(!Lid.Opened)
		{
			Background_Player();
			BAG_UpdateIN();
			BAG_UpdateAudio();
		}
		
		if(tempCpu == SETTINGS_CPU_MUSIC)
			MusicPlayer->BGplay = 0;
			
		Change_Cpu(tempCpu, &Main_Settings);
        ds2_setBacklight(3);
	}
}


extern void Play_Audio(SndFile *snd);		
void Load_BootSong(void)
{
	if(Main_Settings.bootSong[0] != '~' && Main_Settings.bootSong[0] != ' ' && Main_Settings.bootSong[0] != '\0')
	{
		if(!FB_FileExists(Main_Settings.bootSong))
			return;
			
		char ext[EXT_NAME_LEN];
		FB_GetExt(Main_Settings.bootSong, ext, sizeof(ext));
		if(Launch_Audio(Main_Settings.bootSong, ext))
		{
			Play_Audio(&MusicPlayer->AudioFile);
			MusicPlayer->BGplay = 1;
			BAG_SetSync(&Background_Player);
		}
	}
}

void Clean_All(void)
{
	ClearLists();
	Clear_DaysText();
	Clear_Name_Cache(&InternalNames);
    Free_IconCache(&UI_Style);
	Unload_GFX (&UI_Style);   
    Unload_MediaPlayer_Gfx(MusicPlayer);
	FreeMediaPlayer(MusicPlayer);
	MusicPlayer = NULL;

	FBUI_Free(Main_Browser);
	Main_Browser = NULL;
}



int Get_Args(char *file, char **filebuf, int lim)
{
    FILE* dat = fopen(file, "rb");
    if(dat)
    {
        int i = 0;
        while(!feof (dat) && i <= lim)
        {
            char * test = fgets(filebuf[i], MAX_PATH, dat);
			if(test == NULL)
				break;

            int len = strlen(filebuf[i]);
			while((filebuf[i][len] == '\0' || filebuf[i][len] == '\n' || filebuf[i][len] == '\r') && len > 0)
				filebuf[i][len--] = '\0'; 
				
			if(len > 0)
				i++;
        }
            
        fclose(dat);
        remove(file);
        return i;
    }
    return 0;
}

char global_launchFile[MAX_PATH];

void SetRoot(void)
{
	//check for args (BAGPlug and iMenu for DSTwo)
	char argv_buffer[1][MAX_PATH];// [0] for file path of launched binary, [1] for file path of selected rom
	memset(argv_buffer[0], 0, sizeof(argv_buffer[0]));
	
	char *argv[1] = { (char*)&argv_buffer[0] };
	int argc = Get_Args("/plgargs.dat", argv, 1);	
	
	//set root directory
	char rootPath[MAX_PATH];
	memset(rootPath, 0, sizeof(rootPath));
	if(argc > 0)
	{
		FB_GetFilePath(rootPath, argv[0]);
		strcat(rootPath, "/");
	}
	else
		strcpy(rootPath, "/_bagui/");

	Global_Set_Paths(rootPath);
	printf("root set to:\n %s\n", rootPath);
	User_SetMode("a");//set admin as default mode
}

void Load_Splash(void)
{
	typedef enum
	{
		top_screen = 0,
		bot_screen,
	}screenvals;

	BMPOBJ splash[2];

	
	int err = Load_Image(GlobalPaths.SplashScreen, &splash[bot_screen], 256, 192);
	if(err != NO_ERR)
		goto CLEANUP;

	BAG_SetObjFrame( &splash[bot_screen], FRAME_VERT, 1);
	
	err = BAG_Display_CopyObj(&splash[bot_screen], &splash[top_screen]);
	if(err != NO_ERR)
		goto CLEANUP;

	BAG_SetObjFrame(&splash[top_screen], FRAME_VERT, 0);	
	

	BAG_DrawObj(&splash[top_screen], up_screen_addr, 0, 0);
	BAG_DrawObj(&splash[bot_screen], down_screen_addr, 0, 0);
	
	ds2_flipScreen(DUAL_SCREEN, 1);
	
	struct key_buf input;
	memset(&input, 0, sizeof(struct key_buf));
	
	while(input.key == 0)
	{
		memset(&input, 0, sizeof(struct key_buf));
		ds2_getrawInput(&input);
	}
	ds2_clearScreen(DUAL_SCREEN, RGB15(0,0,0));
	ds2_flipScreen(DUAL_SCREEN, 1);
		
	CLEANUP:
		BAG_Display_DeleteObj(&splash[top_screen]);
		BAG_Display_DeleteObj(&splash[bot_screen]);
}

void ds2_main(void)
{
	//initialize the library
	if(!BAG_Init(1))
		ds2_plug_exit();
	
	BAG_DBG_Init(NULL, 0, 0);
	printf("BAGPlug R 3-5\n");
	
	SetRoot();
	
    //SET_FLAG(_BAG_Debug.Flags, DBG_HALT);//set program to halt on errors
    BAG_SetSync(&Sleep_Mode);//initialize sleep mode
	BAG_SetSync(&Render_TopScreen);
	
	SetScreenCap(0);
	BAG_SetSync(&ScreenCap);
	
	//display splash screen if in root of bagui
	Load_Splash();
	
	//initialize filebrowser
	Initialize_Filebrowser();
	
	ForceTopScreenRefresh();
	int loop = 1, menu = 0;
	
	if(Main_Settings.bootFavs || !UserPerm_CheckBrowser())
		menu = FAVORITE_ESCAPE;
		
	Load_BootSong();
	
	while(loop)
	{	
		if(menu == FB_LOAD || menu == FB_LOAD_SKIP)
		{
			if(menu != FB_LOAD_SKIP)
			{
				Render_DirGfx(down_screen_addr);
				FILE_LAUNCH_VAL = Load_File(global_launchFile);
			}
			
			if( FILE_LAUNCH_VAL == FILE_AUDIO)
				menu = MUSIC_ESCAPE;
				
			else if(FILE_LAUNCH_VAL == FILE_NDSPLG)
			{
				loop = 0;
				menu = 0;
				break;
			}
			else if(FILE_LAUNCH_VAL == FILE_NZIP)
			{
				FB_StripExt(global_launchFile);
				strcat(global_launchFile, ".");
				strcat(global_launchFile, sfiles_exec[0]);
				//incase of launching from favorites
				menu = FB_LOAD;
			}
			else
				menu = 0;				
		}
		
		if(menu == MUSIC_ESCAPE)
		{
			Change_Cpu(SETTINGS_CPU_MUSIC, &Main_Settings);
			menu = Media_Player(MusicPlayer, 1);
		}
		else if(menu == FAVORITE_ESCAPE)
		{
			if(MusicPlayer->BGplay == 0)
				Change_Cpu(SETTINGS_CPU_FAVS, &Main_Settings);
			else
				Change_Cpu(SETTINGS_CPU_FAVS_BGMUSIC, &Main_Settings);
				
			menu = Favorites_Menu(global_launchFile);
		}
		else if(menu == SETTINGS_ESCAPE)
		{
			Settings_Main(Main_Browser, &UI_Style);
			menu = 0;
		}
		else if(menu == FILEOPS_ESCAPE)
		{
            File_OpsMenu(&ClipBoard, Main_Browser);
			menu = 0;
		}
		else if(menu == NZIP_MANAGER_ESCAPE)
		{
			Nzip_Manager();
			menu = 0;
		}
		

        if(menu != FB_LOAD && menu != FB_LOAD_SKIP)
		{
			if(MusicPlayer->BGplay == 0)
				Change_Cpu(SETTINGS_CPU_DEFAULT, &Main_Settings);
			else
				Change_Cpu(SETTINGS_CPU_BGPLAY, &Main_Settings);
				
            menu = FBUI_Run(Main_Browser, global_launchFile, stypes_Count, sfiles_all);
		}	
	}
    Clean_All();
    ds2_clearScreen(UP_SCREEN, 0);
    Flip_Screen_Fun(UP_SCREEN, 0);
    ds2_clearScreen(DOWN_SCREEN, 0);
    Flip_Screen_Fun(DOWN_SCREEN, 0);
	ds2_plug_exit();
}

