#ifndef _SETTINGS_
#define _SETTINGS_

#ifdef __cplusplus
extern "C" {
#endif

#include "interface.h"

extern u16 cpu[14]; 
extern u16 cpuVal_old;
typedef struct _fbsettings
{
	char hidestr[6], skin_name[MAX_PATH];
	char bootSong[MAX_PATH];
	u16 cpu_default, cpu_media, cpu_bgmusic, cpu_sleep, cpu_favs, cpu_favs_bgmusic, cpu_nzip_decompress;
	int lastfolder, brightness, extensions, favs_hideTxt,
		copy_buf_len, copy_display_update,
		topscreen_refresh,
		ndsInternalName, clock24Hour,
		bootFavs,
		CombineFavsHidFiles,
		ndsTrans;
}FBSETTINGS;
extern FBSETTINGS Main_Settings;

typedef enum
{
    SETTINGS_FILE,
    SETTINGS_SKIN,
    SETTINGS_CPU,
    SETTINGS_CLOSE,
    SETTINGS_END,
}SETTINGS_LIST;


typedef enum
{
	SETTINGS_FILE_HIDEFAVFILES,
	SETTINGS_FILE_OPENLASTDIR,
    SETTINGS_FILE_SHOWEXT,
	SETTINGS_FILE_HIDEFAVSTXT,
	SETTINGS_NDS_INTERNALNAMES,
	SETTINGS_CLOCK_24HOUR,
	SETTINGS_AUTO_FAVS,
	SETTINGS_NDS_TRANS,
	SETTINGS_FILE_END,
}SETTINGS_FILELIST;

typedef enum
{
	SETTINGS_CPU_DEFAULT,
	SETTINGS_CPU_MUSIC,
	SETTINGS_CPU_BGPLAY,
	SETTINGS_CPU_SLEEP,
	SETTINGS_CPU_FAVS,
	SETTINGS_CPU_FAVS_BGMUSIC,
	SETTINGS_CPU_NZIP,
	SETTINGS_CPU_END,
}SETTINGS_CPULIST;

#define SETTINGS_MENU FB_ESCAPE3

extern FBSETTINGS Settings_Read_Ini(UI_FB * gui, GUI *skinInfo, const char *filename, bool skip_last_folder);
extern void Write_Last_Folder(const char *path);
extern int Settings_Write_Ini(FBSETTINGS * settings, const char *filename);
extern void Settings_Main(UI_FB * gui,GUI* skinInfo);
extern void Change_Brightness(void);
extern void Change_Cpu(int menu, FBSETTINGS * settings);

#ifdef __cplusplus
}
#endif

#endif
