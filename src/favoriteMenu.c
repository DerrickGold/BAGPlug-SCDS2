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
#include "usermode.h"
#include "miscInterface.h"
/*========================================================================
Favorites Menu
========================================================================*/
#define FILESPERSCREEN 15
#define ICONSPERROW 5
struct _fav_disp_list
{
	char *Name;//full path
	char *ext;//extension
	char *shortName;//just name
	u8 Delete;
};

struct _fav_disp
{
	struct _scrollbar FavScroll;
	s16 txtheight, txtwidth, iconWd, iconHt, iconSlot, iconDiff, xStart;
	s16 Scroll, OldScroll, Max_Count, Scroll_Count, Display_Num, Old_DispNum, Selected;
	s16 xincrease, yincrease, xoffset, yoffset;
	struct _fav_disp_list *List;
	char list_file[MAX_PATH];//file path to favorites.txt
	BMPOBJ *Icon[FILESPERSCREEN];
};


int Cach_FavName(struct _fav_disp *menu, int file_num)
{
	int scroll_val = file_num + menu->Scroll;
	
	menu->List[scroll_val].Name = Global_FavoritesList.buffer[scroll_val];
	//cache extension for future use
	if(menu->List[scroll_val].ext == NULL)
	{
		char ext[EXT_NAME_LEN];
		memset(ext, 0, sizeof(ext));
		if(FB_GetExt(menu->List[scroll_val].Name, ext, EXT_NAME_LEN))
		{
			menu->List[scroll_val].ext = calloc(strlen(ext) + 1, sizeof(char));
			strcpy(menu->List[scroll_val].ext, ext);
		}
		else return 0;
	}

	//cache just the name for future use
	if(menu->List[scroll_val].shortName == NULL)
	{
		char filename[MAX_PATH];
		memset(filename, 0, sizeof(filename));
		if(FB_GetFileFromPath(filename, menu->List[scroll_val].Name))
		{
			menu->List[scroll_val].shortName = calloc(strlen(filename) + 1, sizeof(char));
			strcpy(menu->List[scroll_val].shortName, filename);
		}
		else 
			return 0;
	}	
	return 1;
}


inline int ShiftIconsUp(const struct _fav_disp *menu, int file_num)
{
	if(file_num >= FILESPERSCREEN - ICONSPERROW)
		return 0;
		
	BAG_Display_CloneObj(menu->Icon[file_num + ICONSPERROW], menu->Icon[file_num]);
	return 1;
}

inline int ShiftIconsDown(const struct _fav_disp *menu, int file_num)
{
	if(file_num <= ICONSPERROW)
		return 0;

	BAG_Display_CloneObj(menu->Icon[file_num - ICONSPERROW], menu->Icon[file_num]);
	return 1;
}

inline int ShiftIcons(const struct _fav_disp *menu, int file_num)
{
	if(menu->OldScroll == -1 || (menu->Scroll - menu->OldScroll) > ICONSPERROW)
		return 0;
	
	else if(menu->Scroll > menu->OldScroll)
		return ShiftIconsUp(menu, file_num);

	else if(menu->Scroll < menu->OldScroll)
		return ShiftIconsDown(menu, file_num);
		
	return 0;
}

int Load_FavIcon(const struct _fav_disp *menu, int file_num,BMPOBJ * Object)
{
	int scroll_val = file_num + menu->Scroll;
	if(ShiftIcons(menu, file_num))
		return 0;

	if(Load_CustomIcon(menu->List[scroll_val].shortName, "~", Object))
        return 1;
       
	//load nds icons
	if(UI_Style.IconsWd==NDS_ICON_WIDTH && UI_Style.IconsHt == NDS_ICON_HEIGHT)//if icon sizes in the browser are the same as nds icons
	{//load nds icon
		int compressed = 0;
		if(!strcasecmp(sfiles_exec[NDS_EXT], menu->List[scroll_val].ext))//standard nds file
			compressed = 0;
		else if(!strcasecmp(sfiles_exec[NZIP_EXT], menu->List[scroll_val].ext))//compressed nds file
			compressed = 1;
		else//not an nds format
			compressed = -1;
							
		if(compressed > -1)
		{		
			BAG_Display_DeleteObj( Object);
			BAG_CreateBlankObj( Object, 16, NDS_ICON_WIDTH, NDS_ICON_HEIGHT, NDS_ICON_WIDTH, NDS_ICON_HEIGHT);
			
			int rtrn = 0;
			switch(compressed)
			{
				case 0: rtrn = NDS_ReadIconEx(menu->List[scroll_val].Name, Object, 0);break;
				case 1:rtrn = NZIP_ReadIconEx(menu->List[scroll_val].Name, Object, 0);break;
			}	
			if(rtrn > 0)
				return 2;
		}
	}
	
    if(Load_DefaultIcon(menu->List[scroll_val].ext, Object) > 0)
        return 3;
	
	BAG_Display_CloneObj(UI_Style.gfx.Icons,  Object);
	BAG_SetObjFrame(Object, FRAME_VERT, 2);
	return 4;
}

void Favorites_Window_Create(windowObj * window)
{
	char filename[MAX_PATH];
	memset(filename, 0, sizeof(filename));
	sprintf(filename, "%s/skin.ini", UI_Style.Current_Skin);
	
	u16 tempbuf[MAX_PATH];
	memset(tempbuf, 0, sizeof(tempbuf));
	Language_FileReadStr(GlobalPaths.Language, tempbuf, "Favorites", "main_window_title", NULL);
	
	Set_Window_Font (window, &UI_Style.gfx.Font);
	Load_UniWindow_Ini(window, "Favorites Window", tempbuf, filename);
	Set_Window_Dim2(window, 16, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	Create_WindowBG_Scrn( window, UI_Style.gfx.FavBG, DOWN_SCREEN);
}


int Favorites_Icons_Create(struct _fav_disp * menu)
{
	int i = 0;
	for( i = 0; i < FILESPERSCREEN; i++)
	{
		menu->Icon[i] = calloc(sizeof(BMPOBJ), 1);
		if(!menu->Icon[i])
			return -i;
	}
	return 0;
}

void Favorite_Icons_Clean(struct _fav_disp * menu)
{
	int i = 0;
	for( i = 0; i < FILESPERSCREEN; i++)
	{
		if(menu->Icon[i])
		{
			BAG_DeleteObj(menu->Icon[i]);
			free(menu->Icon[i]);
		}
	}
}
		

void Favorites_List_Clean(struct _fav_disp * menu)
{
	if(menu->List)
	{
		int i = 0;
		for(i = 0; i < menu->Max_Count; i++)
		{
			//free(menu->List[i].Name);
			menu->List[i].Name = NULL;
			if(menu->List[i].ext)
			{
				free(menu->List[i].ext);
				menu->List[i].ext = NULL;
			}
			if(menu->List[i].shortName)
			{
				free(menu->List[i].shortName);
				menu->List[i].shortName = NULL;
			}
		}
		free(menu->List);
		menu->List = NULL;
	}
}	

//read favorites file and strip useless characters
int Favorites_List_Create(struct _fav_disp * menu)
{
	menu->Max_Count = TxtFile_getLineCount(&Global_FavoritesList);
	menu->List = NULL;
	menu->List = calloc(menu->Max_Count + 1, sizeof(struct _fav_disp_list));
	if(!menu->List)
		return 0;
	return 1;
}

void Favorites_Add(const char *file)
{
	TxtFile_writeAddLine(GlobalPaths.Favorites_txt, file);
	TxtFile_addLine(&Global_FavoritesList, file);
	
	/*if(Main_Settings.CombineFavsHidFiles)
		TxtFile_addLine(&Main_Browser->FB_Dir->HidFileList, file);*/	
	RefreshMainLists();
}
		
void Favorites_Remove(struct _fav_disp * menu)
{
	int i = 0;
	do
	{
		if(menu->List[i].Delete == 1)
		{
			TxtFile_removeLine(&Global_FavoritesList, i);
			RefreshMainLists();//recombine lists
			break;
		}
	}
	while(++i < menu->Max_Count);	
	TxtFile_Write(menu->list_file, &Global_FavoritesList, "wb");
}
	
void Draw_List(u16 * screen, windowObj * window, struct _fav_disp * menu)
{
	menu->Display_Num = (FILESPERSCREEN > menu->Max_Count - menu->Scroll) ? menu->Max_Count - menu->Scroll : FILESPERSCREEN;
	Draw_Window(window);//draw window background

	int x = menu->xoffset;
	int y = menu->yoffset;
	int xcount = 0, ycount = 0, xincrease = menu->xincrease, yincrease = menu->yincrease;
	int bary = ((FILESPERSCREEN / ICONSPERROW) * menu->yincrease) + menu->yoffset;
	//draw out the favorites icons
	u16 Unicode[MAX_NAME_LEN];
	
	//looping instructions
	int start = 0,
		end = menu->Display_Num,
		dir = 1,
		xstart = 0,
		rowmod = 1;
		
	//if scrolling up, adjust everything to render backwards for backwards looping	
	if(menu->Scroll < menu->OldScroll)
	{
		start = menu->Display_Num - 1;
		end = -1;
		dir = -1;
		xincrease = -menu->xincrease;
		yincrease = -menu->yincrease;
		ycount = abs(yincrease) * 2;
		xstart = abs(xincrease) * (ICONSPERROW - 1);
		xcount = xstart;
		rowmod = 0;
	}
	
	int i = start;
	while( i != end)
	{
		menu->List[menu->Scroll + i].Delete = 0;
		
		if(menu->OldScroll != menu->Scroll)
		{
			Cach_FavName(menu, i);
			Load_FavIcon(menu, i, menu->Icon[i]);
		}
	
		u16 TxtCol = window->FontCol;
		if(menu->Selected == i)
		{
			TxtCol = window->HilightTxtCol;
			BAG_SetObjGrey(menu->Icon[i], 1);
			if(Main_Settings.favs_hideTxt)
				Screen_Draw_Rect(screen, x + xcount - 2, y + ycount - 2, menu->iconWd + 4, menu->iconHt + 4, window->HilightCol);
		}
		else
			BAG_SetObjGrey(menu->Icon[i], 0);
			
	
	    BAG_DrawObj(menu->Icon[i], screen, x + xcount, y + ycount);
		
		//write names
		int text_x = x + xcount - 2;
		int text_y = y + ycount + menu->iconHt + 2;
		int text_x2 =  text_x + menu->iconWd + menu->txtwidth;
		
		if(menu->List[i + menu->Scroll].shortName)
		{
			memset(Unicode, 0, sizeof(Unicode));
			utf8_to_unicode16(menu->List[i + menu->Scroll].shortName, Unicode);
			
			if(Main_Settings.favs_hideTxt == 0)
				BAG_UniBmpPrintScrn(screen, text_x , text_y, text_x2, window->y + window->height, Unicode, window->font, TxtCol, -2);	
			if(menu->Selected == i)
			{
				int bottom_textY =bary +  ((((window->y + window->height) - bary)>>1) - (menu->txtheight>>1));
				int bottom_textX = ((window->x + window->width) >> 1) - (BAG_BmpTxt_UniGetStrLenPix(Unicode, window->font) >> 1);
				BAG_UniBmpPrintScrn(screen, bottom_textX , bottom_textY, window->x + window->width, window->y + window->height, Unicode, window->font, TxtCol, -2);	
			}
		}
		else
		{
			if(Main_Settings.favs_hideTxt == 0)
				BAG_BmpPrintScrn(screen,  text_x, text_y, text_x2, window->y + window->height, (unsigned char*)"Error", window->font, TxtCol, -2);
		}
			
		if( (i + rowmod) % ICONSPERROW == 0 )
		{
			xcount = xstart;
			ycount += yincrease;
		}
		else
			xcount += xincrease;
			
		i += dir;
	}
	//BAG_DrawObj(UI_Style.gfx.ScrollBar, screen, window->x + window->width - UI_Style.gfx.ScrollBar->Width, menu->ScrollBarY);
	ScrollBar_Draw(screen, &menu->FavScroll, menu->Scroll);
	menu->OldScroll = menu->Scroll;
	menu->Old_DispNum = menu->Display_Num;
}	

void Fav_ScanningMsg(u16 * Screen, windowObj * window)
{
	Draw_Window(window);
	
	u16 msg[MAX_PATH];
	memset(msg, 0, sizeof(msg));
	Language_FileReadStr(GlobalPaths.Language, msg, "Favorites", "scan_msg0", NULL);

	int strwidth = BAG_UniGetStrWd(msg, window->font);
	int txtheight = BAG_GetCharHt('g', window->font);
	int textX = ((window->x + window->width)>>1) - (strwidth >>1);
	int textY = ((window->y + window->height)>>1) - (txtheight >> 1);
	
	BAG_UniBmpPrintScrn(Screen, textX, textY, window->x + window->width, window->y + window->height, msg, window->font, window->FontCol, -2);	
	Flip_Screen_Fun(DOWN_SCREEN, 0);	
}

void Favs_EmptyWarning(void)
{
	typedef enum{
		__WINDOW_TITLE,
		__MESSAGE,
	}favswarningmsgs;
	
	struct _language msgs;
	memset(&msgs, 0, sizeof(struct _language));
	if(Language_InitBuffers(&msgs, 2) == 1)
	{

		Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__WINDOW_TITLE], "Error Messages", "window_title", NULL);
		Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__MESSAGE], "Error Messages", "favorites_empty", NULL);
		Unicode_Notification_Window(msgs.buffer[__WINDOW_TITLE], (u16**)&msgs.buffer[__MESSAGE], 1, 1);
		
		Language_CleanBuffers(&msgs);
	}

}

int InitFavs(struct _fav_disp * menu, windowObj * window)
{
	 int list_test = Favorites_List_Create(menu);
	if(menu->Max_Count <= 0 || list_test != 1 )
	{
		Favs_EmptyWarning();
		return 0;
	}
	
	Favorites_Icons_Create(menu);
	
	//populate the menu struct
	menu->txtheight = BAG_GetCharHt('g', window->font) + 1;
	menu->txtwidth = BAG_GetCharWd (' ', window->font);
	menu->iconWd = UI_Style.IconsWd;
	menu->iconHt = UI_Style.IconsHt;
	menu->iconSlot = menu->iconHt + menu->txtheight + 1;
	menu->iconDiff = ((window->width / ICONSPERROW) - (menu->iconSlot) + menu->txtwidth);
	menu->xStart =  (window->width - (ICONSPERROW * (menu->iconWd+ menu->iconDiff))) >> 1;
	menu->Display_Num = 0;
	menu->Scroll = 0;
	menu->OldScroll = -1;
	menu->Scroll_Count = menu->Max_Count;
	menu->xincrease = menu->iconWd+menu->iconDiff;
	if(Main_Settings.favs_hideTxt == 1)
		menu->yincrease = menu->iconHt + menu->txtheight + 2;
	else
		menu->yincrease = menu->iconHt + menu->txtheight + 6;
		
	menu->xoffset = window->x + menu->xStart + 5;
	menu->yoffset = window->y + window->barHt + 10;

	while(menu->Scroll_Count % 5)
		menu->Scroll_Count++;
		
	return 1;
}

int Reload_Menu(struct _fav_disp * menu, windowObj * window)
{
	Favorite_Icons_Clean(menu);
	Favorites_List_Clean(menu);
	strcpy(menu->list_file, GlobalPaths.Favorites_txt);
	return InitFavs(menu, window);
}

int Favorites_Remove_Msg(struct _fav_disp * menu, int file)
{
	typedef enum{
		__WINDOW_TITLE,
		__MESSAGE0,
		__MESSAGE1,
		__MESSAGE2,
		__MSGCOUNT,
	}removemsgs;
	
	struct _language *msgs = NULL;
	msgs = calloc(sizeof(struct _language), 1);
	if(msgs == NULL)
		return 0;
		
	if(Language_InitBuffers(msgs, __MSGCOUNT) == 1)
	{
		Language_FileReadStr(GlobalPaths.Language, msgs->buffer[__WINDOW_TITLE], "Favorites", "remove_window_title", NULL);
		Language_FileReadStr(GlobalPaths.Language, msgs->buffer[__MESSAGE0], "Favorites", "remove_msg0", NULL);
		utf8_to_uni16( menu->List[file].shortName, msgs->buffer[__MESSAGE1]);
		Language_FileReadStr(GlobalPaths.Language, msgs->buffer[__MESSAGE2], "Favorites", "remove_msg1", NULL);
		int choice = Unicode_Notification_Window(msgs->buffer[__WINDOW_TITLE], (u16**)&msgs->buffer[__MESSAGE0], __MSGCOUNT - 1, 2);
		Language_CleanBuffers(msgs);
		
		if(choice == 1)
		{
			menu->List[file].Delete = 1;
			Favorites_Remove(menu);
			menu->Selected = 0;
		}
	}
	free(msgs);
	return 1;
}

extern int Load_File(char *filename);

int Favorites_RomProperties(struct _fav_disp * menu, windowObj * window)
{
	int selected = menu->Scroll + menu->Selected;
	NDSHEADER *nds = NULL;

	//determine if file is an nds or compressed file, or not.
	int isNDS = 0;
	if(!strcasecmp(menu->List[selected].ext, sfiles_exec[NDS_EXT]))
		isNDS = 1;
	else if(!strcasecmp(menu->List[selected].ext, sfiles_exec[NZIP_EXT]))
		isNDS = 2;
		
	if(isNDS == 0)
		return 0;
	
	//allocate nds header memory
	nds = calloc(sizeof(NDSHEADER), 1);
	if(nds == NULL)
		return 0;
	
	//read file header
	switch(isNDS)
	{
		default:
			NDS_ReadTitle(menu->List[selected].Name, nds);
		break;
		case 2:
			NZIP_ReadTitle(menu->List[selected].Name, nds);
		break;
	}
	
	int type = nds->type;
	free(nds);
	nds = NULL;
	
	if(type == ROM_CM)
	{
		Draw_List(down_screen_addr, window, menu);
		EOS_Properties(menu->List[selected].Name, (isNDS - 1));
		return 1;
	}
	return 0;
}


int Favorites_Menu(char *file)
{
	windowObj *fav_window = NULL;
    //Create the window graphics
	fav_window = calloc(sizeof(windowObj), 1);
	if(fav_window == NULL)
		return 0;
		
	Favorites_Window_Create(fav_window);
	Fav_ScanningMsg(down_screen_addr, fav_window);
	
	//allocate favorites list struct
	struct _fav_disp *fav_menu = NULL;
	fav_menu = calloc(sizeof(struct _fav_disp), 1);
	if(fav_menu == NULL)
	{
		Delete_Window(fav_window);
		free(fav_window);
		fav_window = NULL;
		return 0;
	}
		
	//check if favorites file exists and is populated
   if(!Reload_Menu(fav_menu, fav_window))
   {
		Delete_Window(fav_window);
		free(fav_window);
		free(fav_menu);
		return 0;
	}
			
	//some useful variables
    int rtrnval = 0;
    //create buffers for icons
	
    int pix = 0, bary = ((FILESPERSCREEN / ICONSPERROW) * fav_menu->yincrease) + fav_menu->yoffset;
    for( pix = 0; pix < fav_window->width; pix++)
        BAG_SetObjPix( fav_window->gfx, pix, bary, fav_window->HilightCol);  
		
    //keep the menu alive
    s16 loop = 1, oldselected = -1, selected = 0;
	fav_menu->Selected = 0;
	
	int scrollbarCount = fav_menu->Max_Count;
	while(scrollbarCount % ICONSPERROW != 0)
		scrollbarCount++;
		
	ScrollBar_Create(&fav_menu->FavScroll, UI_Style.gfx.ScrollBar, fav_window->width - UI_Style.gfx.ScrollBar->Width, fav_window->barHt, 
								fav_window->width, bary, scrollbarCount, FILESPERSCREEN, ICONSPERROW);
	
	BAG_UpdateIN();
	CoverArt_UseFavorites(1);
    while(loop)
    {
        //select the item by touch screen
        if(Stylus.Newpress || Stylus.Held)
        {
			int scroll = ScrollBar_Stylus(&fav_menu->FavScroll);
			if(scroll != -1)
			{
				fav_menu->Scroll = scroll;
				oldselected = -1;
			}
			else
			{
				int touch_loop = 0;
				for(touch_loop = 0; touch_loop < fav_menu->Display_Num; touch_loop++)
				{
					if(BAG_ObjTouched(fav_menu->Icon[touch_loop]))
					{
						fav_menu->Selected = touch_loop;
						break;
					}
				}
			}
        }
		
		//delete favorite
		if(Pad.Newpress.Select && UserPerm_CheckFileOps())
		{
			Draw_List(down_screen_addr, fav_window, fav_menu);
			if(Favorites_Remove_Msg(fav_menu, selected))
			{
				if(!Reload_Menu(fav_menu, fav_window))
					break;
			}
			oldselected = -1;
		}
		
         //pad controls
         if(Pad.Newpress.Left && fav_menu->Selected > 0)
            fav_menu->Selected--;
        if(Pad.Newpress.Right && fav_menu->Selected < fav_menu->Display_Num - 1)
            fav_menu->Selected++;
            
        if(Pad.Newpress.Up && fav_menu->Selected - ICONSPERROW >= 0)
            fav_menu->Selected-=ICONSPERROW;
		else if(Pad.Newpress.Up)
		{
			if(fav_menu->Scroll >= ICONSPERROW)
				fav_menu->Scroll-= ICONSPERROW;
			oldselected = -1;
		}
        if(Pad.Newpress.Down && fav_menu->Selected + ICONSPERROW < fav_menu->Display_Num )
            fav_menu->Selected+=ICONSPERROW;
		else if(Pad.Newpress.Down)
		{
			if(fav_menu->Scroll + fav_menu->Display_Num < fav_menu->Max_Count)
				fav_menu->Scroll+= ICONSPERROW;
				
			while(fav_menu->Scroll + fav_menu->Selected >= fav_menu->Max_Count)
				fav_menu->Selected--;
				
			oldselected = -1;
		}

		selected = fav_menu->Scroll + fav_menu->Selected;

        if(fav_menu->Selected != oldselected || MusicPlayer->minimize)
        {
             Draw_List(down_screen_addr, fav_window, fav_menu);
             Flip_Screen_Fun(DOWN_SCREEN, 0);	
             oldselected = fav_menu->Selected;
			 CoverArt_ExternFile(fav_menu->List[selected].Name);
        }   
		
         //load file
        if( Pad.Newpress.A || (Stylus.DblClick && oldselected ==  fav_menu->Selected))
        {
			Draw_List(down_screen_addr, fav_window, fav_menu);
			BAG_Update();
			strcpy(global_launchFile, fav_menu->List[selected].Name);
			FILE_LAUNCH_VAL = Load_File(fav_menu->List[selected].Name);
			if(FILE_LAUNCH_VAL != 0)
			{
				rtrnval = FB_LOAD_SKIP;
				break;
			}
			oldselected = -1;
        }
        //exit favorites
        if(Pad.Newpress.B && UserPerm_CheckBrowser())
        {
            rtrnval = 0;
			break;
        }
		
		if(Pad.Newpress.Y)
		{		
			if(Favorites_RomProperties(fav_menu, fav_window))
				oldselected = -1;
		}		
		
        BAG_Update();
    }
     if(rtrnval == FB_LOAD)
        strcpy(file, fav_menu->List[fav_menu->Scroll + fav_menu->Selected].Name);
		
	ScrollBar_Delete(&fav_menu->FavScroll);
	Delete_Window(fav_window);
	free(fav_window);
	Favorite_Icons_Clean(fav_menu);
	Favorites_List_Clean(fav_menu);
	free(fav_menu);
	CoverArt_UseFavorites(0);
	return rtrnval;//return 0 to go back to file browser
}
