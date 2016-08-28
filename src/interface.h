#ifndef _INTERFACE_
#define _INTERFACE_


#ifdef __cplusplus
extern "C" {
#endif

#include <libBAG.h>
#include "fb_api.h"
#include "supportedFiles.h"
#include "fileops.h"

extern UI_FB * Main_Browser;

extern int icon_refresh;
struct _iconCache
{
    BMPOBJ Icon;
    char *ext;//string to determine which icon is cached where
};

struct _nameCache_
{
	char *Name;
	int file_num;
	int isHB;
};
extern struct NameCache InternalNames;

struct NameCache
{
	struct _nameCache_ *Cache;
	int count;
};

#define NUM_SIDE_ICONS 6
typedef struct _gfx
{
	BMP_FNT Font;
	
	BMPOBJ GUIGFX[46];
		
	BMPOBJ *Icons;
	BMPOBJ *BG[2];
	BMPOBJ *FavBG;
	BMPOBJ *Cursor;
	BMPOBJ *SideBar;
    BMPOBJ *BoxOn;
    BMPOBJ *BoxOff;
	BMPOBJ *WindowConfirm;
	BMPOBJ *WindowCancel;
    BMPOBJ *ScrollBar;
	//side bar
	BMPOBJ *SIDEICONS[NUM_SIDE_ICONS];
	BMPOBJ *Arrow[2];
	BMPOBJ *SettingsIcon;
	BMPOBJ *LightIcon;
	BMPOBJ *FavoriteIcon;
    BMPOBJ *MusicIcon;
	BMPOBJ *NzipIcon;
		
	//nzip manager
	BMPOBJ *SelectAll;
	BMPOBJ *Delete;
	BMPOBJ *Exit;
	//settings menu
	BMPOBJ * Main_Settings[10];
	
	BMPOBJ *NDSIcon[10];
    //cached icons for file types, no custom icons are cached
    struct _iconCache *Cache;
    int Cache_size;
    int Cache_count;
	
	//top screen
	BMPOBJ *TopBar;
	
	int gfxcount;
	
	u8 ShowNzipIcon;
}guigfx;


struct _coverarts
{
	BMPOBJ GFX;
	s8 useCovers, ExternalEntry, Loaded, useLocal, useFolderImage;
	
	//information on currently selected file
	char currentFile[MAX_PATH];
	char currentPath[MAX_PATH];
	
	//old path to check against
	char oldFile[MAX_PATH];
	char oldPath[MAX_PATH];
	
	//read from INI
	char coverPath[MAX_PATH];
	char coverExt[MAX_PATH];
	
	//file extension
	char fileExt[EXT_NAME_LEN];
	
	char LocalIni[MAX_PATH];
	
	char folderImg[MAX_PATH];
};

extern struct _coverarts CoverArt;

//two graphics with customizable alpha
#define NUMOFGFXALPHA 2


typedef struct _gui
{
	int FilesPerScreen, DispNameLen,
	TextX, TextY, TextDiff, 
	IconsWd, IconsHt,
	IconsX, IconsY, IconsDiff,
	CursorX, CursorWd, CursorY, CursorHt;
	guigfx gfx;
	char Current_Skin[MAX_PATH];
	
	//customizable settings for interface via ini file
	s16 FontCol, FontSelCol, FontSize, TransCol, TopBar_TxtCol;
	u8 GFXAlpha[NUMOFGFXALPHA];
    u16 ShowExt;
    char Icon_Folder[MAX_PATH];
}GUI;

extern GUI UI_Style;



extern int Load_Image( const char *filename, BMPOBJ * Object, int width, int height);
extern void ReloadUI(GUI *ui, UI_FB * fbstate);
extern void Read_Settings_Ini(UI_FB * gui, GUI *skinInfo, char *filename);
extern void SetUIStyle(GUI *ui, const char *skin_folder);

extern int StylusFun(void);

extern void Unload_GFX (GUI * ui);
extern void Render_DirGfx(u16 * screen);
extern void Render_TopScreen(void);
extern int Draw_Dir(UI_FB * gui);
extern void ForceTopScreenRefresh(void);
extern void CoverArt_UseFavorites(u8 yes);
extern void CoverArt_ExternFile(const char *file);

extern int Unicode_Notification_Window(u16 * title, u16 * msg[], int lines, int choices);
extern int Favorites_Menu(char *file);
extern void Favorites_Add(const char *file);
extern int OpenWith_Menu(EXTINFO *info);

extern int Init_IconCache(GUI *ui);
extern void Free_IconCache(GUI * ui);
extern int Load_CustomIcon(const char *file, const char *path, BMPOBJ * Object);
extern int Load_DefaultIcon(const char *ext, BMPOBJ * Object);


extern int FolderOperation_Display(struct _clip_board * cb, const char *destination, int action);
extern int FileOperation_Display(struct _clip_board * cb, const char *destination);
extern void File_Info(const char *file);
extern int File_OpsMenu(struct _clip_board * cb, UI_FB * gui);
extern void EOS_Properties(const char *rom_file, int isCompressed);

extern int Load_DaysText(void);
extern void Clear_DaysText(void);

extern void Clear_Name_Cache(struct NameCache * names);

extern int NZip_DecompressMenu(char *inputFile);
extern void Nzip_Manager(void);

#ifdef __cplusplus
}
#endif

#endif
