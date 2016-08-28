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
#include "menu_buttons.h"
#include "fileops.h"
#include "read_txt.h"
#include "menu_files.h"
#include "language.h"
#include "screen_cap.h"

#define DTC_SETTINGS_COUNT 3
const char _settings_key[DTC_SETTINGS_COUNT][16] = {"patch", "cheat", "saveNum"};

void getDTCFile(char *outName, const char *inName)
{
	strcpy(outName, inName);
	
	int len = strlen(outName);
	while(len > 0 && outName[len] != '.')
		outName[len--] = '\0';
	strcat(outName, "dtc\0");

}

void EOS_Properties(const char *rom_file, int isCompressed)
{
	typedef enum{
		WINDOW_TITLE,
		MODE,
		CHEATS,
		SAVENUM,
		EXIT,
		CLEAN,
		PATCH,
		OFF,
		ON,
		MSGCOUNT,
	}removemsgs;
	
	struct _language msgs;
	memset(&msgs, 0, sizeof(struct _language));
	if(Language_InitBuffers(&msgs, MSGCOUNT) != 1)
		return;
		
	SetScreenCap(1);
	char section[64];
	strcpy(section, "EOS Settings");
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[WINDOW_TITLE], section, "window_title", NULL);BAG_UpdateAudio();
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[MODE], section, "settings_msg0", NULL);BAG_UpdateAudio();
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[CHEATS], section, "settings_msg1", NULL);BAG_UpdateAudio();
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[SAVENUM], section, "settings_msg2", NULL);BAG_UpdateAudio();
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[EXIT], section, "settings_msg3", NULL);BAG_UpdateAudio();
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[CLEAN], section, "settings_sub_msg0", NULL);BAG_UpdateAudio();
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[PATCH], section, "settings_sub_msg1", NULL);BAG_UpdateAudio();
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[OFF], section, "settings_sub_msg2", NULL);BAG_UpdateAudio();
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[ON], section, "settings_sub_msg3", NULL);BAG_UpdateAudio();
	
	int optslen[DTC_SETTINGS_COUNT + 1];
	optslen[0] = MAX(wcslen(msgs.buffer[CLEAN]), wcslen(msgs.buffer[PATCH]));
	optslen[1] = MAX(wcslen(msgs.buffer[OFF]), wcslen(msgs.buffer[ON]));
	optslen[2] = 2;//space and number
	optslen[3] = 0;
	
	//load nds icon
	BMPOBJ *_eos_icon = NULL;
	_eos_icon = calloc(1, sizeof(BMPOBJ));
	if(!_eos_icon)
		goto CLEANUP;
		
	BAG_CreateBlankObj( _eos_icon, 16, NDS_ICON_WIDTH, NDS_ICON_HEIGHT, NDS_ICON_WIDTH, NDS_ICON_HEIGHT);
	
	if(isCompressed == 0)
	{
		if(!NDS_ReadIcon(rom_file, _eos_icon))
			goto CLEANUP;
	}
	else if(isCompressed == 1)
	{
		if(!NZIP_ReadIcon(rom_file, _eos_icon))
			goto CLEANUP;	
	}
	
	struct _ini_file DTCFile;
	memset(&DTCFile, 0, sizeof(struct _ini_file));
	
	ini_File_Init(&DTCFile, DTC_SETTINGS_COUNT);
	int c = 0;
	for( c = 0; c < DTCFile.Key_Count; c++)
		DTCFile.Keys[c] = (char*)&_settings_key[c];
	
	//read the settings file
	char iniFile[MAX_PATH];
	memset(iniFile, 0, sizeof(iniFile));
	getDTCFile(iniFile, rom_file);
	
	
	if(ini_Read_File(iniFile, &DTCFile) != 1)
	{
		if(ini_Read_File(GlobalPaths.TemplateDTC, &DTCFile)!=1)
			goto CLEANUP;
			
		strcpy(DTCFile.Path, iniFile);
	}
		
	ini_Read_Settings(&DTCFile, "user info");
	int option_count = 	DTCFile.Key_Count + 1;
	//prepare window
	char filename[MAX_PATH];
	sprintf(filename, "%s/skin.ini", UI_Style.Current_Skin);
	
	windowObj *rom_window = calloc(sizeof(windowObj), 1);
	if(rom_window == NULL)
		goto CLEANUP;
		
	windowObj *window = rom_window;
	
	Set_Window_Font (window, &UI_Style.gfx.Font);
	Load_UniWindow_Ini(window, "Window Settings", msgs.buffer[WINDOW_TITLE], filename);	
	
	int barHt = 16;
    int txtheight = BAG_GetCharHt('g', window->font);
	int txtwidth = BAG_GetCharWd('W', window->font);
   
    u8 iconheight = NDS_ICON_HEIGHT,//nds icon size
			iconwidth = NDS_ICON_WIDTH;
			
	int old_width  = BAG_UniGetStrWd(msgs.buffer[WINDOW_TITLE], window->font);

	int j = 0;
	for (j = 0; j < option_count; j++)
	{
		int width = BAG_UniGetStrWd(msgs.buffer[MODE + j], window->font) + (optslen[j] * txtwidth);
		if( width > old_width)
			old_width = width;
	}	
	int boxwd = old_width + iconwidth + txtwidth,
		 boxht = (option_count * (txtheight + (txtheight>>1))) + barHt  + (txtheight>>1);
		 
	if(iconheight >= boxht)
		boxht += iconheight;
		
	Set_Window_Dim2(window, barHt, (256>>1) - (boxwd >> 1), (192>>1) - (boxht >> 1), boxwd, boxht);
	Create_Window_Scrn(window, DOWN_SCREEN);
	
	int iconX = window->x + (txtwidth>>1);
	int iconY = window->y + window->barHt + ((window->height - window->barHt) >>1) - (iconheight >>1);
	int textX = iconX + iconwidth + (txtwidth>>1);
	
	u16 settingtext[MAX_PATH];
	memset(settingtext, 0, sizeof(settingtext));
	
	void Draw_Menu(u16 * screen, int selected)
	{
		int textY = window->y + window->barHt + (txtheight >>1);
		Draw_Window(window);
		BAG_DrawObj (_eos_icon, screen, iconX, iconY);
		int i = 0;
		for( i = 0; i < option_count; i++)
		{
			int entry = i + 1;
			u16 Color = window->FontCol;
			if(selected == i)
				Color = window->HilightTxtCol;
				
			memset(settingtext, 0, sizeof(settingtext));	
			if( entry == MODE )
			{
				int num = DTCFile.Settings[i];
				wcscpy(settingtext, msgs.buffer[entry]);
				wcscat(settingtext, (u16 *) " ");
				wcscat(settingtext, msgs.buffer[CLEAN + num]);
			}
			else if( entry == CHEATS)
			{
				wcscpy(settingtext, msgs.buffer[entry]);
				wcscat(settingtext, (u16 *) " ");
				wcscat(settingtext, msgs.buffer[OFF + DTCFile.Settings[i]]);
			}
			else if( entry == SAVENUM)
			{
				char val[4];
				sprintf(val, " %d\0", DTCFile.Settings[i]);
				u16 val2[4];
				utf8_to_uni16(val, val2);
				
				wcscpy(settingtext, msgs.buffer[entry]);
				wcscat(settingtext, val2);
			}
			else
				wcscpy(settingtext, msgs.buffer[entry]);
			
			BAG_UniBmpPrintScrn(screen, textX , textY, window->x + window->width, textY, settingtext, window->font, Color, -2);
			textY += txtheight + (txtheight>>1);
		}

		Flip_Screen_Fun(DOWN_SCREEN, 2);
	}
	
	
	Draw_Menu(down_screen_addr, 0);
	int loop = 1, selected = 0, oldselected = -1, oldoption = 0;
	int oldSettings[10];

	int b = 0;
	for( b = 0; b < DTCFile.Key_Count; b++)
		oldSettings[b] = DTCFile.Settings[b];
	
	BAG_UpdateIN();
	while(loop)
	{
		if(Stylus.Held || Stylus.Newpress)
		{
			if(BAG_StylusZone(window->x, window->y, window->x + window->width, window->y + window->height))
				selected = ((Stylus.Y - window->y - window->barHt) *  (DTCFile.Key_Count + 1)) / (window->height - window->barHt - (txtheight>>1));
			else
				loop = 0;
		}
		
		if(Pad.Newpress.Down)
		{
			if(selected < option_count - 1)
				selected++;
		}
		if(Pad.Newpress.Up)
		{
			if(selected > 0)
				selected--;
		}
	
		if(Pad.Newpress.A || (Stylus.DblClick && selected == oldselected))
		{
			int entry = selected + 1;
			if(entry != EXIT)
			{
				int lim = 1;
				if(entry == SAVENUM)
					lim = 3;
					
				if(DTCFile.Settings[selected] < lim)
					DTCFile.Settings[selected]++;
				else
					DTCFile.Settings[selected] = 0;
			}
			else loop = 0;
		}
		
		if(Pad.Newpress.B)
			loop = 0;
		
		if(selected != oldselected || DTCFile.Settings[selected] != oldoption || MusicPlayer->minimize)
		{
			Draw_Menu(down_screen_addr, selected);
			oldselected = selected;
			oldoption = DTCFile.Settings[selected];
		}
		
		BAG_Update();
	}
	
	CLEANUP:
	Language_CleanBuffers(&msgs);
	if(_eos_icon)
	{
		BAG_DeleteObj(_eos_icon);
		free(_eos_icon);
	}
	if(window)
	{
		Delete_Window(window);
		free(window);
	}
	
	//check if there are any changes, if so write them
	for(b = 0; b < DTCFile.Key_Count; b++)
	{
		if(oldSettings[b] != DTCFile.Settings[b])
		{
			ini_Write_File(&DTCFile);
			break;
		}
	}
	
	ini_File_Clean(&DTCFile);
	SetScreenCap(0);
}



struct _file_info
{
	u16 *text[10];
	int count;
};

void CollectInfo(u16 * text, struct _file_info * info)
{
	info->text[info->count] = NULL;
	info->text[info->count] = calloc(256, sizeof(u16));
	if(info->text[info->count])
	{
		wcscpy(info->text[info->count], text);
		info->count++;
	}
}		

int GetFileInfo(const char *file, struct _file_info * info)
{
	char ext[EXT_NAME_LEN];
	memset(ext, 0, sizeof(ext));
	FB_GetExt(file, ext, sizeof(ext));
	info->count = 0;
	u16 textbuf[256];
	memset(textbuf, 0, sizeof(textbuf));
	
	//print the file type
	Language_FileReadStr(GlobalPaths.Language, textbuf, "Properties", "properties_msg0", ext);
	CollectInfo(textbuf, info);
	
	if(!strcasecmp(ext, sfiles_exec[NDS_EXT]))
	{
		//print the game id
		NDSHEADER nds;
		memset(&nds, 0, sizeof(NDSHEADER));
		NDS_ReadBanner(file, NDS_ROM_LANG, &nds);
		if(nds.type == ROM_CM)
		{
			Language_FileReadStr(GlobalPaths.Language, textbuf, "Properties", "properties_msg1", nds.game_code);
			CollectInfo(textbuf, info);
		}
	}
	else if(!strcasecmp(ext, sfiles_exec[NZIP_EXT]))
	{
		//print the game id
		NDSHEADER nds;
		memset(&nds, 0, sizeof(NDSHEADER));
		NZIP_ReadBanner(file, &nds);
		if(nds.type == ROM_CM)
		{
			Language_FileReadStr(GlobalPaths.Language, textbuf, "Properties", "properties_msg1", nds.game_code);
			CollectInfo(textbuf, info);
		}
	}
	
	
	struct _file_size fileInfo;
	memset(&fileInfo, 0, sizeof(struct _file_size));
	
	File_GetSize(file, &fileInfo);
	Language_FileReadFloat(GlobalPaths.Language, textbuf, "Properties", "properties_msg2", fileInfo.size);
	char unit[8];
	u16 unit16[8];
	switch(fileInfo.group)
	{
		case 0:strcpy(unit," bytes\0");break;
		case 1:strcpy(unit, " KB\0");break;
		case 2:strcpy(unit, " MB\0");break;
		case 3:strcpy(unit, " GB\0");break;
	}
	utf8_to_uni16( unit, unit16);
	wcscat(textbuf, unit16);
	CollectInfo(textbuf, info);
	
	return info->count;
}

void CleanInfo(struct _file_info * info)
{
	int i = 0;
	for(i = 0; i < info->count; i++)
	{
		if(info->text[i])
		{
			free(info->text[i]);
			info->text[i] = NULL;
		}
	}
	
	info->count = 0;
}


void File_Info(const char *file)
{
	BAG_UpdateIN();
	
	struct _file_info File_Properties;
	memset(&File_Properties, 0, sizeof(struct _file_info));
	int lines = GetFileInfo(file, &File_Properties);
	
	//prepare window
	char filename[256];
	sprintf(filename, "%s/skin.ini", UI_Style.Current_Skin);

	windowObj info_window;
	windowObj *window = &info_window;
	Set_Window_Font (window, &UI_Style.gfx.Font);
	u16 title[MAX_PATH];
	memset(title, 0, sizeof(title));
	Language_FileReadStr(GlobalPaths.Language, title, "Properties", "window_title", NULL);
	Load_UniWindow_Ini(window, "Window Settings", title, filename);	
	
	int barHt = 16;
    int txtheight = BAG_GetCharHt('g', window->font);

    int iconheight = (UI_Style.gfx.WindowConfirm->Height > UI_Style.gfx.WindowCancel->Height) ? UI_Style.gfx.WindowConfirm->Height : UI_Style.gfx.WindowCancel->Height;
	int old_width  = BAG_UniGetStrWd(title, window->font);
	int j = 0;
	for (j = 0; j < lines; j++)
	{
		int width = BAG_UniGetStrWd(File_Properties.text[j], window->font);
		if( width > old_width)
			old_width = width;
	}	
		
	int boxwd = old_width + 32,
		 boxht = (lines * (txtheight + (txtheight>>1))) + barHt + iconheight + txtheight;
		 
	Set_Window_Dim2(window, barHt, (256>>1) - (boxwd >> 1), (192>>1) - (boxht >> 1), boxwd, boxht);
	Render_DirGfx(down_screen_addr);
	Create_Window_Scrn(window, DOWN_SCREEN);
	
	void Draw(void)
	{
		Draw_Window(window);
		
		//write the message
		int i = 0;
		int TextStartY=window->y + barHt + (txtheight >>1);
		for( i = 0; i < lines; i++)
		{
			int TextStartX = (window->x + (window->width >> 1)) - (BAG_UniGetStrWd(File_Properties.text[i], window->font) >> 1);

			BAG_UniBmpPrintScrn(down_screen_addr, TextStartX , TextStartY, window->x + window->width - 8, window->y + window->height,File_Properties.text[i], window->font, window->FontCol, -2);	
			TextStartY += txtheight + (txtheight >> 1);
		}
		//draw buttons
		int buttonWd = UI_Style.gfx.WindowConfirm->Width;
		int buttonX = window->x + (window->width >> 1) - (buttonWd >> 1);
		int buttony = TextStartY + txtheight + 2;
		
		BAG_DrawObj(UI_Style.gfx.WindowConfirm, down_screen_addr, buttonX, buttony - (UI_Style.gfx.WindowConfirm->Height>>1));
							
		Flip_Screen_Fun(DOWN_SCREEN, 0);		
	}
	
	Draw();
	int read_loop = -1;
	while(read_loop == -1)
	{
		if(Stylus.Newpress)
		{
			if(BAG_ObjTouched(UI_Style.gfx.WindowConfirm))
				read_loop = 1;
		}
		else if(Pad.Newpress.A || Pad.Newpress.Start)
			read_loop = 1;
		
		else if(Pad.Newpress.B)
			read_loop = 1;
		
		//BAG_Update();
		BAG_UpdateAudio();
		BAG_UpdateIN();
	}
	CleanInfo(&File_Properties);
	Delete_Window(window);
}
