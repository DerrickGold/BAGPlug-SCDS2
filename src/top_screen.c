#include <libBAG.h>
#include "supportedFiles.h"
#include "draw.h"
#include "interface.h"
#include "fb_api.h"
#include "minIni.h"
#include "settings.h"
#include "music_player.h"
#include "ndsheader.h"
#include "menu_files.h"
#include "language.h"
#include "sd_card.h"


int Top_ScreenUpdate = 0;
int ForceTopRefresh = 0;

void ForceTopScreenRefresh(void)
{
	Top_ScreenUpdate = Main_Settings.topscreen_refresh + 1;
	ForceTopRefresh = 1;
}

struct _language WeekDays;
int Load_DaysText(void)
{
	const s8 num_days = 7;
	Language_InitBuffers(&WeekDays, num_days);
	
	char keybuf[MAX_PATH];
	memset(keybuf, 0, sizeof(keybuf));
	
	int i = 0;
	for(i = 0; i < num_days; i++)
	{
		sprintf(keybuf, "day_msg%d", i);
		Language_FileReadStr(GlobalPaths.Language, WeekDays.buffer[i], "Days", keybuf, NULL);
	}
	return 1;
}

void Clear_DaysText(void)
{
	Language_CleanBuffers(&WeekDays);
}

int oldminute = 0;
char units[3][6] = {"bytes", "KB", "MB"};

void Display_SdSpace(char *banner, int txtY)
{
	BMP_FNT * font = &UI_Style.gfx.Font;
	//get sd card stats
	u64 total = 0, used = 0, free = 0;
	SD_getSpaceInfo(&used,  &total, &free);
	u8 usedUnit = 0, totalUnit = 0;
	int newUsed = SD_convertSize(used, &usedUnit);
	int newTotal = SD_convertSize(total, &totalUnit);
	
	sprintf(banner, "SD:%d %s/%d %s", newUsed, units[usedUnit], newTotal, units[totalUnit]);
	BAG_BmpPrintScrn(up_screen_addr, 0, txtY, 256, 16, (unsigned char*)banner, font, UI_Style.TopBar_TxtCol, -2);
}

void Top_Bar(void)
{
	BMPOBJ * GFX = UI_Style.gfx.TopBar;
	BMP_FNT * font = &UI_Style.gfx.Font;
	BAG_DrawObj(GFX, up_screen_addr, 0, 0);
	struct rtc Time;
	ds2_getTime(&Time);
	
	char period[2];
	memset(period, 0, sizeof(period));
	
	if(Main_Settings.clock24Hour == 0)
	{
		if(Time.hours > 11)
			strcpy(period, "PM");
		else
			strcpy(period, "AM");
			
		if(Time.hours > 12)
			Time.hours -= 12;
		else if(Time.hours == 0)
			Time.hours = 12;
	}
	
	char banner[64];
	u16 banner16[64];
	sprintf(banner," %d:%02d %s" ,Time.hours, Time.minutes, period);
	utf8_to_uni16( banner, banner16);
	memset(banner, 0, sizeof(banner));
	u16 finalDisp[64];
	wcscpy(finalDisp, WeekDays.buffer[Time.weekday]);
	wcscat(finalDisp, banner16);
	//global text Y
	int txtheight = BAG_GetCharHt('g', font);
	int TextStartY = ((GFX->Y + GFX->FrameHt)>>1) - (txtheight >> 1);
	
	
	//print day and time
	int TextStartX = BAG_GetStrWd((unsigned char*)banner, font);
	TextStartX = (GFX->X + TextStartX  + ((GFX->FrameWd  - TextStartX)>> 1)) - (BAG_UniGetStrWd(finalDisp, font) >> 1);
	
	BAG_UniBmpPrintScrn(up_screen_addr, TextStartX , TextStartY, 256, 16, finalDisp, font, UI_Style.TopBar_TxtCol, -2);		
	oldminute = Time.minutes;
}


struct _coverarts CoverArt;

void CoverArt_UseFavorites(u8 yes)
{
	CoverArt.ExternalEntry = yes;
}

inline void CoverArt_ExternFile(const char *file)
{
	//get file name
	memset(CoverArt.currentFile, 0, sizeof(CoverArt.currentFile));
	FB_GetFileFromPath(CoverArt.currentFile, file);
	
	//get file path
	memset(CoverArt.currentPath, 0, sizeof(CoverArt.currentPath));
	FB_GetFilePath(CoverArt.currentPath, file);
	strcat(CoverArt.currentPath, "/");
}


int Init_CoverArt(void)
{
	int rtrnval = 1;
	
	//loading a cover from a file in the main browser
	if(CoverArt.ExternalEntry == 0)
	{
		struct FILEINFO * File = NULL;
		int file = Main_Browser->State.File_Selected;
		FileBrowser * fb = Main_Browser->FB_Dir;
		int dir_count = fb->Dir_Count;
		
		if(file < dir_count)//is folder
			goto ERROR;
		else
			File = &fb->File[file - dir_count];	
		
		strcpy(CoverArt.currentPath, fb->Current_Dir);
		strcpy(CoverArt.currentFile, File->Name);
	}
	//check if new directory has been entered
	if(strcasecmp(CoverArt.oldPath, CoverArt.currentPath))
	{	
		CoverArt.useFolderImage = 0;
		CoverArt.useLocal = 0;
		rtrnval = -1;//new directory;
		strcpy(CoverArt.oldPath, CoverArt.currentPath);
	}
	
	if(CoverArt.useFolderImage == 1)
		goto USECOVERS;
	
	//check if a new file is selected
	if(!strcasecmp(CoverArt.oldFile, CoverArt.currentFile))
		return rtrnval;
		
	strcpy(CoverArt.oldFile, CoverArt.currentFile);
	
	CoverArt.Loaded = 0;
	strcpy(CoverArt.coverExt, "none");
	
	//collect file extension for ini reading
	memset(CoverArt.fileExt, 0, sizeof(CoverArt.fileExt));
	FB_GetExt(CoverArt.currentFile, CoverArt.fileExt, EXT_NAME_LEN);
	
	//check if there is a local CoverArt.ini
	if(CoverArt.useLocal == 0)
	{
		memset(CoverArt.LocalIni, 0, sizeof(CoverArt.LocalIni));
		strcpy(CoverArt.LocalIni, CoverArt.currentPath);
		strcat(CoverArt.LocalIni, "/CoverArt.ini");		
		if(FB_FileExists(CoverArt.LocalIni))
			CoverArt.useLocal = 1;
		else
		{
			//check for folder.png
			strcpy(CoverArt.folderImg, CoverArt.currentPath);
			strcat(CoverArt.folderImg, "/folder.png");
			if(FB_FileExists(CoverArt.folderImg))
			{
				CoverArt.useFolderImage = 1;
				goto USECOVERS;
			}
			else
				CoverArt.useLocal = -1;
		}
	}
	
	if(CoverArt.useLocal == 1)
	{	
		ini_gets(CoverArt.fileExt,"folder", "~", CoverArt.coverPath, MAX_PATH, CoverArt.LocalIni);
		ini_gets(CoverArt.fileExt,"type", "none", CoverArt.coverExt, EXT_NAME_LEN, CoverArt.LocalIni);
	}
	//check global ini first
	if(CoverArt.useLocal == -1 && FB_FileExists(GlobalPaths.CoverArtIni))
	{
		ini_gets(CoverArt.fileExt,"type", "none", CoverArt.coverExt, EXT_NAME_LEN, GlobalPaths.CoverArtIni);
		ini_gets(CoverArt.fileExt,"folder", "~", CoverArt.coverPath, MAX_PATH, GlobalPaths.CoverArtIni);
	}


	//if a directory can't be read, then 
	if((!strcasecmp(CoverArt.coverPath, "~")) || (!_directory_check(CoverArt.coverPath)) || (!strcasecmp(CoverArt.coverExt, "none")))
		goto ERROR;


	int len = strlen(CoverArt.coverPath);
	if(CoverArt.coverPath[len] != '/' && CoverArt.coverPath[len - 1] != '/')
		strcat(CoverArt.coverPath, "/");
		
	USECOVERS:
	CoverArt.useCovers = 1;	
	return rtrnval;
	
	ERROR:
		CoverArt.useCovers = 0;
		return 0;	
}

void Load_CoverArt(void)
{
	//check if covers have been initiated properly
	if(CoverArt.useCovers == 0)
		return;
		
	if(CoverArt.Loaded == 1)//if cover is loaded, then go straight to drawing the cover
		goto DRAW_COVER;
		
	//load folder image if available
	if(CoverArt.useFolderImage == 1)
	{
		BAG_DeleteObj(&CoverArt.GFX);
		int err = BAG_LoadPng(CoverArt.folderImg, &CoverArt.GFX, 0, 0);
		if(err == NO_ERR)
			BAG_ResetFrameDim(&CoverArt.GFX);
		goto DRAW_COVER;
	}
	
	//final path to the cover to load	
	char CoverToLoad[MAX_PATH];
	memset(CoverToLoad, 0, sizeof(CoverToLoad));
	strcpy(CoverToLoad, CoverArt.coverPath);
		

	NDSHEADER header;
	memset(&header, 0, sizeof(NDSHEADER));
	
	//checking if file is an nds file
	s8 isNds = 0;
	if(!strcasecmp(CoverArt.fileExt, sfiles_exec[NDS_EXT]))//if there is an nds file
		isNds = 1;
	else if(!strcasecmp(CoverArt.fileExt, sfiles_exec[NZIP_EXT]))//if there is a compressed nds file
		isNds = 2;

	//if it is an nds file, then read the header for the game code
	if(isNds > 0)
	{
		//build the full path of the file, for checking nds header
		char FullPath[MAX_PATH];
		memset(FullPath, 0, sizeof(FullPath));
		
		strcpy(FullPath, CoverArt.currentPath);
		strcat(FullPath, "/");
		strcat(FullPath, CoverArt.currentFile);
	
		if(isNds == 1)
		{
			NDS_ReadTitle(FullPath, &header);
			if(header.type != ROM_CM)
				isNds = 0;	
		}
		else if(isNds == 2)
		{
			NZIP_ReadTitle(FullPath, &header);
			if(header.type != ROM_CM)
				isNds = 0;		
		}
	}
		
	if(isNds)
		strcat(CoverToLoad, header.game_code);
	else
		strcat(CoverToLoad, CoverArt.currentFile);
		
	//add extension to file to load
	strcat(CoverToLoad, ".");
	strcat(CoverToLoad, CoverArt.coverExt);
	
	//load the cover now
	BAG_DeleteObj(&CoverArt.GFX);
	int err = NO_ERR;
	
	CoverArt.Loaded = 1;
	
	if(!strcasecmp(CoverArt.coverExt, "png"))
		err = BAG_LoadPng(CoverToLoad, &CoverArt.GFX, 0, 0);
	else if(!strcasecmp(CoverArt.coverExt, "bmp"))
		err = BAG_LoadBmp(CoverToLoad, &CoverArt.GFX, 0, 0);
		
	if(err == NO_ERR)
		BAG_ResetFrameDim(&CoverArt.GFX);
	else
		return;
		
	
	
	int imagex = 0, imagey = 0;
	DRAW_COVER:
	
	imagex = (SCREEN_WIDTH >> 1) - (CoverArt.GFX.Width >>1);
	imagey = ((SCREEN_HEIGHT - UI_Style.gfx.TopBar->Height)>>1) - (CoverArt.GFX.Height>>1) + UI_Style.gfx.TopBar->Height;
	BAG_FastDrawObj(&CoverArt.GFX, up_screen_addr, imagex, imagey);
}


u16 * scraddr = NULL;

inline void Render_TopScreen(void)
{
	int blit = 0;
	int Covers = Init_CoverArt();
	if(Covers == -1)//new directory has been entered, 2 signifies new favorites cover
		ForceTopRefresh = 1;
		
	if(scraddr == down_screen_addr && Pad.key == 0)
		Top_ScreenUpdate++;
	else
		Top_ScreenUpdate = 0;
		
	if(Top_ScreenUpdate >=  Main_Settings.topscreen_refresh || ForceTopRefresh == 1)
	{//only update when the other screen isn't being updated
		Top_ScreenUpdate = 0;
		ForceTopRefresh = 0;
		blit = 1;
	}
	if(blit == 1 && Pad.key == 0 && scraddr == down_screen_addr)
	{
		BAG_FastDrawObj (UI_Style.gfx.BG[1], up_screen_addr, 0, 0);
		Top_Bar();
		Load_CoverArt();	
		BAG_FlipScreen(UP_SCREEN);
	}
	scraddr = down_screen_addr;
}
