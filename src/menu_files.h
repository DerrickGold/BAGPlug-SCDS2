#ifndef _MENUFILES_
#define _MENUFILES_

#ifdef __cplusplus
extern "C" {
#endif

#include <libBAG.h>
#include "read_txt.h"

extern struct _txtFile Global_FavoritesList,
							   Global_NzipFileList,
							   Global_HiddenFilesList;

							   
extern void ReadAllLists(void);
extern void ReadFavoritesList(void);
extern void ClearLists(void);
extern void RefreshMainLists(void);

//global variables
extern char global_launchFile[MAX_PATH];
extern int FILE_LAUNCH_VAL;

struct _globalLocations
{
	char 		BAGPLUG_ROOT[MAX_PATH],
				ScreenShots[MAX_PATH],
				EXT_Folder[MAX_PATH],
				Skin_Root_Folder[MAX_PATH],
				Favorites_txt[MAX_PATH],
				HiddenFiles_txt[MAX_PATH],
				HiddenFolders_txt[MAX_PATH],
				InitData_dat[MAX_PATH],
				Settings_ini[MAX_PATH],
				ExtLink_dat[MAX_PATH],
				LastSave_ini[MAX_PATH],
				LoadFile_dat[MAX_PATH],
				GlobalSettings_ini[MAX_PATH],
				TemplateDTC[MAX_PATH],
				Language[MAX_PATH],
				UnzipList[MAX_PATH],
				CoverArtIni[MAX_PATH],
				SplashScreen[MAX_PATH];
};
extern struct _globalLocations GlobalPaths;

extern void Global_Set_Paths(const char* root);

#ifdef __cplusplus
}
#endif

#endif
