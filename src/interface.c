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
#include "menu_files.h"
#include "language.h"
#include "usermode.h"
#include "miscInterface.h"
/*========================================================================
FileBrowser Interface coding
========================================================================*/

GUI UI_Style;
int StylusFun(void);
int icon_refresh = 0;

/************************
Load Interface graphics
************************/
//load skin settings from ini file
void Skin_Ini(GUI * ui, const char *filename)
{
	ui->FontCol = ini_getl("Main Browser", "Text_Col", 0, filename);
	ui->FontSelCol = ini_getl("Main Browser", "Text_Highlight", 0, filename);
	ui->TransCol = ini_getl("Main Browser", "GFX_Transparency", 0, filename);
	ui->FontSize = ini_getl("Main Browser", "FontSize", 0, filename);
	ui->TopBar_TxtCol = ini_getl("Main Browser", "Top_Bar_Text_Col", 0, filename);
	ui->GFXAlpha[0] = ini_getl("Main Browser", "Cursor_Alpha", 255, filename);
	ui->GFXAlpha[1] = ini_getl("Main Browser", "Sidebar_Alpha", 255, filename);
    ini_gets("Main Browser","Custom_Icon_Folder", "~", ui->Icon_Folder, MAX_PATH, filename);

	int len = strlen(ui->Icon_Folder);
	if(len == 0)
		ui->Icon_Folder[0] = '~';
	else
		FB_FixFilePath(ui->Icon_Folder);
}


inline int Load_Image( const char * filename, BMPOBJ * Object, int width, int height)
{
	char name[MAX_PATH];
    memset(name, 0, sizeof(name));
    int err = NO_ERR;
    
    strcpy(name, filename);
    strcat(name, ".bmp");
	
	if(FB_FileExists(name))
	{
		err = BAG_LoadBmp(name, Object, width, height);
        BAG_ObjTransCol(Object, UI_Style.TransCol);
		return err;
	}

	name[0] = '\0';
	strcpy(name, filename);
	strcat(name, ".png");
	err = BAG_LoadPng(name, Object, width, height);
	BAG_ObjTransCol(Object, UI_Style.TransCol);
    return err;
}

void Load_Font(const char *filename, BMP_FNT * _fnt_, int width, int height, u16 transcol)
{
	char name[MAX_PATH];
	sprintf(name, "%s.bmp", filename);
	if(FB_FileExists(name))
		BAG_BmpLoadFnt(name, _fnt_, width, height, transcol);
	else
	{
		sprintf(name, "%s.png", filename);
		BAG_PngLoadFnt(name, _fnt_, width, height, transcol);
	}
}
	
void Load_GFX(GUI * ui, const char *folder)
{
	char file[MAX_PATH];
	memset(file, 0, sizeof(file));
	int i = 0;
	
	sprintf(file, "%s/skin.ini", folder);
	Skin_Ini(ui, file);
	
	
	guigfx * gfx = &ui->gfx;

	sprintf(file, "%s/font", folder);
	Load_Font(file, &gfx->Font, 16, 16, ui->TransCol);
	gfx->Font.Space_Wd-=1;
	
	int gfxnum = 0, sideicon_count = 0;

	sprintf(file, "%s/mainbg", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 256, 192);
	BAG_SetObjFrame( &gfx->GUIGFX[gfxnum], FRAME_VERT, 1);
    gfx->BG[0] = &gfx->GUIGFX[gfxnum++];
	
	BAG_Display_CopyObj(gfx->BG[0], &gfx->GUIGFX[gfxnum]);
	BAG_SetObjFrame( &gfx->GUIGFX[gfxnum], FRAME_VERT, 0);
    gfx->BG[1] = &gfx->GUIGFX[gfxnum++];	
	
	sprintf(file, "%s/favbg", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 256, 192);
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
    gfx->FavBG = &gfx->GUIGFX[gfxnum++];
	
	sprintf(file, "%s/selectbar", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 240, 38);
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
	BAG_Effects_ObjAlpha (&gfx->GUIGFX[gfxnum], ui->GFXAlpha[0]);
    gfx->Cursor = &gfx->GUIGFX[gfxnum++];
	
	sprintf(file, "%s/sidebar", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 16, 192);
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
	BAG_Effects_ObjAlpha (&gfx->GUIGFX[gfxnum], ui->GFXAlpha[1]);
    gfx->SideBar = &gfx->GUIGFX[gfxnum++];
	
	sprintf(file, "%s/confirm", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 42, 22);	
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
    gfx->WindowConfirm = &gfx->GUIGFX[gfxnum++];
	
	sprintf(file, "%s/cancel", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 42, 22);
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
    gfx->WindowCancel = &gfx->GUIGFX[gfxnum++];
    
	sprintf(file, "%s/scrollbar", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 8, 16);
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
    gfx->ScrollBar = &gfx->GUIGFX[gfxnum++];
	
	sprintf(file, "%s/settings", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 24, 24);
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
	gfx->SIDEICONS[sideicon_count++] = &gfx->GUIGFX[gfxnum];
    gfx->SettingsIcon = &gfx->GUIGFX[gfxnum++];
	
	sprintf(file, "%s/light", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 24, 24);
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
	gfx->SIDEICONS[sideicon_count++] = &gfx->GUIGFX[gfxnum];
    gfx->LightIcon = &gfx->GUIGFX[gfxnum++];
	
	sprintf(file, "%s/favorite", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 24, 24);
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
	gfx->SIDEICONS[sideicon_count++] = &gfx->GUIGFX[gfxnum];
    gfx->FavoriteIcon = &gfx->GUIGFX[gfxnum++];
    
	sprintf(file, "%s/music", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 24, 24);
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
	gfx->SIDEICONS[sideicon_count++] = &gfx->GUIGFX[gfxnum];
    gfx->MusicIcon = &gfx->GUIGFX[gfxnum++];
	
	sprintf(file, "%s/nzip", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 24, 24);
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
	gfx->SIDEICONS[sideicon_count++] = &gfx->GUIGFX[gfxnum];
    gfx->NzipIcon = &gfx->GUIGFX[gfxnum++];
	
	sprintf(file, "%s/arrow", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 16, 16);	
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
	gfx->SIDEICONS[sideicon_count++] = &gfx->GUIGFX[gfxnum];
    gfx->Arrow[0] = &gfx->GUIGFX[gfxnum++];
	
	BAG_Display_CopyObj(gfx->Arrow[0], &gfx->GUIGFX[gfxnum]);
	BAG_FlipObj(&gfx->GUIGFX[gfxnum], FRAME_VERT);
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
    gfx->Arrow[1] = &gfx->GUIGFX[gfxnum++];
  
	//nzip manager
	sprintf(file, "%s/selectall", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 22, 22);
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
    gfx->SelectAll = &gfx->GUIGFX[gfxnum++];  
	
	sprintf(file, "%s/exit", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 22, 22);
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
    gfx->Exit = &gfx->GUIGFX[gfxnum++]; 
	
	sprintf(file, "%s/delete", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 22, 22);
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
    gfx->Delete = &gfx->GUIGFX[gfxnum++]; 

	//create buffers for nds icons
	i = 0;
	for(i = 0; i < ui->FilesPerScreen; i++)
    {	
        
		BAG_CreateBlankObj(&gfx->GUIGFX[gfxnum], 16, ui->IconsWd, ui->IconsHt, ui->IconsWd, ui->IconsHt);
        gfx->NDSIcon[i] = &gfx->GUIGFX[gfxnum++];
    }
	
	//load default file icons - copy for each file on screen
	sprintf(file, "%s/icons", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], ui->IconsWd, ui->IconsHt);
    gfx->Icons = &gfx->GUIGFX[gfxnum++];
	
    //settings main graphics
	sprintf(file, "%s/settings/display", folder);
	Load_Image(file,  &gfx->GUIGFX[gfxnum], ui->IconsWd, ui->IconsHt);	
	BAG_ResetFrameDim( &gfx->GUIGFX[gfxnum]);
    gfx->Main_Settings[0] = &gfx->GUIGFX[gfxnum++];
    
	sprintf(file, "%s/settings/skin", folder);
	Load_Image(file,  &gfx->GUIGFX[gfxnum], ui->IconsWd, ui->IconsHt);	
	BAG_ResetFrameDim( &gfx->GUIGFX[gfxnum]);
    gfx->Main_Settings[1] = &gfx->GUIGFX[gfxnum++];
    
	sprintf(file, "%s/settings/cpu", folder);
	Load_Image(file,  &gfx->GUIGFX[gfxnum], ui->IconsWd, ui->IconsHt);	
	BAG_ResetFrameDim( &gfx->GUIGFX[gfxnum]);
    gfx->Main_Settings[2] = &gfx->GUIGFX[gfxnum++];
    //close
	sprintf(file, "%s/settings/close", folder);
	Load_Image(file,  &gfx->GUIGFX[gfxnum], ui->IconsWd, ui->IconsHt);	
	BAG_ResetFrameDim( &gfx->GUIGFX[gfxnum]);
    gfx->Main_Settings[3] = &gfx->GUIGFX[gfxnum++];
    
    sprintf(file, "%s/settings/on", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 16, 16);	
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
    gfx->BoxOn = &gfx->GUIGFX[gfxnum++];
    
    sprintf(file, "%s/settings/off", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 16, 16);	
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
    gfx->BoxOff = &gfx->GUIGFX[gfxnum++];
        
	//top bar
	sprintf(file, "%s/topbar", folder);
	Load_Image(file, &gfx->GUIGFX[gfxnum], 256, 16);
	BAG_ResetFrameDim(&gfx->GUIGFX[gfxnum]);
    gfx->TopBar = &gfx->GUIGFX[gfxnum++];
		
		
	gfx->gfxcount = gfxnum;
}

void Unload_GFX (GUI * ui)
{
	guigfx * gfx = &ui->gfx;
	BAG_DelBmpFnt(&gfx->Font);
	int i = 0;
	for(i = 0; i < gfx->gfxcount; i++)
		BAG_DeleteObj(&gfx->GUIGFX[i]);
}

/************************
Set browser style
************************/
void SetUIStyle(GUI *ui, const char *skin_folder)
{
	
	ui->IconsWd = NDS_ICON_WIDTH;
	ui->IconsHt = NDS_ICON_HEIGHT;
	ui->IconsDiff = ui->IconsHt + 6;//ammount of pixels to skip to draw next icon in list
	ui->FilesPerScreen = SCREEN_HEIGHT / ui->IconsDiff;
	
	Load_GFX(ui, skin_folder);
	ui->IconsX = ui->gfx.SideBar->Width;
	int i = 0;
	for( i = 0; i < NUM_SIDE_ICONS; i++)
	{
		if(ui->gfx.SIDEICONS[i]->Width > ui->IconsX)
			ui->IconsX = ui->gfx.SIDEICONS[i]->Width;
	}
	
	
	ui->IconsX += 2;
	ui->IconsY = ((SCREEN_HEIGHT / ui->IconsDiff) >> 1) + 1;//y position to start drawing icon list at

	ui->TextX = ui->IconsX + ui->IconsWd + (ui->gfx.Font.Space_Wd);//x position to draw text at
	ui->TextY = ui->IconsY + (ui->IconsHt >> 1) - (BAG_GetCharHt('g', &ui->gfx.Font)>>1); //draw text at the center of each icon listed
	ui->TextDiff = ui->IconsDiff;//draw text every icon height
	ui->DispNameLen = ((SCREEN_WIDTH - ui->TextX) / ui->gfx.Font.Space_Wd) << 1;
	
	
	ui->CursorX = ui->gfx.SideBar->Width;
	ui->CursorY = 0;
	ui->CursorHt = ui->IconsDiff;
}

void ReloadUI(GUI *ui, UI_FB * fbstate)
{
    Free_IconCache(ui);
	Unload_GFX (ui);
    Unload_MediaPlayer_Gfx(MusicPlayer);
	Settings_Read_Ini(Main_Browser, &UI_Style, GlobalPaths.Settings_ini, 1);
	memset(Main_Browser->State.Folder, 0, sizeof(Main_Browser->State.Folder));//don't open the last folder
	SetUIStyle(ui, UI_Style.Current_Skin);
    
    Init_IconCache(ui);
	fbstate->Render_FB(fbstate);
	fbstate->Render_FB(fbstate);
}
	
	

/************************
File browser cursor
************************/
void FB_Cursor(GUI *ui, u16 *Dest, s16 Cursor_Scroll)
{
	int i = 0;
	if(Cursor_Scroll < 0) 
		Cursor_Scroll = 0;
	else if(Cursor_Scroll % (ui->FilesPerScreen - 1) > ui->FilesPerScreen-1)
		Cursor_Scroll = ui->FilesPerScreen-1;
		
	BAG_DrawObj (ui->gfx.Cursor, Dest, ui->CursorX, ui->CursorY + (Cursor_Scroll * ui->CursorHt) + i);
}
	
/************************
NDS Icon handling
************************/	
int oldScroll = 0;	

inline int Reload_Image( const char *filename, BMPOBJ * Object)
{
	char name[MAX_PATH];
    memset(name, 0, sizeof(name));
    int err = NO_ERR;
    
    strcpy(name, filename);
    strcat(name, ".bmp");
	
	if(FB_FileExists(name))
		err = BAG_ReloadBmp(name, Object);
	else
	{
        name[0] = '\0';
        strcpy(name, filename);
        strcat(name, ".png");
		err = BAG_ReloadPng(name, Object);
	}
	
    return err;
}

int Load_CustomIcon(const char *file, const char *path, BMPOBJ * Object)
{
    //copy the name
    char name[MAX_PATH];
    memset(name, 0, sizeof(name));
    strcpy(name, file);
      
	BAG_Display_DeleteObj(Object);
	BAG_CreateBlankObj(Object, 16, UI_Style.IconsWd, UI_Style.IconsHt, UI_Style.IconsWd, UI_Style.IconsHt);
    
    char fileLocation[MAX_PATH];
    memset(fileLocation, 0, sizeof(fileLocation));
    //check if icon is in the custom icon folder
    if(UI_Style.Icon_Folder[0] != '~')
    {
        strcpy(fileLocation, UI_Style.Icon_Folder);
        strcat(fileLocation, name);
		if(Reload_Image( fileLocation, Object) == NO_ERR)
		{
            BAG_SetObjFrame( Object, FRAME_VERT, 0);
            BAG_ObjTransCol(  Object, UI_Style.TransCol);
            return 1;
        }
    }
    else
    {
        //check in local directory
        if(path[0] != '~')
        {
            strcpy(fileLocation, path);
            strcat(fileLocation, name);
           
        }
        else
            strcpy(fileLocation, name);
        
		if(Reload_Image( fileLocation, Object) == NO_ERR)
		{
            BAG_SetObjFrame( Object, FRAME_VERT, 0);
            BAG_ObjTransCol(  Object, UI_Style.TransCol);
            return 1;
        }
    }
    return 0;
}

/*Default Icon handling=================================================*/

int Init_IconCache(GUI *ui)
{
    FileBrowser *icon_list = FB_Init (ALLTYPES, 0);
    
    char folder[MAX_PATH];
    memset(folder, 0, sizeof(folder));
    sprintf(folder, "%s/icons/", UI_Style.Current_Skin);
    
    FB_ScanDir( folder, icon_list, 0, (char**)&sfiles_image, 0);
    
    ui->gfx.Cache_size = icon_list->File_Count;
    
    FB_Free (icon_list);
    
    ui->gfx.Cache_count = 0;
    
    ui->gfx.Cache = calloc( ui->gfx.Cache_size + 1, sizeof(struct _iconCache));
    if(!ui->gfx.Cache)
    {
         ui->gfx.Cache_size = 0;
        return -1;
    }
        
    return 1;
}

void Free_IconCache(GUI * ui)
{
    if(ui->gfx.Cache_size > 0)
    {
        int i = 0;
        for(i = 0; i < ui->gfx.Cache_size; i++)
        {
           BAG_DeleteObj(&ui->gfx.Cache[i].Icon);

            //delete text
            if(ui->gfx.Cache[i].ext)
            {
                free(ui->gfx.Cache[i].ext);
                ui->gfx.Cache[i].ext = NULL;
            }
        }
        ui->gfx.Cache_size = 0;
        
        free(ui->gfx.Cache);
        ui->gfx.Cache = NULL;
    }
}

//will return the first freed cache slot for loading into
int Cache_Icon( const char * ext, GUI * ui)
{
    if(ui->gfx.Cache_size)
    {
        //if all the cache slots are full, don't cache an icon
        int cache_num = ui->gfx.Cache_count;
        
        if(cache_num >= ui->gfx.Cache_size)
            return -1;
        
       BAG_CreateBlankObj(&ui->gfx.Cache[cache_num].Icon, 16, UI_Style.IconsWd, UI_Style.IconsHt, UI_Style.IconsWd, UI_Style.IconsHt);
       
       ui->gfx.Cache[cache_num].ext = calloc(strlen(ext), sizeof(u8));
       if(!ui->gfx.Cache[cache_num].ext)
            return -3;
            
       strcpy(ui->gfx.Cache[cache_num].ext, ext);
       
       ui->gfx.Cache_count++;
       return cache_num;
   }
   return -4;
}

inline int isCached(const char * ext, GUI *ui)
{
    int i = 0;
    for(i = 0; i < ui->gfx.Cache_count; i++)
    {
        if(!strcasecmp(ext, ui->gfx.Cache[i].ext))
            return i;
    }
    return -1;
}

int Load_DefaultIcon(const char *ext, BMPOBJ * Object)
{
	int cache = isCached(ext, &UI_Style);
	if(cache > -1)
	{
		BAG_Display_CloneObj(&UI_Style.gfx.Cache[cache].Icon, Object);
		return 1;
	}
	else
	{
		char filepath[MAX_PATH];
		memset(filepath, 0, sizeof(filepath));
		strcpy(filepath, UI_Style.Current_Skin);
		strcat(filepath,"/icons/");
		strcat(filepath, ext);

		char name[MAX_PATH];
		memset(name, 0, sizeof(name));
		
		strcpy(name, filepath);
		strcat(name,".bmp");
		
		if(FB_FileExists(name))
		{
			int load_cache = Cache_Icon(ext, &UI_Style);
			if(load_cache < 0)
				return 0;
				
			BMPOBJ * icon = (BMPOBJ*)&UI_Style.gfx.Cache[load_cache].Icon;
		
            BAG_ReloadBmp(name, icon);
            BAG_SetObjFrame( icon, FRAME_VERT, 0);
            BAG_ObjTransCol( icon, UI_Style.TransCol);
            BAG_Display_CloneObj(icon, Object);
            return 2;
        }
        else
        {	
            name[0] = '\0';
            strcpy(name, filepath);
            strcat(name,".png");           
			if(FB_FileExists(name))
			{
				//prepare icon cache
				int load_cache = Cache_Icon(ext, &UI_Style);
				if(load_cache < 0)
					return 0;
					
				BMPOBJ * icon = (BMPOBJ*)&UI_Style.gfx.Cache[load_cache].Icon;		

                BAG_ReloadPng(name, icon);
                BAG_SetObjFrame( icon, FRAME_VERT, 0);
                BAG_ObjTransCol( icon, UI_Style.TransCol);
                BAG_Display_CloneObj( icon, Object);
                return 3;		
            }
        }
   }
	return 0;
}


int _load_Icon(guigfx * GFX, BMPOBJ * Object,  FileBrowser * FB_Handle,int Scroll, int i, int isFolder)
{
    char ext[EXT_NAME_LEN];
    memset(ext, 0, sizeof(ext));
    if(!isFolder)
    {
        int file_selected = i + Scroll -FB_Handle->Dir_Count;
        
        struct FILEINFO * File = &FB_Handle->File[file_selected];
        FB_GetExt(File->Name, ext, EXT_NAME_LEN);	
        
        if(Load_CustomIcon(File->Name, FB_Handle->Current_Dir, Object))
            return 1;
        
        //if no custom icon, then load default icons
        if(UI_Style.IconsWd==NDS_ICON_WIDTH && UI_Style.IconsHt == NDS_ICON_HEIGHT)//if icon sizes in the browser are the same as nds icons
        {//load nds icon
			int compressed = 0;
			if(!strcasecmp(sfiles_exec[NDS_EXT], ext))//standard nds file
				compressed = 0;
			else if(!strcasecmp(sfiles_exec[NZIP_EXT], ext))//compressed nds file
				compressed = 1;
			else//not an nds format
				compressed = -1;
								
			if(compressed > -1)
			{
				char filepath[MAX_PATH];
				memset(filepath, 0, sizeof(filepath));
				strcpy(filepath, FB_Handle->Current_Dir);
				strcat(filepath, File->Name);
				
				BAG_Display_DeleteObj( Object);
				BAG_CreateBlankObj( Object, 16, NDS_ICON_WIDTH, NDS_ICON_HEIGHT, NDS_ICON_WIDTH, NDS_ICON_HEIGHT);
				
				int rtrn = 0;
				switch(compressed)
				{
					case 0: rtrn = NDS_ReadIconEx(filepath, Object, Main_Settings.ndsTrans);break;
					case 1:rtrn = NZIP_ReadIconEx(filepath, Object, Main_Settings.ndsTrans);break;
				}
				
				if(rtrn > 0)
					return 2;
			}
        }
 
        //if not an nds file, then use default icons
        if(Load_DefaultIcon(ext, Object) > 0)
            return 3;
            
        //load unknown icon
        BAG_Display_CloneObj(GFX->Icons,  Object);
        BAG_SetObjFrame(Object, FRAME_VERT, 2);
        return 4;
    }
    else
    {
        int file_selected = i + Scroll;
        struct FILEINFO * File = &FB_Handle->Dir[file_selected];
        BAG_Display_CloneObj(GFX->Icons, Object);
        if(!strcmp("..",File->Name))
            BAG_SetObjFrame(Object, FRAME_VERT, ICO_PD);       
        else
            BAG_SetObjFrame(Object, FRAME_VERT, ICO_DIR);
		return 5;
    }
	return -1;
}

int Icon_handler(GUI *ui, FileBrowser * FB_Handle,  s16 Scroll, int fullpage, int i, int isFolder)
{
	if(Scroll == oldScroll && fullpage == 0)
		return 0;
		
	guigfx * GFX = &ui->gfx;
	if(!fullpage)
	{
		if(Scroll > oldScroll)
		{
			if(i < ui->FilesPerScreen - 1)
			{
				BAG_Display_CloneObj(GFX->NDSIcon[i + 1], GFX->NDSIcon[i]);
				return 0;
			}	
		}
		else if(Scroll < oldScroll)
		{
			if(i > 0)
			{
				BAG_Display_CloneObj(GFX->NDSIcon[i - 1], GFX->NDSIcon[i]);
				return 0;
			}	
		}
	}	
	BAG_ObjRenderMode (GFX->NDSIcon[i], 0);//set default icon rendering to slow
	BAG_ObjTransCol(GFX->NDSIcon[i], ui->TransCol);
	//check for custom icon for any file
	int test = _load_Icon(GFX, GFX->NDSIcon[i], FB_Handle, Scroll, i, isFolder);
	if(test == 2 && Main_Settings.ndsTrans == 0)
		BAG_ObjRenderMode (GFX->NDSIcon[i], 1);//set fast render
		
	return test;
}

/************************
Filebrowser scrolling
************************/
struct NameCache InternalNames;

inline void Name_Cache_Add(struct NameCache * names, const char *name, int file_num)
{
	names->Cache = realloc(names->Cache, (names->count + 1) * sizeof(struct _nameCache_));
	if(names->Cache)
	{
		memset(&names->Cache[names->count], 0, sizeof(struct _nameCache_));

		names->Cache[names->count].Name = calloc(strlen(name) + 1, sizeof(u8));
		if(names->Cache[names->count].Name == NULL)
			return;
			
		strcpy(names->Cache[names->count].Name, name);
		names->Cache[names->count].file_num = file_num;
		names->Cache[names->count].isHB = 0;
		names->count++;
	}
}

inline void Name_Cache_Add_HB(struct NameCache * names, int file_num)
{
	names->Cache = realloc(names->Cache, (names->count + 1) * sizeof(struct _nameCache_));
	if(names->Cache)
	{
		memset(&names->Cache[names->count], 0, sizeof(struct _nameCache_));
		names->Cache[names->count].file_num = file_num;
		names->Cache[names->count].isHB = 1;
		names->count++;
	}
}

inline int Get_Cached_Name(struct NameCache * names, char *output, int file_num)
{
	if(names->count < 1)
		return 0;
		
	int i = 0;
	for( i = 0; i < names->count; i++)
	{
		if(file_num == names->Cache[i].file_num)
		{
			if(names->Cache[i].isHB == 1)
				return -1;
				
			strcpy(output, names->Cache[i].Name);
			return 1;
		}
	}
	return 0;
}

void Clear_Name_Cache(struct NameCache * names)
{
	if(names->Cache)
	{	
		int i = 0;
		for(i = 0; i < names->count; i++)
		{
			if(names->Cache[i].Name)
			{
				free(names->Cache[i].Name);
				names->Cache[i].Name = NULL;
			}
			names->Cache[i].file_num = 0;
			names->Cache[i].isHB = 0;
		}
	
		free(names->Cache);
		names->Cache = NULL;	
	}
	names->count = 0;
}


inline int nameCache(struct FILEINFO *File, FileBrowser *FB, char *namebuf, int file_number)
{
	char tempext[10];
	memset(tempext, 0, sizeof(tempext));
	FB_GetExt(File->Name, tempext, 10);
	
	int type = 0;
	if(!strcasecmp(tempext, sfiles_exec[NDS_EXT]))
		type = 1;
	else if(!strcasecmp(tempext, sfiles_exec[NZIP_EXT]))
		type = 2;
	else
		return 0;
		
	NDSHEADER header;
	memset(&header, 0, sizeof(NDSHEADER));

	char fullpath[MAX_PATH];
	memset(fullpath, 0, sizeof(fullpath));
	sprintf(fullpath, "%s/%s", FB->Current_Dir, File->Name); 
	
	switch(type)
	{
		case 1:NDS_ReadBanner(fullpath, NDS_ROM_LANG, &header);break;
		case 2:NZIP_ReadBanner(fullpath, &header);break;
	}
	
	if(header.type == ROM_CM)
		Name_Cache_Add(&InternalNames, header.text, file_number);
	else
		Name_Cache_Add_HB(&InternalNames, file_number);
	
	return Get_Cached_Name(&InternalNames, namebuf, file_number);
}


void Scroll_FB_Text (GUI *ui, u16 * Dest,  s16 Scroll, FileBrowser *FB_Handle, int filesel, int update_all)
{
    int files_perScreen = ui->FilesPerScreen;
    int i = 0, loopEnd = files_perScreen, loopdir = 1;
	s8 endLoop = 0;
    
    int List_Count = FB_Handle->File_Count + FB_Handle->Dir_Count;
    if(List_Count < loopEnd)
    {
        Scroll = 0;
        files_perScreen = List_Count;
    }
    	
	char TempFile[MAX_FILENAME_LEN];
	memset(TempFile,0,MAX_FILENAME_LEN);
	int textXlim = SCREEN_WIDTH;
	
	u16 Unicode[MAX_FILENAME_LEN];
	memset(Unicode, 0, sizeof(Unicode));
	guigfx * GFX = &ui->gfx;
	
	//For reversable looping
	//scroll down
	if(Scroll > oldScroll)
	{
		i = 0;
		loopEnd = files_perScreen;
		loopdir = 1;
	}
	//scroll up
	else if(Scroll < oldScroll)
	{
		i = files_perScreen -1;
		loopEnd = -1;
		loopdir = -1;
	}
	
	int txtheight = BAG_GetCharHt('g', &GFX->Font);  
	struct FILEINFO *File = NULL;	   
	while ( i != loopEnd)
	{
		BAG_UpdateAudio();
        int filenum = i + Scroll, isFolder = 0;
		if( filenum >= List_Count)
			break;
			
		
		if(filenum < FB_Handle->Dir_Count)
		{
			File = &FB_Handle->Dir[filenum];
			isFolder++;
		}
		else if(FB_Handle->File_Count > 0)
			File = &FB_Handle->File[filenum - FB_Handle->Dir_Count];
		
		s16 TextCol =  ui->FontCol;
		s16 SelX = 0, TempNameLen = (ui->DispNameLen >> 2), centerName = 0;
		if (i + Scroll == filesel)
		{
			TextCol = ui->FontSelCol;
			SelX = 6;
			TempNameLen = ui->DispNameLen;
			centerName++;
		}
		strcpy(TempFile, File->Name);
	  
		int len = strlen(TempFile);
		  //strip file extensions
	   if(!ui->ShowExt && isFolder == 0)
			len -= FB_StripExt(TempFile);

		while(len > TempNameLen)
			TempFile[len--] = '\0';
		
		int strrow = 0;	
		//Folders / directories
		if(isFolder)
		{
			if(centerName)
			{
				int len = BAG_GetStrWd((unsigned char*)TempFile, &GFX->Font);
				if(ui->TextX + SelX + len > textXlim)
					strrow = -(BAG_GetStrHt((unsigned char*)TempFile, &GFX->Font)>>1);
			}			
		
			if(!strcmp("..",File->Name))
				strcpy(TempFile, "<-PrevDir");
			
			utf8_to_unicode16(TempFile, Unicode);
			BAG_UniBmpPrintScrn(Dest, ui->TextX + SelX, ui->TextY + (i * ui->TextDiff) + strrow, textXlim, 192, Unicode, &GFX->Font, TextCol,TempNameLen);
		}
		
		//file type
		else
		{
			int getName = 0, newlines = 0;
			//print filename
			if(Main_Settings.ndsInternalName)
			{//use internal nds file name
				int file_number = filenum - FB_Handle->Dir_Count;
				getName = Get_Cached_Name(&InternalNames, TempFile, file_number);
				if( getName == 0)
				{			
					getName = nameCache(File, FB_Handle, TempFile, file_number);
					int templen = strlen(TempFile);
					if(templen < TempNameLen)
						TempNameLen = templen;
				}
			}
			
			if(centerName && (getName == -1 || getName == 0))
			{
				int len = BAG_GetStrWd((unsigned char*)TempFile, &GFX->Font);
				if(ui->TextX + SelX + len > textXlim)
					strrow = -(txtheight>>1);
			}
			if(getName == 1)
			{
				int len= TempNameLen = strlen(TempFile);
				int k = 0;
				int newline_points[5];
				while(k < len)
				{
					if(TempFile[k] == '\n')
						newline_points[newlines++] = k;
						
					k++;
				}
				while((txtheight * newlines) >= ui->IconsHt)
				{
					newlines--;
					TempNameLen = newline_points[newlines] - 1;
				}		
				strrow = -((txtheight>>1) * (newlines));
				BAG_BmpPrintScrn(Dest, ui->TextX + SelX, ui->TextY + (i * ui->TextDiff) + strrow, textXlim, 192, (unsigned char*)TempFile, &GFX->Font, TextCol,TempNameLen);
			}
			if(getName == -1 || getName == 0)
			{
				utf8_to_unicode16(TempFile, Unicode);
				BAG_UniBmpPrintScrn(Dest, ui->TextX + SelX, ui->TextY + (i * ui->TextDiff) + strrow, textXlim, 192, Unicode, &GFX->Font, TextCol,TempNameLen);
			}
		}

		Icon_handler(ui, FB_Handle, Scroll, update_all, i, isFolder);
		BAG_SimpleDrawObj(GFX->NDSIcon[i], Dest, ui->IconsX + SelX, ui->IconsY + (ui->IconsDiff * i));
		i += loopdir;
	}
	oldScroll = Scroll;
}

/************************
Main render function
************************/
void SetStylusCursor(UI_FB * gui, int y)
{
	UI_State *state = &gui->State;
	FileBrowser *FB = gui->FB_Dir;
	
	state->Cursor_Scroll = y / UI_Style.IconsDiff;
	while(state->Cursor_Scroll > (FB->File_Count + FB->Dir_Count) -1)
		state->Cursor_Scroll--;	
		
	while(state->Cursor_Scroll > state->FilesPerScreen - 1)
		state->Cursor_Scroll--;
				
		state->File_Selected = state->List_Scroll + state->Cursor_Scroll;
}

int StylusScrollDown(UI_FB * gui)
{
	UI_State *state = &gui->State;
	FileBrowser *FB = gui->FB_Dir;
	
	//scroll down arrow
	if(BAG_ObjTouched(UI_Style.gfx.Arrow[1]))
	{
		if(state->List_Scroll + state->FilesPerScreen -1  < (FB->File_Count +FB->Dir_Count) -1)
			state->List_Scroll++;
			
		state->File_Selected = state->List_Scroll + state->Cursor_Scroll;
		return 2;
	}
	return 0;
}

int StylusScrollUp(UI_FB * gui)		
{
	UI_State * state = &gui->State;
	//scroll up arrow
	if(BAG_ObjTouched(UI_Style.gfx.Arrow[0]))
	{
		if(state->List_Scroll > 0)
			state->List_Scroll--;
			
		state->File_Selected = state->List_Scroll + state->Cursor_Scroll;
		return 2;
	}	
	return 0;
}

int StylusFun(void)
{	

	UI_FB * gui = Main_Browser;
	UI_State * state = &gui->State;
	
	int rtrn = 0;
	 if(Stylus.Newpress)
	{
		if (BAG_StylusZone(24, 0, 256, 192))
		{
			SetStylusCursor(gui, Stylus.Y);
			rtrn = 2;
		}
		
		rtrn += StylusScrollUp(gui);
		rtrn += StylusScrollDown(gui);
		
		if(BAG_ObjTouched(UI_Style.gfx.SettingsIcon))
		{
			rtrn = SETTINGS_ESCAPE;
			return rtrn;
		}
		
		else if(BAG_ObjTouched(UI_Style.gfx.LightIcon))
			Change_Brightness();
			
		else if(BAG_ObjTouched(UI_Style.gfx.FavoriteIcon))
		{
			rtrn = FAVORITE_ESCAPE;			
			return rtrn;
		}
        else if(MusicPlayer->BGplay && BAG_ObjTouched(UI_Style.gfx.MusicIcon))
        {
            rtrn = MUSIC_ESCAPE;
			return rtrn;
        }
		else if(UI_Style.gfx.ShowNzipIcon && BAG_ObjTouched(UI_Style.gfx.NzipIcon))
		{
			rtrn = NZIP_MANAGER_ESCAPE;
			return rtrn;
		}
	}
	if(Stylus.DblClick)
	{
		if (BAG_StylusZone(24, 0, 256, 192) && state->File_Selected == state->OldFile)
		{
			rtrn = 1;
			BAG_ResetDblClick();
			return 1;
		}
	}	
	else if(Stylus.Held)
	{
		if (BAG_StylusZone(24, 0, 256, 192))
		{
			SetStylusCursor(gui, Stylus.Y);	
			rtrn = 2;
		}
		rtrn += StylusScrollUp(gui);
		rtrn += StylusScrollDown(gui);
	}
	
	state->OldFile = state->File_Selected;
	if(MusicPlayer->minimize == 1)
		rtrn = 2;
		
	return rtrn;
}


void DrawGfx( u16 * screen, UI_FB * gui, int update_all)
{
	UI_State * state = &gui->State;
	FileBrowser * FB = gui->FB_Dir;
	//ds2_clearScreen(DOWN_SCREEN,RGB15(31,31,31));
		
	BAG_FastDrawObj (UI_Style.gfx.BG[0], screen, 0, 0);
	
	//move cursor
	FB_Cursor(&UI_Style, screen, state->Cursor_Scroll);
	
	if(MusicPlayer->BGplay)
		BAG_Audio_UpdateAudio();    
	//scroll list if needed	
	Scroll_FB_Text (&UI_Style, screen,  state->List_Scroll,  FB, state->File_Selected, update_all);
	
	//draw sidebar
	BAG_DrawObj(UI_Style.gfx.SideBar, screen, 0, 0);
	int arrowx =(UI_Style.gfx.SideBar->Width > 0) ? (UI_Style.gfx.SideBar->Width >> 1) - (UI_Style.gfx.Arrow[0]->Width >> 1) : 0;
	BAG_DrawObj(UI_Style.gfx.Arrow[0], screen, arrowx, 0);
	BAG_DrawObj(UI_Style.gfx.Arrow[1], screen, arrowx, SCREEN_HEIGHT -UI_Style.gfx.Arrow[1]->Height );
	
	if(MusicPlayer->BGplay)
		BAG_Audio_UpdateAudio();   	
		
	//sidebar icons
	int y = UI_Style.gfx.Arrow[0]->Height + 4;
	BAG_DrawObj(UI_Style.gfx.SettingsIcon, screen, 0, y);
	y += UI_Style.gfx.SettingsIcon->Height + 4;
	BAG_DrawObj(UI_Style.gfx.LightIcon, screen, 0, y);
	y += UI_Style.gfx.LightIcon->Height + 4;
	BAG_DrawObj(UI_Style.gfx.FavoriteIcon, screen, 0, y);
    y += UI_Style.gfx.FavoriteIcon->Height + 4;
    if(MusicPlayer->BGplay)
	{
        BAG_DrawObj(UI_Style.gfx.MusicIcon, screen, 0, y);
		y += UI_Style.gfx.MusicIcon->Height + 4;
	}
	if(UI_Style.gfx.ShowNzipIcon)
		BAG_DrawObj(UI_Style.gfx.NzipIcon, screen, 0, y);
}



void Render_DirGfx(u16 * screen)
{
    DrawGfx(screen, Main_Browser, 1);
}

int Draw_Dir(UI_FB * gui)
{
	FileBrowser * FB = gui->FB_Dir;

	if(FB_isNewDir(FB))
	{
		oldScroll = -1;
		icon_refresh = 1;
		Clear_Name_Cache(&InternalNames);
	}
	
	if(FBUI_ifPagingScroll(gui))
		icon_refresh = 1;
		
	DrawGfx( down_screen_addr, gui, icon_refresh);
	Flip_Screen_Fun(DOWN_SCREEN, 0);
	icon_refresh = 0;
	return 1;
}

/*========================================================================
Notification Window
========================================================================*/
int Unicode_Notification_Window(u16 * title, u16 * msg[], int lines, int choices)
{
	BAG_RmSync(&Background_Player);
    BAG_UpdateIN();
	u16 * Screen = (u16*)down_screen_addr;
    
	char filename[MAX_PATH];
	sprintf(filename, "%s/skin.ini", UI_Style.Current_Skin);
	
	windowObj warning_window;
	windowObj *window = &warning_window;
	Set_Window_Font (window, &UI_Style.gfx.Font);
	Load_UniWindow_Ini(window, "Window Settings", title, filename);	
	
	int barHt = 16;
    int txtheight = BAG_GetCharHt('g', warning_window.font);
    int iconheight = (UI_Style.gfx.WindowConfirm->Height > UI_Style.gfx.WindowCancel->Height) ? UI_Style.gfx.WindowConfirm->Height : UI_Style.gfx.WindowCancel->Height;
	int old_width  = BAG_UniGetStrWd(title, window->font);
	int j = 0;
	for (j = 0; j < lines; j++)
	{
		int width = BAG_UniGetStrWd(msg[j], window->font);
		if( width > old_width)
			old_width = width;
	}	
		
	int boxwd = old_width + 32,
		 boxht = (lines * txtheight) + barHt + iconheight + txtheight;
		 
	Set_Window_Dim2(window, barHt, (256>>1) - (boxwd >> 1), (192>>1) - (boxht >> 1), boxwd, boxht);
	Create_Window_Scrn(window, DOWN_SCREEN);
	Draw_Window(window);
	
	//write the message
	int i = 0;
	int TextStartY=window->y + barHt + (txtheight >>1);
	for( i = 0; i < lines; i++)
	{
		int TextStartX = (window->x + (window->width >> 1)) - (BAG_UniGetStrWd(msg[i], window->font) >> 1);

		BAG_UniBmpPrintScrn(Screen, 
								TextStartX , 
								TextStartY, 
								window->x + window->width - 8,
								window->y + window->height,
								msg[i], 
								window->font, 
								window->FontCol,
								-2);	
		TextStartY += txtheight;
	}
	//draw buttons
	int buttonWd = (choices == 2) ? (UI_Style.gfx.WindowConfirm->Width << 1) + 6:UI_Style.gfx.WindowConfirm->Width;
	int buttonX = window->x + (window->width >> 1) - (buttonWd >> 1);
	int buttony = TextStartY + txtheight + 2;
    
	BAG_DrawObj(UI_Style.gfx.WindowConfirm, Screen, buttonX, buttony - (UI_Style.gfx.WindowConfirm->Height>>1));
	if(choices  == 2)				
	{
		int xpos = UI_Style.gfx.WindowConfirm->X + UI_Style.gfx.WindowConfirm-> Width+ 6;
		BAG_DrawObj(UI_Style.gfx.WindowCancel, Screen, xpos, buttony - (UI_Style.gfx.WindowCancel->Height>>1));	
	}

						
	Flip_Screen_Fun(DOWN_SCREEN, 2);					
	int read_loop = -1;
	while(read_loop == -1)
	{
		if(Stylus.Newpress)
		{
			if(BAG_ObjTouched(UI_Style.gfx.WindowConfirm))
				read_loop = 1;
				
			else if(choices > 1 && BAG_ObjTouched(UI_Style.gfx.WindowCancel))
				read_loop = 0;
		}
		else if(Pad.Newpress.A || Pad.Newpress.Start)
			read_loop = 1;
		
		else if(Pad.Newpress.B)
			read_loop = (choices > 1) ? 0 : 1;
		
		BAG_Update();
	}
	Delete_Window(window);
	BAG_SetSync(&Background_Player);
	return read_loop;
}

/*========================================================================
Open with menu
========================================================================*/

int OpenWith_Menu(EXTINFO *info)
{
	typedef enum{
		__WINDOW_TITLE,
		__MESSAGE_CANCEL,
		__MESSAGE_FILE,
		__MESSAGE_END,
	}openwithmsgs;
	
	struct _language *msgs;
	msgs = calloc(sizeof(struct _language), 1);
	if(msgs == NULL)
		return 0;
	
	if(Language_InitBuffers(msgs, __MESSAGE_END) != 1)
	{
		free(msgs);
		msgs = NULL;
		return 0;
	}
		
	Language_FileReadStr(GlobalPaths.Language, msgs->buffer[__WINDOW_TITLE], "Misc", "open_with_window_title", NULL);
	Language_FileReadStr(GlobalPaths.Language, msgs->buffer[__MESSAGE_CANCEL], "Misc", "cancel_msg", NULL);	

	//create and load window
	char filename[MAX_PATH];
	memset(filename, 0, sizeof(filename));
	sprintf(filename, "%s/skin.ini", UI_Style.Current_Skin);

	windowObj *open_window;
	open_window = calloc(sizeof(windowObj), 1);
	if(open_window == NULL)
	{
		Language_CleanBuffers(msgs);
		free(msgs);
		msgs = NULL;
		return 0;
	}
	
	windowObj * window = open_window;
	
	Set_Window_Font (window, &UI_Style.gfx.Font);
	Load_UniWindow_Ini(window, "Window Settings", msgs->buffer[__WINDOW_TITLE], filename);
	//determine width of window
	int old_width  = BAG_UniGetStrWd(msgs->buffer[__WINDOW_TITLE], window->font);
	int j = 0;
	for (j = 0; j < info->num_programs; j++)
	{
		int width = BAG_GetStrWd((unsigned char*)info->Entry[j].Name, window->font);
		if( width > old_width)
			old_width = width;
	}	
		
    int barHt = 16;
    int txtheight = BAG_GetCharHt('g', window->font);
	int boxwd = old_width + 32,
		 boxht = ((info->num_programs + 2) * txtheight) + barHt;// + 1 for cancel and another to make it roomy
		 
	Set_Window_Dim2(window, barHt, (256>>1) - (boxwd >> 1), (192>>1) - (boxht >> 1), boxwd, boxht);

	Create_Window_Scrn(window, DOWN_SCREEN);
	void Draw_OpenList(u16 * Dest, int selected)
    {
        Draw_Window(window);

        //draw list to window
        int i = 0, 
            TextStartY= window->y + window->barHt + (txtheight>>1),
            TextStartX = 0;
        u16 Color = window->FontCol;
        for( i = 0; i < info->num_programs; i++)
        {
            Color = ( i == selected) ? window->HilightTxtCol : window->FontCol;
            TextStartX = (window->x + (window->width >> 1)) - (BAG_GetStrWd((unsigned char*)info->Entry[i].Name, window->font) >> 1);

			utf8_to_uni16(info->Entry[i].Name, msgs->buffer[__MESSAGE_FILE]);
            BAG_UniBmpPrintScrn(Dest, TextStartX , TextStartY, window->x + window->width, window->y + window->height, msgs->buffer[__MESSAGE_FILE], window->font, Color, 26);	
            TextStartY += txtheight;
        }
        //cancel
        if(selected >= info->num_programs)
            Color = window->HilightTxtCol;
        else
           Color = window->FontCol;
           
        TextStartX = (window->x + (window->width >> 1)) - (BAG_UniGetStrWd(msgs->buffer[__MESSAGE_CANCEL], window->font) >> 1);
        BAG_UniBmpPrintScrn(Dest, TextStartX , TextStartY, window->x + window->width, window->y + window->height, msgs->buffer[__MESSAGE_CANCEL], window->font, Color, 26);	
        
        Flip_Screen_Fun(DOWN_SCREEN, 0);
	}
	
    Draw_OpenList(down_screen_addr, 0);
	int loop = 1, rtrn = -1, selected = 0, oldselected = -1;
	BAG_UpdateIN();
	while(loop)
	{
		if(Stylus.Newpress || Stylus.Held)
		{
			if (BAG_StylusZone(window->x, window->y, window->x + window->width, window->y + window->height))
				selected = ((Stylus.Y - window->y - window->barHt + (txtheight>>1)) *  (info->num_programs)) / (window->height - window->barHt - (txtheight>>1));
			else
				loop = 0;
		}
		
        if(Pad.Newpress.Down)
        {
            if(selected < info->num_programs)
                selected++;
        }
        if(Pad.Newpress.Up)
        {
             if(selected > 0)
                selected--;   
        }
        
        if(Pad.Newpress.B)
        {
            rtrn = -1;
            loop = 0;
       }
       
       if(Pad.Newpress.A || (Stylus.DblClick && selected == oldselected))
       {
            if(selected >= info->num_programs)
                rtrn = -1;
            else
                rtrn = selected;
                
            loop = 0;
        }
		if(MusicPlayer->minimize || selected != oldselected)
		{
			 Draw_OpenList(down_screen_addr, selected);  
			 oldselected = selected;
		}
                
		BAG_Update();
	}
	Language_CleanBuffers(msgs);
	free(msgs);
	msgs = NULL;
	
	Delete_Window(open_window);
	free(open_window);
	open_window = NULL;
	BAG_UpdateIN();
	return rtrn;
}




/*========================================================================
clibboard interface
========================================================================*/
int FolderOperation_Display(struct _clip_board * cb, const char *destination, int action)
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

    Render_DirGfx(down_screen_addr);

	char filename[256];
	sprintf(filename, "%s/skin.ini", UI_Style.Current_Skin);
	//create the window
	windowObj ops_window;
    memset(&ops_window, 0, sizeof(windowObj));
    windowObj * window = &ops_window;
    
	if(action == DIRWALK_COPY)
		Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__WINDOW_TITLE], "Clip Board", "file_action_title0", NULL);
	else if(action == DIRWALK_CUT)
		Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__WINDOW_TITLE], "Clip Board", "file_action_title1", NULL);
	else if(action == DIRWALK_DEL)
		Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__WINDOW_TITLE], "Clip Board", "file_action_title2", NULL);
	  
	//get source dir file counts
	char source[MAX_PATH];
    memset(source, 0, sizeof(source));
	strcpy(source, cb->File_Path);
    strcat(source,"\0");
	
    //pre scan to get information
	struct _dir_info SrcDir, CurrentDir;
	memset(&SrcDir, 0, sizeof(struct _dir_info));
	_directory_walk_init(source, NULL, DIRWALK_COUNT, &SrcDir);
	while(!_directory_walk(&SrcDir));
    
	Set_Window_Font (window, &UI_Style.gfx.Font);
	Load_UniWindow_Ini(window, "Window Settings", msgs.buffer[__WINDOW_TITLE], filename);
    
	//determine width of window
	int old_width  = BAG_UniGetStrWd(msgs.buffer[__WINDOW_TITLE], window->font);
    
	Language_FileReadInt(GlobalPaths.Language, msgs.buffer[__MESSAGE0], "Clip Board", "folder_action_msg1", SrcDir.File_Count, SrcDir.Folder_Count);
    int tempWd =  BAG_UniGetStrWd(msgs.buffer[__MESSAGE0], window->font);
    if(tempWd > old_width)
        old_width = tempWd;
		
    int barHt = 16;
    int txtheight = BAG_GetCharHt('g', window->font);
    int lines_of_Txt = 2;
      
    struct Progress_Bar bar;
	memset(&bar, 0, sizeof(struct Progress_Bar));
    ProgressBar_Reset(&bar);
    bar.height = txtheight + 2;
    bar.fnt = window->font;
    bar.text_height = txtheight;
    bar.boarder_col = window->BoarderCol;
    bar.fill_col = window->HilightTxtCol;
    bar.font_col = window->FontCol;
    
   
	int boxwd = old_width + 32,
		 boxht = (lines_of_Txt * (txtheight + 2)) + barHt +2 + bar.height + 6;//3 lines of text + progress bar

    Set_Window_Dim2(window, barHt, (256>>1) - (boxwd >> 1), (192>>1) - (boxht >> 1), boxwd, boxht);
	Create_Window_Scrn(window, DOWN_SCREEN);
    
    bar.x = window->x + 5;
    bar.y = window->y + barHt + 2 + ((lines_of_Txt) * (txtheight + 2)); 	
    bar.width = window->width - 10;

	Language_FileReadInt(GlobalPaths.Language, msgs.buffer[__MESSAGE0], "Clip Board", "folder_action_msg0", SrcDir.File_Count, SrcDir.Folder_Count);
	char tempbuf[MAX_PATH];
	memset(tempbuf, 0, sizeof(tempbuf));
	ini_gets("Clip Board", "folder_action_msg1", "no lang", tempbuf, MAX_PATH, GlobalPaths.Language);
	char TempStr[MAX_PATH];
	memset(TempStr, 0, sizeof(TempStr));
	//redraw the window
	void Update_Info(void)
	{
		Draw_Window(window);	
		int TextStartX = (window->x + (window->width >> 1)) - (BAG_UniGetStrWd(msgs.buffer[__MESSAGE0], window->font) >> 1);
		int TextY = window->y + window->barHt + 2;
		
		BAG_UniBmpPrintScrn(down_screen_addr, TextStartX , TextY, window->x + window->width - 8, window->y + window->height, msgs.buffer[__MESSAGE0], window->font, window->FontCol, -2);
		TextY += txtheight + 2;

		sprintf(TempStr, tempbuf, SrcDir.File_Count - CurrentDir.File_Count, SrcDir.Folder_Count - CurrentDir.Folder_Count);
		utf8_to_uni16(TempStr, msgs.buffer[__MESSAGE1]);
		TextStartX = (window->x + (window->width >> 1)) - (BAG_UniGetStrWd(msgs.buffer[__MESSAGE1], window->font) >> 1);
		BAG_UniBmpPrintScrn(down_screen_addr, TextStartX , TextY, window->x + window->width - 8, window->y + window->height, msgs.buffer[__MESSAGE1], window->font, window->FontCol, -2);
		
        ProgressBar(&bar);
	}
	void Update_Bar(void)
	{
        if(action == DIRWALK_COPY || action == DIRWALK_CUT)
        {
            
            bar.total = SrcDir.total_bytes;
            if(CurrentDir.isCopying)
            {
                if(CurrentDir.copy.done == 0)
                    bar.count += CurrentDir.copy.read_size;
            }

        }
        else if(action == DIRWALK_DEL)
        {
            bar.total = SrcDir.Folder_Count + SrcDir.File_Count;
            bar.count = CurrentDir.Folder_Count + CurrentDir.File_Count;
        }
        
        bar.dest = down_screen_addr;	
	}
    //do the file operation
	memset(&CurrentDir, 0, sizeof(struct _dir_info));
	_directory_walk_init(source, destination, action, &CurrentDir);
	int update = Main_Settings.copy_display_update;
	
    do
	{
		BAG_UpdateAudioBuffers();
		Update_Bar();
		if(update >= Main_Settings.copy_display_update)
		{
			BAG_UpdateAudioBuffers();
			Update_Info();
			Flip_Screen_Fun(DOWN_SCREEN, 1);	
			update = 0;
		}          
		
        BAG_UpdateIN();
		update++;
	}while(!_directory_walk(&CurrentDir));

	Language_CleanBuffers(&msgs);
	ProgressBar_Reset(&bar);
    Directory_Clear(&CurrentDir);
    Directory_Clear(&SrcDir);
	BAG_UpdateIN();
	Delete_Window(window);
    window=NULL;
	return 0;
}

int FileOperation_Display(struct _clip_board * cb, const char *destination)
{
	u16 uniTitle[MAX_PATH];
	memset(uniTitle, 0, sizeof(uniTitle));
	
    Render_DirGfx(down_screen_addr);

	char filename[256];
	sprintf(filename, "%s/skin.ini", UI_Style.Current_Skin);
	//create the window
	windowObj ops_window;
    memset(&ops_window, 0, sizeof(windowObj));
    windowObj * window = &ops_window;
    
	if(GET_FLAG(cb->Flags, CB_COPY))
		Language_FileReadStr(GlobalPaths.Language, uniTitle, "Clip Board", "file_action_title0", NULL);
	else if(GET_FLAG(cb->Flags, CB_CUT))
		Language_FileReadStr(GlobalPaths.Language, uniTitle, "Clip Board", "file_action_title1", NULL);
	  
	//get source dir file counts
	char source[MAX_PATH];
    memset(source, 0, sizeof(source));
	strcpy(source, cb->File_Path);
    strcat(source,"\0");
	
	Set_Window_Font (window, &UI_Style.gfx.Font);
	Load_UniWindow_Ini(window, "Window Settings", uniTitle, filename);
    
	//determine width of window
	int old_width  = BAG_UniGetStrWd(uniTitle, window->font);
	FB_GetFileFromPath(filename, cb->File_Path);
    int tempWd =  BAG_GetStrWd((unsigned char*)filename, window->font);
    if(tempWd > old_width)
        old_width = tempWd;
	if(old_width > SCREEN_WIDTH)
		old_width = SCREEN_WIDTH;
		
    int barHt = 16;
    int txtheight = BAG_GetCharHt('g', window->font);
    int lines_of_Txt = 1;
 
    struct Progress_Bar bar;
	memset(&bar, 0, sizeof(struct Progress_Bar));
    ProgressBar_Reset(&bar);
    bar.height = txtheight + 2;
    bar.fnt = window->font;
    bar.text_height = txtheight;
    bar.boarder_col = window->BoarderCol;
    bar.fill_col = window->HilightTxtCol;
    bar.font_col = window->FontCol;
    
   
	int boxwd = old_width + 32,
		 boxht = (lines_of_Txt * (txtheight + 2)) + barHt +2 + bar.height + 6;//3 lines of text + progress bar

    Set_Window_Dim2(window, barHt, (256>>1) - (boxwd >> 1), (192>>1) - (boxht >> 1), boxwd, boxht);
	Create_Window_Scrn(window, DOWN_SCREEN);
    
    bar.x = window->x + 5;
    bar.y = window->y + barHt + 2 + ((lines_of_Txt) * (txtheight + 2)); 	
    bar.width = window->width - 10;

    u16 Unicode[MAX_NAME_LEN];
	memset(Unicode, 0, sizeof(Unicode));
	utf8_to_unicode16(filename, Unicode);
	//init copying
	struct _copy_info copy;
	memset(&copy, 0, sizeof(struct _copy_info));
	if( GET_FLAG(cb->Flags, CB_COPY))
		File_Copy(destination, cb->File_Path, &copy);
	else
		File_Cut(destination, cb->File_Path, &copy);
	
	//redraw the window
	void Update_Info(void)
	{
		Draw_Window(window);

		int TextStartX = (window->x + (window->width >> 1)) - (BAG_UniGetStrWd(Unicode, window->font) >> 1);
		int TextY = window->y + window->barHt + 2;
		
		BAG_UniBmpPrintScrn(down_screen_addr, TextStartX , TextY, window->x + window->width - 8, window->y + window->height, Unicode, window->font, window->FontCol, -2);
		TextY += txtheight + 2;

        ProgressBar(&bar);
	}
	void Update_Bar(void)
	{
        bar.total = copy.file_size;
		bar.count = copy.bytes_read;
                
        bar.dest = down_screen_addr;
	}
	
	int update = Main_Settings.copy_display_update;
    do
	{
		BAG_UpdateAudioBuffers();
		Update_Bar();
		if(update >= Main_Settings.copy_display_update)
		{
			BAG_UpdateAudioBuffers();
			Update_Info();
			Flip_Screen_Fun(DOWN_SCREEN, 1);	
			update = 0;
		}
        BAG_UpdateIN();
		update++;
	}while(!File_Update(&copy));

	ProgressBar_Reset(&bar);
	BAG_UpdateIN();
	Delete_Window(window);
    window=NULL;
	return 0;
}

#define ISNDS 1
#define ISNDSCM 4
#define ISAUDIO 2
#define MAX_OPTIONS 11

struct _fileOpsData
{
	struct _clip_board *cb;
	UI_FB *gui;
	struct _language msgs;
	FileBrowser *fb;
	struct FILEINFO *File;
	char SourceFile[MAX_PATH];
	u8 isDir, file_type, ndsCompressed;
	char FileOps[MAX_OPTIONS][32];
	char section[32];
	int NUM_OPTIONS, selected;
};

int init_FileOpsMenu(struct _fileOpsData *fops, struct _clip_board *cb, UI_FB *gui)
{
	if(fops == NULL || cb == NULL || gui == NULL)
		return 0;

	fops->cb = cb;
	fops->gui = gui;

	memset(&fops->msgs, 0, sizeof(struct _language));
	if(Language_InitBuffers(&fops->msgs, MAX_OPTIONS) != 1)
		return 0;

	memset(fops->SourceFile, 0, sizeof(fops->SourceFile));
	
	fops->fb = fops->gui->FB_Dir;
    strcpy(fops->SourceFile, fops->fb->Current_Dir);
    
    int file = fops->gui->State.File_Selected;
    int dir_count = fops->fb->Dir_Count;
    if(file < dir_count)
	{
		fops->isDir = 1;
        fops->File = &fops->fb->Dir[file];
	}
    else
        fops->File = &fops->fb->File[file - dir_count];
	
    strcat(fops->SourceFile, fops->File->Name);
	
	if(!strcasecmp(fops->File->Name, ".") || !strcasecmp(fops->File->Name, ".."))
	{
		Language_CleanBuffers(&fops->msgs);
		return 0;
	}
	
	return 1;
}

void filetype_FileOpsMenu(struct _fileOpsData *fops)
{
	//determine if special options can be unlocked
	fops->file_type = 0;//generic file type
	fops->ndsCompressed = 0;
	if(!fops->isDir)
	{
		char ext[EXT_NAME_LEN];
		memset(ext, 0, sizeof(ext));
		
		FB_GetExt(fops->File->Name, ext, sizeof(ext));
		
		//check if nds or otherwise
		if(!strcasecmp(ext, sfiles_exec[NDS_EXT]))
		{
			fops->ndsCompressed = 0;
			fops->file_type = ISNDS;
			NDSHEADER nds;
			NDS_ReadTitle(fops->SourceFile, &nds);
				
			if(nds.type == ROM_CM)
				fops->file_type = ISNDSCM;	
		}
		else if(!strcasecmp(ext, sfiles_exec[NZIP_EXT]))
		{
			fops->ndsCompressed = 1;
			fops->file_type = ISNDS;
			NDSHEADER nds;
			NZIP_ReadTitle(fops->SourceFile, &nds);
				
			if(nds.type == ROM_CM)
				fops->file_type = ISNDSCM;	
		}			
		else
		{
			int j = 0;
			for(j = 0; j < AUDIOTYPES; j++)
			{
				if(!strcasecmp(ext, sfiles_audio[j]))
				{
					fops->file_type = ISAUDIO;
					break;
				}
			}
		}
	}
}

void CreateOptions_FileOpsMenu(struct _fileOpsData *fops)
{	
	//copy cut, delete and add to favorites menu
	strcpy(fops->section, "File Options");
    fops->NUM_OPTIONS = 0;
	
	if(!UserPerm_CheckFileOps())//user doesn't have permission to access clipboards
		ClipBoard.Flags = 0;
		
     if(!GET_FLAG(ClipBoard.Flags, CB_FULL))//if clip board is empty, then let people copy/cut/delete stuff
     {
        int m = 0;
		if(UserPerm_CheckFileOps())
		{
			strcpy(fops->FileOps[m], "file_ops_msg0");
			Language_FileReadStr(GlobalPaths.Language, fops->msgs.buffer[m], fops->section, fops->FileOps[m], NULL);m++;//copy
			strcpy(fops->FileOps[m], "file_ops_msg1");
			Language_FileReadStr(GlobalPaths.Language, fops->msgs.buffer[m], fops->section, fops->FileOps[m], NULL);m++;//cut
			strcpy(fops->FileOps[m], "file_ops_msg2");
			Language_FileReadStr(GlobalPaths.Language, fops->msgs.buffer[m], fops->section, fops->FileOps[m], NULL);m++;//delete
		}
		if(!fops->isDir)//can't favorite a dir
		{
			strcpy(fops->FileOps[m], "file_ops_msg3");
			Language_FileReadStr(GlobalPaths.Language, fops->msgs.buffer[m], fops->section, fops->FileOps[m], NULL);m++;//favorites
			strcpy(fops->FileOps[m], "file_ops_msg4");
			Language_FileReadStr(GlobalPaths.Language, fops->msgs.buffer[m], fops->section, fops->FileOps[m], NULL);m++;//properties
			strcpy(fops->FileOps[m], "file_ops_msg11");
			Language_FileReadStr(GlobalPaths.Language, fops->msgs.buffer[m], fops->section, fops->FileOps[m], NULL);m++;//hide file
		}
		else//is a directory
		{
			strcpy(fops->FileOps[m], "file_ops_msg12");
			Language_FileReadStr(GlobalPaths.Language, fops->msgs.buffer[m], fops->section, fops->FileOps[m], NULL);m++;//hide folder
		}
		
		if(fops->file_type == ISNDSCM)
		{
			strcpy(fops->FileOps[m], "file_ops_msg5");
			Language_FileReadStr(GlobalPaths.Language, fops->msgs.buffer[m], fops->section, fops->FileOps[m], NULL);m++;//EOS Settings
		}
			
		if(fops->file_type == ISAUDIO && UserPerm_CheckSettings())
		{
			strcpy(fops->FileOps[m], "file_ops_msg6");
			Language_FileReadStr(GlobalPaths.Language, fops->msgs.buffer[m], fops->section, fops->FileOps[m], NULL);m++;//set boot song
			strcpy(fops->FileOps[m], "file_ops_msg7");
			Language_FileReadStr(GlobalPaths.Language, fops->msgs.buffer[m], fops->section, fops->FileOps[m], NULL);m++;//clear boot song
		}
		strcpy(fops->FileOps[m], "file_ops_msg10");
		Language_FileReadStr(GlobalPaths.Language, fops->msgs.buffer[m], fops->section, fops->FileOps[m], NULL);m++;//cancel
		fops->NUM_OPTIONS = m;
    }
    else
    {
        int m = 0;
		strcpy(fops->FileOps[m], "file_ops_msg8");
		Language_FileReadStr(GlobalPaths.Language, fops->msgs.buffer[m], fops->section, fops->FileOps[m], NULL);m++;//paste
		strcpy(fops->FileOps[m], "file_ops_msg2");
		Language_FileReadStr(GlobalPaths.Language, fops->msgs.buffer[m], fops->section, fops->FileOps[m], NULL);m++;//delete
		strcpy(fops->FileOps[m], "file_ops_msg9");
		Language_FileReadStr(GlobalPaths.Language, fops->msgs.buffer[m], fops->section, fops->FileOps[m], NULL);m++;//clear clipboard
		strcpy(fops->FileOps[m], "file_ops_msg10");
		Language_FileReadStr(GlobalPaths.Language, fops->msgs.buffer[m], fops->section, fops->FileOps[m], NULL);m++;//cancel
		fops->NUM_OPTIONS = m;
    }
}

int hideConfirmation(const char *title)
{
	typedef enum{
		__WINDOW_TITLE,
		__MESSAGE0,
	}overwritemsgs;
	
	struct _language msgs;
	memset(&msgs, 0, sizeof(struct _language));
	if(Language_InitBuffers(&msgs, 2) != 1)
		return 0;
		
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__WINDOW_TITLE], "File Options", title, NULL);
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__MESSAGE0], "Clip Board", "delete_msg1", NULL);

    int force = Unicode_Notification_Window(msgs.buffer[__WINDOW_TITLE], (u16**)&msgs.buffer[__MESSAGE0], 1, 2);
	Language_CleanBuffers(&msgs);
	
	return force;
}

void Action_FileOpsMenu(const struct _fileOpsData *fops)
{	
	//collect all different options
	if(!strcasecmp(fops->FileOps[fops->selected], "file_ops_msg0"))//copy
		ClipBoard_Copy(fops->SourceFile, fops->cb);
	else if(!strcasecmp(fops->FileOps[fops->selected], "file_ops_msg1"))//cut
		ClipBoard_Cut(fops->SourceFile, fops->cb);
	else if(!strcasecmp(fops->FileOps[fops->selected], "file_ops_msg2"))//delete
	{
		BAG_UpdateIN();
		ClibBoard_DeleteItem(fops->File->Name, fops->SourceFile, fops->cb);

		//refresh interface
		icon_refresh = 1;
		FBUI_Refresh(fops->gui);//refresh interface after deleting files
	}
	else if(!strcasecmp(fops->FileOps[fops->selected], "file_ops_msg3"))//favorite
		Favorites_Add(fops->SourceFile);
	else if(!strcasecmp(fops->FileOps[fops->selected], "file_ops_msg8"))//paste
	{
		ClipBoard_Paste(fops->fb->Current_Dir, fops->cb);
		icon_refresh = 1;
		FBUI_Refresh(fops->gui);//refresh interface after deleting files
	}
	else if(!strcasecmp(fops->FileOps[fops->selected], "file_ops_msg9"))//clear clipboard
		ClipBoard_Clear(fops->cb);
	else if(!strcasecmp(fops->FileOps[fops->selected], "file_ops_msg4"))//properties
		 File_Info(fops->SourceFile);
	else if(!strcasecmp(fops->FileOps[fops->selected], "file_ops_msg6"))//boot song
	{
		memset(Main_Settings.bootSong, 0, sizeof(Main_Settings.bootSong));
		strcpy(Main_Settings.bootSong, fops->SourceFile);
		
		Settings_Write_Ini(&Main_Settings, GlobalPaths.Settings_ini);
	}
	else if(!strcasecmp(fops->FileOps[fops->selected], "file_ops_msg7"))//clear boot song
	{
		memset(Main_Settings.bootSong, 0, sizeof(Main_Settings.bootSong));
		Settings_Write_Ini(&Main_Settings, GlobalPaths.Settings_ini);
	}
	else if(!strcasecmp(fops->FileOps[fops->selected], "file_ops_msg5"))//eos settings
		EOS_Properties(fops->SourceFile, fops->ndsCompressed);    
	else if(!strcasecmp(fops->FileOps[fops->selected], "file_ops_msg11"))//hide file
	{
		if(hideConfirmation(fops->FileOps[fops->selected]))
		{
			TxtFile_addLine(&Main_Browser->FB_Dir->HidFileList, fops->SourceFile);//add file to internal list
			TxtFile_writeAddLine(GlobalPaths.HiddenFiles_txt, fops->SourceFile);//write file to stored list
			icon_refresh = 1;
			FBUI_Refresh(fops->gui);//refresh interface after hiding files
		}
	}
	else if(!strcasecmp(fops->FileOps[fops->selected], "file_ops_msg12"))//hide folder
	{
		if(hideConfirmation(fops->FileOps[fops->selected]))
		{
			TxtFile_addLine(&Main_Browser->FB_Dir->HidDirList, fops->SourceFile);//add file to internal list
			TxtFile_writeAddLine(GlobalPaths.HiddenFolders_txt, fops->SourceFile);//write file to stored list
			icon_refresh = 1;
			FBUI_Refresh(fops->gui);//refresh interface after hiding files
		}
	}
}

int File_OpsMenu(struct _clip_board * cb, UI_FB * gui)
{  
	struct _fileOpsData fops;
	memset(&fops, 0, sizeof(struct _fileOpsData));

	if(init_FileOpsMenu(&fops, cb, gui) == 0)
		return 0;

	filetype_FileOpsMenu(&fops);
	CreateOptions_FileOpsMenu(&fops);

	//create and load window
	char filename[MAX_PATH];
	sprintf(filename, "%s/skin.ini", UI_Style.Current_Skin);

	windowObj options_window;
    windowObj * window = &options_window;
	Set_Window_Font (window, &UI_Style.gfx.Font);
	
	u16 tempName[MAX_PATH];
	Language_FileReadStr(GlobalPaths.Language, tempName, fops.section, "file_ops_window_title", NULL);
	Load_UniWindow_Ini(window, "Window Settings", tempName, filename);
		
    //get window dimensions based on available options
	int old_width  = BAG_UniGetStrWd(tempName, window->font);
	int j = 0;
	for (j = 0; j < fops.NUM_OPTIONS; j++)
	{
		int width = BAG_UniGetStrWd(fops.msgs.buffer[j], window->font);
		if( width > old_width)
			old_width = width;
	}	
		
    int barHt = 16;
    int txtheight = BAG_GetCharHt('g', window->font);
	int boxwd = old_width + 32,
		 boxht = (( fops.NUM_OPTIONS) * txtheight) + barHt + txtheight;
		 
	Set_Window_Dim2(window, barHt, (SCREEN_WIDTH>>1) - (boxwd >> 1), (SCREEN_HEIGHT>>1) - (boxht >> 1), boxwd, boxht);
    
    u16 * Screen = (u16*)down_screen_addr;
    Render_DirGfx(Screen);
	Create_Window_Scrn(window, DOWN_SCREEN);
	
    //Draw the options list
	void Draw_Options(u16 * Dest, int selected)
    {
        Draw_Window(window);
        //draw list to window
        int i = 0, 
            TextStartY= window->y + window->barHt + (txtheight >> 1),
            TextStartX = 0;
        u16 Color = window->FontCol;
        for( i = 0; i < fops.NUM_OPTIONS; i++)
        {
            Color = ( i == selected) ? window->HilightTxtCol : window->FontCol;
            TextStartX = (window->x + (window->width >> 1)) - (BAG_UniGetStrWd(fops.msgs.buffer[i], window->font) >> 1);

            BAG_UniBmpPrintScrn(Dest, TextStartX , TextStartY, window->x + window->width, window->y + window->height, fops.msgs.buffer[i], window->font, Color, 26);	
            TextStartY += txtheight;
        }
        
        Flip_Screen_Fun(DOWN_SCREEN, 0);
	}
    
    Draw_Options(down_screen_addr, 0);
	int loop = 1, selected = 0, oldselected = -1, launch = 0;
    
	while(loop)
	{
		if(Stylus.Newpress || Stylus.Held)
		{
			if (BAG_StylusZone(window->x, window->y, window->x + window->width, window->y + window->height))
				selected = ((Stylus.Y - window->y - window->barHt) *  (fops.NUM_OPTIONS)) / (window->height - window->barHt - (txtheight>>1));
			else
				loop = 0;
		}
        if(Pad.Newpress.Down)
        {
            if(selected < fops.NUM_OPTIONS - 1)
                selected++;
        }
        if(Pad.Newpress.Up)
        {
             if(selected > 0)
                selected--;    
        }
       
       if(Pad.Newpress.A || (Stylus.DblClick && selected == oldselected))
       {
			loop = 0;
			launch = 1;
        }
		
		if(oldselected != selected || MusicPlayer->minimize)
		{
			Draw_Options(down_screen_addr, selected);  
			oldselected = selected;
		}
		
        if(Pad.Newpress.B)
		{
			launch = 0;
            loop = 0;
			break;
		}
                
		BAG_Update();
	}
	Language_CleanBuffers(&fops.msgs);
	Delete_Window(window);
	BAG_UpdateIN();
	
	if(launch == 0)
		return 0;
		
	Render_DirGfx(down_screen_addr);
	fops.selected = selected;
	Action_FileOpsMenu(&fops);
	return 0;
}


