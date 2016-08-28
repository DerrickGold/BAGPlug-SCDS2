#include <libBAG.h>
#include "supportedFiles.h"
#include "draw.h"
#include "interface.h"
#include "ndsheader.h"
#include "fb_api.h"
#include "minIni.h"
#include "settings.h"
#include "window.h"
#include "music_player.h"
#include "menu_files.h"
#include "language.h"
#include "screen_cap.h"

//default size
#define COPY_BUF_LEN 131072


u16 cpu[14] = {0, 120, 0, 144, 192, 204, 240, 264, 288, 300, 336, 360, 384, 396};
FBSETTINGS Main_Settings;
u16 cpuVal_old = -1;

//convert a file path to folder path and write data
void Write_Last_Folder(const char *path)
{
	char *lastpath = calloc(MAX_PATH, 1);
    if(lastpath)
    {
        FB_GetFilePath(lastpath, path);
        
        FILE * browserdat = fopen(GlobalPaths.InitData_dat, "wb");
        if(browserdat)
        {
            fprintf(browserdat, "%s", lastpath);
            fclose(browserdat);
        }
        
        free(lastpath);
    }
}

void Read_Last_Folder(const char *path, UI_FB * gui)
{
	FILE * browserdat = fopen(path, "rb");
	if(browserdat)
	{
		char tempbuf[MAX_PATH];
		memset(tempbuf, 0, sizeof(tempbuf));  
		fgets (gui->State.Folder, MAX_PATH, browserdat);
		fclose(browserdat);
		
		FB_GetFilePath(tempbuf, gui->State.Folder);
		FB_FixFilePath(tempbuf);
		
		memcpy(gui->State.Folder, tempbuf, MAX_PATH);
		DIR* dir = opendir(gui->State.Folder);
		if(!dir)
		{
			memset(gui->State.Folder, 0, sizeof(gui->State.Folder));
			strcpy(gui->State.Folder, "/");			
		}
		else
			closedir(dir);
	}
	else
		strcpy(gui->State.Folder, "/");
}



FBSETTINGS Settings_Read_Ini(UI_FB * gui, GUI *skinInfo, const char *filename, bool skip_last_folder)
{
	FBSETTINGS settings;
	
	settings.CombineFavsHidFiles = ini_getl("Settings", "hide_favorited_files", 0, filename);
	//folder mode
	//load last folder opened
	settings.lastfolder = ini_getl("Settings", "last_folder", 0, filename);
	if(settings.lastfolder && !skip_last_folder)
		Read_Last_Folder(GlobalPaths.InitData_dat, gui);

	
	//load skin
	ini_gets("Settings","skin", "Default", settings.skin_name, MAX_PATH, filename);
    memset(skinInfo->Current_Skin, 0, sizeof(skinInfo->Current_Skin));
	sprintf(skinInfo->Current_Skin,"%s%s", GlobalPaths.Skin_Root_Folder, settings.skin_name);

	//screen brightness
	settings.brightness = ini_getl("Settings", "brightness", 0, filename);
	ds2_setBrightness(settings.brightness);
	//file extensions
    settings.extensions = ini_getl("Settings", "show_extensions", 1, filename);
    skinInfo->ShowExt = settings.extensions;
	//hide favorites text
	settings.favs_hideTxt = ini_getl("Settings", "hide_favorites_txt", 0, filename);
	//internal nds names
	settings.ndsInternalName = ini_getl("Settings", "nds_internal_names", 0, filename);
	//24 hour clock
	settings.clock24Hour = ini_getl("Settings", "24_hour_clock", 0, filename);
	
	//24 hour clock
	settings.clock24Hour = ini_getl("Settings", "24_hour_clock", 0, filename);
	//auto show favorites
	settings.bootFavs = ini_getl("Settings", "auto_show_favorites", 0, filename);
	//boot song
	ini_gets("Settings","boot_song", "~", settings.bootSong, MAX_PATH, filename);
	//use transparency for nds icons
	settings.ndsTrans = ini_getl("Settings", "nds_icon_transparency", 1, filename);
	
	//load cpu settings
	settings.cpu_default = ini_getl("CPU", "default", 240, filename);
	settings.cpu_media = ini_getl("CPU", "media_player", 240, filename);
	settings.cpu_bgmusic = ini_getl("CPU", "bg_music", 240, filename);
	settings.cpu_sleep = ini_getl("CPU", "sleep", 240, filename);
	settings.cpu_favs = ini_getl("CPU", "favorites", 240, filename);
	settings.cpu_favs_bgmusic = ini_getl("CPU", "favorites_music", 240, filename);
	settings.cpu_nzip_decompress = ini_getl("CPU", "nzip_decompression", 396, filename);
	
	settings.copy_buf_len = ini_getl("System", "copy_buffer_length", COPY_BUF_LEN, filename);
	settings.copy_display_update = ini_getl("System", "copy_display_update", 2, filename);
	settings.topscreen_refresh = ini_getl("System", "top_display_refresh", 25, filename);
	return settings;
}


int Settings_Write_Ini(FBSETTINGS * settings, const char *filename)
{
	//remove(filename);
	FILE * inifile = fopen(filename, "wb");
	if(!inifile)
		goto err;
	
	//write the ini section header
	fprintf( inifile, "[Settings]\n");
	//write hide favorites
	fprintf( inifile, "hide_favorited_files = %d\n", settings -> CombineFavsHidFiles);
	//write last folder
	fprintf( inifile, "last_folder = %d\n", settings -> lastfolder);
	//write skin name
	fprintf( inifile, "skin = %s\n", settings -> skin_name);
	//write brightness
	fprintf( inifile, "brightness = %d\n", settings -> brightness);
    //write file extention
    fprintf( inifile, "show_extensions = %d\n", settings -> extensions);
	//hide favorites text
	fprintf( inifile, "hide_favorites_txt = %d\n", settings -> favs_hideTxt);
	//internal nds names
	fprintf( inifile, "nds_internal_names = %d\n", settings -> ndsInternalName);
	//24 hour clock
	fprintf( inifile, "24_hour_clock = %d\n", settings -> clock24Hour);
	//auto show favs
	fprintf( inifile, "auto_show_favorites = %d\n", settings -> bootFavs);
	//boot song
	fprintf( inifile, "boot_song = %s\n", settings -> bootSong);
	//nds icon transparency
	fprintf( inifile, "nds_icon_transparency = %d\n", settings -> ndsTrans);
	
    //write cpu settings
    fprintf( inifile, "\n");
    fprintf( inifile, "[CPU]\n");
	fprintf( inifile, "default = %d\n", settings -> cpu_default);
	fprintf( inifile, "media_player = %d\n", settings -> cpu_media);
	fprintf( inifile, "bg_music = %d\n", settings -> cpu_bgmusic);
	fprintf( inifile, "sleep = %d\n", settings -> cpu_sleep);
	fprintf( inifile, "favorites = %d\n", settings -> cpu_favs);
	fprintf( inifile, "favorites_music = %d\n", settings -> cpu_favs_bgmusic);
	fprintf( inifile, "nzip_decompression = %d\n", settings -> cpu_nzip_decompress);

	//write system settings
	fprintf( inifile, "\n");
    fprintf( inifile, "[System]\n");
	fprintf( inifile, "copy_buffer_length = %d\n", settings -> copy_buf_len);
	fprintf( inifile, "copy_display_update = %d\n", settings -> copy_display_update);
	fprintf( inifile, "top_display_refresh = %d\n", settings->topscreen_refresh);
	
	fclose(inifile);
	return 1;
	
	err:
		return 0;
}


void Change_Brightness(void)
{
	//read settings
	FBSETTINGS tempset;
	tempset = Settings_Read_Ini(Main_Browser, &UI_Style, GlobalPaths.Settings_ini, 1);		
	if(tempset.brightness < 4)
		tempset.brightness++;
	if(tempset.brightness > 3)
		tempset.brightness = 0;
		
	mdelay(20);
	ds2_setBrightness(tempset.brightness);
	Settings_Write_Ini(&tempset, GlobalPaths.Settings_ini);
	memcpy(&Main_Settings, &tempset, sizeof(FBSETTINGS));
}

inline int Get_CpuClockVal(int clock)
{
	int i = 0;
	for( i = 0; i < 14; i++)
	{
		if(cpu[i] == clock)
			return i;
	}
	return 0;
}

inline int Get_CpuClock(int val)
{
	return cpu[val];
}

void Change_Cpu(int menu, FBSETTINGS * settings)
{
	
	int cpuClock = 0, cpuVal = 0;
	switch(menu)
	{
		case SETTINGS_CPU_DEFAULT:
			cpuClock = settings -> cpu_default;
		break;
		case SETTINGS_CPU_MUSIC:
			cpuClock = settings -> cpu_media;
		break;
		case SETTINGS_CPU_BGPLAY:
			cpuClock = settings -> cpu_bgmusic;
		break;
		case SETTINGS_CPU_SLEEP:
			cpuClock = settings -> cpu_sleep;
		break;
		case SETTINGS_CPU_FAVS:
			cpuClock = settings -> cpu_favs;
		break;
		case SETTINGS_CPU_FAVS_BGMUSIC:
			cpuClock = settings -> cpu_favs_bgmusic;
		break;
		case SETTINGS_CPU_NZIP:
			cpuClock = settings -> cpu_nzip_decompress;
		break;
	}	
	cpuVal = Get_CpuClockVal(cpuClock);
	if(cpuVal_old != menu)
	{
		cpuVal_old = menu;
		ds2_setCPUclocklevel(cpuVal);
		mdelay(50);
	}
}



/*==========================================================================
Settings Language
==========================================================================*/
typedef enum
{
	LANG_MAIN_TITLE,
	LANG_MAIN_DISP,
	LANG_MAIN_SKIN,
	LANG_MAIN_CPU,
	LANG_MAIN_CLOSE,
	
	LANG_DISP_TITLE,
	LANG_DISP_HIDFAV,
	LANG_DISP_LASTDIR,
	LANG_DISP_SHOWEXT,
	LANG_DISP_HIDFAVTXT,
	LANG_DISP_INTERNALNDS,
	LANG_DISP_24HCLOCK,
	LANG_DISP_AUTOFAVS,
	LANG_DISP_NDSTRANS,
	
	LANG_CPU_TITLE,
	LANG_CPU_DEFAULT,
	LANG_CPU_MP,
	LANG_CPU_BGM,
	LANG_CPU_SLEEP,
	LANG_CPU_FAVS,
	LANG_CPU_FAVSMUSIC,
	LANG_CPU_NZIP,
	
	LANG_SKINPREV_TITLE,
	LANG_SKINPREV_MSG0,
	LANG_SKINPREV_MSG1,
	LANG_SKINPREV_BLANK,
	LANG_SKINPREV_MSG2,
	LANG_SKINPREV_MSG3,
	
	LANG_SKIN_TITLE,
	LANG_COUNT,
}settings_language_enum;
struct _language Settings_MSGS;



/*==========================================================================
Settings Main window
==========================================================================*/
void Create_Settings_Menu(GUI *skinInfo, windowObj *window, u16 * screen)
{
	int bufcount = LANG_MAIN_TITLE;
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "main_window_title", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "main_msg0", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "main_msg1", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "main_msg2", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "main_msg3", NULL);

	char filename[MAX_PATH];
	sprintf(filename, "%s/skin.ini", UI_Style.Current_Skin);
	
	guigfx * GFX = &skinInfo->gfx;
	
	Set_Window_Font (window, &GFX->Font);
	Load_UniWindow_Ini(window, "Window Settings", Settings_MSGS.buffer[LANG_MAIN_TITLE], filename);
    
 	//dynamically size menu
	int old_width  = BAG_UniGetStrWd(Settings_MSGS.buffer[LANG_MAIN_TITLE], window->font);
    int old_icoWidth = 0, old_icoHeight = 0;
	int j = 0;
	for (j = 0; j < SETTINGS_END; j++)
	{
        //get widest string
		int width = BAG_UniGetStrWd(Settings_MSGS.buffer[LANG_MAIN_DISP + j], window->font);
		if( width > old_width)
			old_width = width;
            
        //get the widest icon
        int ico_width = skinInfo->gfx.Main_Settings[j]->Width;
        if(ico_width > old_icoWidth)
            old_icoWidth = ico_width;
            
        //get tallest icon   
        int ico_height = skinInfo->gfx.Main_Settings[j]->Height;
        if(ico_width > old_icoHeight)
            old_icoHeight = ico_height + 1;        
	}

		
    int barHt = 16;
   
    int txtheight = BAG_GetCharHt('g', window->font);
     if(old_icoHeight < txtheight)
        old_icoHeight = txtheight + 1;
        
	int boxwd = old_width + old_icoWidth + 8 +  BAG_GetCharWd(' ', window->font);
	int boxht = ((SETTINGS_END) * old_icoHeight) + barHt + txtheight;   
    
	Set_Window_Dim2(window, barHt, skinInfo->gfx.SideBar->Width, 0, boxwd, boxht);
    Create_Window_Scrn(window, DOWN_SCREEN);
}


void Draw_SettingsMain(u16 *Dest, windowObj * window, GUI * ui, int *sub_window_y, int selected)
{
    guigfx * GFX = &ui->gfx;
    Draw_Window(window);
                            
    int i = 0;
    int txtheight = BAG_GetCharHt('g', window->font); 
    int txtwidth =  BAG_GetCharWd(' ', window->font);
	int icon_y = window->y + window->barHt + (txtheight>>1);
    //((window->y + window->height) / SETTINGS_END) + window->barHt + 1;
    int next_y = 0;
    for( i = 0; i < SETTINGS_END; i++)
    {	
        int icon_x = window->x + 4;
        next_y = (txtheight > GFX->Main_Settings[i]->Height) ? txtheight : GFX->Main_Settings[i]->Height; 
         
        BAG_DrawObj(GFX->Main_Settings[i], Dest, icon_x , icon_y );	
        int text_y = icon_y + (GFX->Main_Settings[i]->Height >> 1) - (txtheight>>1);
        int text_x = icon_x + GFX->Main_Settings[i]->Width + txtwidth;
        
        u16 Color = window->FontCol;
        if(selected == i)
		{
            Color = window->HilightTxtCol;
			*sub_window_y = text_y;
		}
        
        BAG_UniBmpPrintScrn(Dest, text_x , text_y, window->x + window->width, icon_y, Settings_MSGS.buffer[LANG_MAIN_DISP + i], window->font, Color,-1);	
        
        icon_y += next_y + 1;
    }
    Flip_Screen_Fun(DOWN_SCREEN, 2);
} 
    
/*==========================================================================
Display settings window
==========================================================================*/
void Create_Settings_Display(GUI *skinInfo, windowObj * oldwindow, windowObj *window, u16 * screen, int y)
{
	int bufcount = LANG_DISP_TITLE;
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "display_title", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "display_msg0", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "display_msg1", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "display_msg2", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "display_msg3", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "display_msg4", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "display_msg5", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "display_msg6", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "display_msg7", NULL);
	char filename[MAX_PATH];
	sprintf(filename, "%s/skin.ini", UI_Style.Current_Skin);
	
	guigfx * GFX = &skinInfo->gfx;
	
	Set_Window_Font (window, &GFX->Font);
	Load_UniWindow_Ini(window, "Sub Window Settings", Settings_MSGS.buffer[LANG_DISP_TITLE], filename);
    
 	//dynamically size menu
	int old_width  = BAG_UniGetStrWd( Settings_MSGS.buffer[LANG_DISP_TITLE], window->font);
	int j = 0;
	for (j = 0; j < SETTINGS_FILE_END; j++)
	{
        //get widest string
		int width = BAG_UniGetStrWd(Settings_MSGS.buffer[LANG_DISP_HIDFAV + j], window->font);
		if( width > old_width)
			old_width = width;     
	}

		
    int barHt = 16;
   
    int txtheight = BAG_GetCharHt('g', window->font);
    int icoheight = (txtheight > GFX->BoxOn->Height) ? txtheight : GFX->BoxOn->Height;
        
	//determine y coords for window
	int oldWindowPosY =y - (txtheight >> 1);
	int boxht = ((SETTINGS_FILE_END) * icoheight) + barHt + (icoheight >>1);   
	if(boxht > SCREEN_HEIGHT)
		boxht = SCREEN_HEIGHT;
		
    int box_y = (boxht < SCREEN_HEIGHT) ? oldWindowPosY: 0;
	
	//dtermine x coords for window
	int oldWindowPosX = oldwindow->x + oldwindow->width;
	int boxwd = old_width + GFX->BoxOn->Width + 8 +  BAG_GetCharWd(' ', window->font);
    int box_x = ((SCREEN_WIDTH - oldWindowPosX) > boxwd) ? oldWindowPosX + 1: SCREEN_WIDTH - boxwd;
    
	Set_Window_Dim2(window, barHt, box_x, box_y, boxwd, boxht);
    Create_Window_Scrn(window, DOWN_SCREEN);
}
    

void Draw_SettingsDisp(u16 *Dest, windowObj * window, GUI * ui, FBSETTINGS* settings, int selected, int input)
{
    guigfx * GFX = &ui->gfx;
    Draw_Window(window);
                            
    int i = 0;
    int icon_y = window->y + window->barHt + 1;
    int txtheight = BAG_GetCharHt('g', window->font); 
    
    BMPOBJ *CheckBox[SETTINGS_FILE_END];

    //collect all the options
    int *OPTIONS[SETTINGS_FILE_END];
	OPTIONS[SETTINGS_FILE_HIDEFAVFILES] = (int*)&settings->CombineFavsHidFiles;
    OPTIONS[SETTINGS_FILE_OPENLASTDIR] = (int*)&settings->lastfolder;
    OPTIONS[SETTINGS_FILE_SHOWEXT] = (int*)&settings->extensions;
	OPTIONS[SETTINGS_FILE_HIDEFAVSTXT] = (int*)&settings->favs_hideTxt;
	OPTIONS[SETTINGS_NDS_INTERNALNAMES] = (int*)&settings->ndsInternalName;
	OPTIONS[SETTINGS_CLOCK_24HOUR] = (int*)&settings->clock24Hour;
    OPTIONS[SETTINGS_AUTO_FAVS] = (int*)&settings->bootFavs;
	OPTIONS[SETTINGS_NDS_TRANS] = (int*)&settings->ndsTrans;
	
    int next_y = 0;
    for( i = 0; i < SETTINGS_FILE_END; i++)
    {	
        //draw the icon
        int icon_x = window->x + window->width - GFX->BoxOn->Width;
        next_y = (txtheight > GFX->BoxOn->Height) ? txtheight : GFX->BoxOn->Height; 
        
        u16 Color = window->FontCol;
        if(selected == i)
        {
            Color = window->HilightTxtCol;
            if(input)
            {
                if((*OPTIONS[i]) < 1)
				{
                    (*OPTIONS[i])++;
					if(i == SETTINGS_FILE_HIDEFAVFILES)
					{
						Main_Settings.CombineFavsHidFiles =  (*OPTIONS[i]);
						RefreshMainLists();
					}
					
				}
                else
				{
                    (*OPTIONS[i]) = 0;
					if(i == SETTINGS_FILE_HIDEFAVFILES)
					{
						//ReadFavoritesList();
						Main_Settings.CombineFavsHidFiles =  (*OPTIONS[i]);
						RefreshMainLists();
					}

				}
            }
        }
        
        CheckBox[i] = GFX->BoxOff;
        if(*OPTIONS[i])
            CheckBox[i] = GFX->BoxOn;
            
        BAG_DrawObj(CheckBox[i], Dest, icon_x , icon_y );	

        int text_y = icon_y + (GFX->BoxOn->Height >> 1) - (txtheight>>1);
        int text_x = window->x + 4;
        
        
        BAG_UniBmpPrintScrn(Dest, text_x , text_y, window->x + window->width, icon_y, Settings_MSGS.buffer[LANG_DISP_HIDFAV + i], window->font, Color,-1);	
        
        icon_y += next_y + 1;
    }
    Flip_Screen_Fun(DOWN_SCREEN, 2);
} 
     
 /*==========================================================================
Display settings window
==========================================================================*/  
void Skin_Preview_Window(const char *newSkin)
{
	int bufcount = LANG_SKINPREV_TITLE;
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "skin_preview_title", NULL);
	
		
	char skin_ini[MAX_PATH];
	memset(skin_ini, 0, sizeof(skin_ini));
	sprintf(skin_ini, "%s/%s/skin.ini", GlobalPaths.Skin_Root_Folder, newSkin);
	
	char ini_buf[MAX_PATH];
	memset(ini_buf, 0, sizeof(ini_buf));
	
	char tempbuf[MAX_PATH];
	memset(tempbuf, 0, sizeof(tempbuf));
	//skin name
	ini_gets("Skin Info","Skin_Name", "no_name", ini_buf, MAX_PATH, skin_ini);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "skin_preview_msg0", ini_buf);
	
	//get authors
	ini_gets("Skin Info","Creator", "no_creator", ini_buf, MAX_PATH, skin_ini);
	sprintf(tempbuf, "%s", ini_buf);
	utf8_to_unicode16(tempbuf, Settings_MSGS.buffer[bufcount++]);
	bufcount++;//blank line
	//instructions
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "skin_preview_msg1", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "skin_preview_msg2", NULL);
	Unicode_Notification_Window(Settings_MSGS.buffer[LANG_SKINPREV_TITLE], (u16**)&Settings_MSGS.buffer[LANG_SKINPREV_MSG0], bufcount - LANG_SKINPREV_TITLE - 1, 1);
}


BMPOBJ prevTop, prevBottom;
int Skin_Preview(const char *newSkin)
{
	SetScreenCap(2);
	BAG_RmSync(&Render_TopScreen);
	BAG_RmSync(&Background_Player);
	BAG_UpdateIN();
	
	
	char file[MAX_PATH];
	memset(file, 0, sizeof(file));
	
	sprintf(file, "%s/%s/preview", GlobalPaths.Skin_Root_Folder, newSkin);
	int err = Load_Image(file, &prevBottom, 256, 192);
	if(err != NO_ERR)
		return 0;
	
	BAG_SetObjFrame( &prevBottom, FRAME_VERT, 1);

	BAG_Display_CopyObj(&prevBottom, &prevTop);
	BAG_SetObjFrame( &prevTop, FRAME_VERT, 0);

	void refresh_preview(void)
	{
		BAG_FastDrawObj(&prevBottom, down_screen_addr, 0 , 0);	
		BAG_FastDrawObj(&prevTop, up_screen_addr, 0 , 0);
		Flip_Screen_Fun(UP_SCREEN, 2);
		Flip_Screen_Fun(DOWN_SCREEN, 2);
	}
	refresh_preview();
	Skin_Preview_Window(newSkin);
	refresh_preview();
	
	int preview_loop = 1;
	while(preview_loop)
	{
	
		if(Pad.Newpress.Y)
			preview_loop = 0;
	
		BAG_Update();
	}
	BAG_SetSync(&Render_TopScreen);
	BAG_SetSync(&Background_Player);
	BAG_DeleteObj(&prevTop);
	BAG_DeleteObj(&prevBottom);
	SetScreenCap(1);
	return 1;
}

void Create_Settings_SkinList(GUI *skinInfo, windowObj * oldwindow, windowObj *window, u16 * screen, FileBrowser *skins, int y)
{
    FB_ScanDir( GlobalPaths.Skin_Root_Folder, skins, 0, NULL, 1);


	char filename[MAX_PATH];
	memset(filename, 0, sizeof(filename));
	sprintf(filename, "%s/skin.ini", UI_Style.Current_Skin);
	
	guigfx * GFX = &skinInfo->gfx;
	
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[LANG_SKIN_TITLE], "Settings", "skin_title", NULL);
	Set_Window_Font (window, &GFX->Font);
	Load_UniWindow_Ini(window, "Sub Window Settings", Settings_MSGS.buffer[LANG_SKIN_TITLE], filename);
    
 	//dynamically size menu
	int old_width  = BAG_UniGetStrWd(Settings_MSGS.buffer[LANG_SKIN_TITLE], window->font);
	int j = 0;
	for (j = 0; j < skins->Dir_Count; j++)
	{
        //get widest string
		int width = BAG_GetStrWd((unsigned char*)skins->Dir[j].Name, window->font);
		if( width > old_width)
			old_width = width;     
	}

		
    int barHt = 16;
   
    int txtheight = BAG_GetCharHt('g', window->font) + 1;
        
	int boxwd = old_width + 8;
    int truesize =  (barHt) + (txtheight *  (skins->Dir_Count - 1)) + txtheight;
	int boxht = (truesize < SCREEN_HEIGHT) ? truesize : SCREEN_HEIGHT;
    
	int oldWindowPosX = oldwindow->x + oldwindow->width;
    int box_x = ((SCREEN_WIDTH - oldWindowPosX) > boxwd) ? oldWindowPosX + 1: SCREEN_WIDTH - boxwd;

	int oldWindowPosY =y - (txtheight >> 1);
    int box_y = (boxht < SCREEN_HEIGHT) ? oldWindowPosY: 0;
    
	Set_Window_Dim2(window, barHt, box_x, box_y, boxwd, boxht);
    Create_Window_Scrn(window, DOWN_SCREEN);
}
    
int skin_oldSelected = 0, skin_curScroll = 0, skin_reload = 0;
char PreviewSkin[MAX_PATH];
void Draw_SettingsSkin(u16 *Dest, windowObj * window, GUI * ui, FileBrowser* fb, char *skinName,  int *selected, int input)
{
    Draw_Window(window);
    int txtheight = BAG_GetCharHt('g', window->font); 
	
    int i = 0, dir_hide = 1, list_scroll = 0;
    int text_y = window->y + window->barHt + (txtheight>>1);
    int text_x = window->x + 4;

    
    int ItemsPerScreen = (window->height - window->barHt -1) / (txtheight + 1);

    list_scroll = skin_curScroll;
    if(*selected < dir_hide)
        (*selected) = dir_hide;
     
    //list scrolling
    if(*selected > skin_oldSelected && *selected > ItemsPerScreen)
    {
        list_scroll++; 
        skin_curScroll = list_scroll;
        skin_oldSelected = *selected;
    }
    
    if(*selected < skin_oldSelected && *selected < (fb->Dir_Count - ItemsPerScreen))
    {
        list_scroll--; 
        skin_curScroll = list_scroll;
        skin_oldSelected = *selected;
    }
    
    if(skin_curScroll < 0)
        skin_curScroll = 0;
    else if(skin_curScroll + ItemsPerScreen > fb->Dir_Count)
        skin_curScroll = fb->Dir_Count - ItemsPerScreen;
        
	u16 Unicode[MAX_PATH];
	memset(Unicode, 0, sizeof(Unicode));
   //draw the list
    int next_y = txtheight + 1;
    for( i = 0; i < ItemsPerScreen; i++)
    {	
        int File = i + dir_hide + skin_curScroll;
        //draw the icon
		if(&fb->Dir[File] == NULL)//no file allocated
			break;
			
        u16 Color = window->FontCol;

        if(!strcasecmp(fb->Dir[File].Name, skinName))
            Color = window->ExtraCol;
            
        if(*selected == File)
            Color = window->HilightTxtCol;

		utf8_to_unicode16(fb->Dir[File].Name, Unicode);
        BAG_UniBmpPrintScrn(Dest, text_x , text_y, window->x + window->width, text_y, Unicode, window->font, Color, -2);	
        text_y+= next_y;
    }
    if(input == 1)
    {
      strcpy(skinName,  fb->Dir[*selected].Name);
      skin_reload++;
    }
	else if(input == 2)
		strcpy(PreviewSkin, fb->Dir[*selected].Name);

    Flip_Screen_Fun(DOWN_SCREEN, 2);
} 

/*==========================================================================
CPU Settings window
==========================================================================*/
void Create_Settings_CPU(GUI *skinInfo, windowObj * oldwindow, windowObj *window, u16 * screen, int y)
{
	int bufcount = LANG_CPU_TITLE;
	
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "cpu_title", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "cpu_msg0", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "cpu_msg1", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "cpu_msg2", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "cpu_msg3", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "cpu_msg4", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "cpu_msg5", NULL);
	Language_FileReadStr(GlobalPaths.Language, Settings_MSGS.buffer[bufcount++], "Settings", "cpu_msg6", NULL);

	char filename[MAX_PATH];
	sprintf(filename, "%s/skin.ini", UI_Style.Current_Skin);
	
	guigfx * GFX = &skinInfo->gfx;
	
	Set_Window_Font (window, &GFX->Font);
	Load_UniWindow_Ini(window, "Window Settings", Settings_MSGS.buffer[LANG_CPU_TITLE], filename);
    
 	//dynamically size menu
	int old_width  = BAG_UniGetStrWd(Settings_MSGS.buffer[LANG_CPU_TITLE], window->font);
	int j = 0;
	for (j = 0; j < SETTINGS_CPU_END; j++)
	{
        //get widest string
		int width = BAG_UniGetStrWd(Settings_MSGS.buffer[LANG_CPU_DEFAULT + j ], window->font);
		if( width > old_width)
			old_width = width;       
	}

    int barHt = 16;
   
    int txtheight = BAG_GetCharHt('g', window->font);
        
	//determine y coords for window
	int oldWindowPosY =y - (txtheight >> 1);
	int boxht = ((SETTINGS_CPU_END) * txtheight) + barHt + (txtheight >>1) + txtheight;   
	if(boxht > SCREEN_HEIGHT)
		boxht = SCREEN_HEIGHT;
		
    int box_y = (boxht < SCREEN_HEIGHT) ? oldWindowPosY: 0;
	
	//dtermine x coords for window
	int oldWindowPosX = oldwindow->x + oldwindow->width;
	int boxwd = old_width + GFX->BoxOn->Width + 8 +  BAG_GetCharWd(' ', window->font);
    int box_x = ((SCREEN_WIDTH - oldWindowPosX) > boxwd) ? oldWindowPosX + 1: SCREEN_WIDTH - boxwd;
    
	Set_Window_Dim2(window, barHt, box_x, box_y, boxwd, boxht);
    Create_Window_Scrn(window, DOWN_SCREEN);
}


void Draw_SettingsCPU(u16 *Dest, windowObj * window, GUI * ui, FBSETTINGS* settings, int selected, int input)
{
    Draw_Window(window);
                        
    int i = 0;
    int txtheight = BAG_GetCharHt('g', window->font); 
    int text_y = window->y + window->barHt + (txtheight>>1);
    int text_x = window->x + 4;
	
    int next_y = 0;
    u16 *CPUVALS[SETTINGS_CPU_END];
    CPUVALS[SETTINGS_CPU_DEFAULT] = (u16*)&settings -> cpu_default;
    CPUVALS[SETTINGS_CPU_MUSIC] = (u16*)&settings -> cpu_media;
    CPUVALS[SETTINGS_CPU_BGPLAY] = (u16*)&settings -> cpu_bgmusic;
    CPUVALS[SETTINGS_CPU_SLEEP] = (u16*)&settings -> cpu_sleep;
	CPUVALS[SETTINGS_CPU_FAVS] = (u16*)&settings -> cpu_favs;
	CPUVALS[SETTINGS_CPU_FAVS_BGMUSIC] = (u16*)&settings -> cpu_favs_bgmusic;
	CPUVALS[SETTINGS_CPU_NZIP] =  (u16*)&settings -> cpu_nzip_decompress;

    char cpuText[64];
	memset(cpuText, 0, sizeof(cpuText));
	u16 finalText[64], convertText[64];
	
    for( i = 0; i < SETTINGS_CPU_END; i++)
    {	
        next_y = txtheight + 1; 
        u16 Color = window->FontCol;
        if(selected == i)
		{
            Color = window->HilightTxtCol;
            
			if(input)
			{
				int tempval = Get_CpuClockVal(*CPUVALS[selected]);
				tempval+=input;
				if(tempval > 13)
					tempval = 1;
				if(tempval < 1)
					tempval = 13;
					
					
				while(Get_CpuClock(tempval) == 0 && tempval < 14)
					tempval+=input;
					
				*CPUVALS[selected] = Get_CpuClock(tempval);
				if(selected == SETTINGS_CPU_DEFAULT)//options uses the default cpu
					Change_Cpu(selected, settings);
			}
		}
        
		wcscpy(finalText, Settings_MSGS.buffer[LANG_CPU_DEFAULT + i ]);
		
		sprintf(cpuText, " ~C[%d]%d",window->ExtraCol, *CPUVALS[i]);
		utf8_to_uni16( cpuText, convertText);
		
		wcscat(finalText, convertText);
		
        BAG_UniBmpPrintScrn(Dest, text_x , text_y, window->x + window->width, text_y, finalText, window->font, Color,-1);	
        
        text_y += next_y;
    }
    Flip_Screen_Fun(DOWN_SCREEN, 2);
} 
/*==========================================================================
Settings handling
==========================================================================*/
void Settings_Main(UI_FB * gui, GUI *skinInfo)
{
	if(Language_InitBuffers(&Settings_MSGS, LANG_COUNT) != 1)
		return;
		
	SetScreenCap(1);
	
    //create window
	Render_DirGfx(down_screen_addr);
	windowObj Settings_Window, //main window
                    Sub_Window; //reusable sub menu
	Create_Settings_Menu(skinInfo, &Settings_Window, down_screen_addr);

	//read settings
	char filepath[MAX_PATH];
	memset(filepath, 0, sizeof(filepath));
	strcpy(filepath, GlobalPaths.Settings_ini);
	
	FBSETTINGS tempset;
	tempset = Settings_Read_Ini(gui, skinInfo, filepath, 1);
	memcpy(&Main_Settings, &tempset, sizeof(FBSETTINGS));
    
    //prepare skin listing
    FileBrowser *Skin_List = FB_Init (0, 64);
	if(Skin_List == NULL)
		return;
    
    typedef enum
    {
        sMain,
        sDisp,
        sSkin,
        sCpu,
        sEnd,
    }settings_sections;
    
	int settings_loop = 1, selected = 0, old_selected = -1;
    int selected_main = 0;
    int Menu = sMain , old_menu = 0;
    int menu_selections = 0, menu_input = 0;
    windowObj * Cur_Window = NULL;
	
	int sub_Window_Y = 0;
    
    void Exit_CurWindow(void)
    {
        switch(Menu)
        {
            case sMain:
                settings_loop = 0;
            break;
            default:
                Menu = sMain;
                Delete_Window(&Sub_Window);
                Render_DirGfx(down_screen_addr);
                //Flip_Screen_Fun(DOWN_SCREEN, 0);
                 Render_DirGfx(down_screen_addr);
                selected = selected_main;
            break;
        }
    }
    BAG_UpdateIN();
	while(settings_loop)
	{
        //determine the number of options per menu section
        switch(Menu)
        {
            case sMain:
                menu_selections = SETTINGS_END;
                Cur_Window = &Settings_Window;
            break;
            case sDisp:
                menu_selections = SETTINGS_FILE_END;
                Cur_Window = &Sub_Window;
            break;
            case sSkin:
                menu_selections = Skin_List->Dir_Count;
                Cur_Window = &Sub_Window;
            break;
            case sCpu:
				menu_selections = SETTINGS_CPU_END;
				Cur_Window = &Sub_Window;
			break;
        }
        
         if(old_selected == selected && Stylus.DblClick)
            menu_input++;
    
		if(Stylus.Newpress || Stylus.Held)
        {
            switch(Menu)
            {
                default:
                    selected = ((Stylus.Y - Cur_Window->y - Cur_Window->barHt - 1) * menu_selections ) / (Cur_Window->height - Cur_Window->barHt);
                    if(Stylus.Newpress && !BAG_StylusZone(Cur_Window->x, Cur_Window->y, Cur_Window->x + Cur_Window->width, Cur_Window->y + Cur_Window->height))
                        Exit_CurWindow();
                break;
            }
        }
		if(Pad.Newpress.Right)
		{
			if(Menu == sCpu)
				menu_input = 1;
		}
		if(Pad.Newpress.Left)
		{
			if(Menu == sCpu)
				menu_input = -1;
		}
		if(Pad.Newpress.Y)
		{
			if(Menu == sSkin)
				menu_input = 2;
		
		}
        
        if(Pad.Newpress.Down)
        {
            if(selected < menu_selections  - 1)
                selected++;
        }
        if(Pad.Newpress.Up)
        {
            if(selected > 0)    
                selected--;
        }
        
        if(Pad.Newpress.B)
            Exit_CurWindow();

        if(Pad.Newpress.A || (Stylus.DblClick && old_selected == selected))
        {
            switch(Menu)
            {
                case sMain://if on main menu, then A button is selecting a new menu
					menu_input = 0;
                    selected_main =selected;
                    switch(selected + 1)
                    {
                        case sDisp:
                            Draw_SettingsMain(down_screen_addr, &Settings_Window, skinInfo, &sub_Window_Y, selected);
                            Create_Settings_Display(skinInfo, &Settings_Window,&Sub_Window, down_screen_addr, sub_Window_Y);
                            Menu = sDisp;
                            selected = 0;
							BAG_UpdateIN();
                        break;
                        case sSkin:
                            Draw_SettingsMain(down_screen_addr, &Settings_Window, skinInfo, &sub_Window_Y, selected);
                            Create_Settings_SkinList(skinInfo, &Settings_Window, &Sub_Window, down_screen_addr, Skin_List, sub_Window_Y);
                            Menu = sSkin;
                            selected = 0;
                            skin_curScroll = 0;
							BAG_UpdateIN();
                        break;
                        case sCpu:
							Draw_SettingsMain(down_screen_addr, &Settings_Window, skinInfo, &sub_Window_Y, selected);
							Create_Settings_CPU(skinInfo, &Settings_Window, &Sub_Window, down_screen_addr, sub_Window_Y);
							Menu = sCpu;
							selected = 0;
							BAG_UpdateIN();
						break;
                        default:
                           settings_loop = 0;
						   BAG_UpdateIN();
                        break;
                    }
                break;
                case sDisp:
                    menu_input++;
                break;
                case sSkin:
                    menu_input++;
                break;
                case sCpu:
					menu_input++;
				break;
            }
        }
        
        //update interface
        if(old_selected != selected || old_menu != Menu || menu_input != 0 || MusicPlayer->minimize)
        {
			if(MusicPlayer->minimize)
				 Draw_SettingsMain(down_screen_addr, &Settings_Window, skinInfo, &sub_Window_Y, selected_main);
            switch(Menu)
            {
                case sMain:
                   Draw_SettingsMain(down_screen_addr, &Settings_Window, skinInfo, &sub_Window_Y, selected);
                break;
                case sDisp:
					Draw_SettingsDisp(down_screen_addr, &Sub_Window, skinInfo, &tempset, selected, menu_input);
                    menu_input = 0;
                break;
                case sSkin:
                   Draw_SettingsSkin(down_screen_addr, &Sub_Window, skinInfo, Skin_List, tempset.skin_name,&selected, menu_input);
				   if(menu_input == 2 && Skin_Preview(PreviewSkin))
				   {
						Render_DirGfx(down_screen_addr);
						Draw_SettingsMain(down_screen_addr, &Settings_Window, skinInfo, &sub_Window_Y, selected_main);
						Draw_SettingsSkin(down_screen_addr, &Sub_Window, skinInfo, Skin_List, tempset.skin_name,&selected, 0);
					}
						
                   menu_input = 0;
                   
                   if(skin_reload)
                   {
                        Settings_Write_Ini(&tempset, filepath);//write settings
                        //delete windows
                        Delete_Window(&Sub_Window);
                        Delete_Window(&Settings_Window);
                        //reload interface
                        ReloadUI(&UI_Style, Main_Browser);
                        Create_Settings_Menu(skinInfo, &Settings_Window, down_screen_addr);
                        Draw_SettingsMain(down_screen_addr, &Settings_Window, skinInfo, &sub_Window_Y, selected_main);
						Create_Settings_SkinList(skinInfo, &Settings_Window, &Sub_Window, down_screen_addr, Skin_List, sub_Window_Y);
						Draw_SettingsSkin(down_screen_addr, &Sub_Window, skinInfo, Skin_List, tempset.skin_name,&selected, menu_input);
                        //Menu = sMain;
                        skin_reload = 0;
                    }
                   
               break;
				case sCpu:
					Draw_SettingsCPU(down_screen_addr, &Sub_Window, skinInfo, &tempset, selected, menu_input);
					menu_input = 0;
				break;
            }
            old_selected = selected;
            old_menu = Menu;
        }
        
		BAG_Update();
	}
	Language_CleanBuffers(&Settings_MSGS);
	SetScreenCap(0);
	//save settings and clean up
	memcpy(&Main_Settings, &tempset, sizeof(FBSETTINGS));
	Settings_Write_Ini(&tempset, filepath);
    FB_Free (Skin_List);
	Delete_Window(&Settings_Window);
	gui->Render_FB(gui);
}

