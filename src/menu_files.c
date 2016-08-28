#include "menu_files.h"
#include "fb_api.h"
#include "settings.h"

struct _globalLocations GlobalPaths;

void ScreenCapDirCheck(void)
{
	if(_directory_check(GlobalPaths.ScreenShots) < 1)
		_directory_create(GlobalPaths.ScreenShots);
		
	strcat(GlobalPaths.ScreenShots, "/");
}

void Global_Set_Paths(const char* root)
{
	memset(&GlobalPaths, 0, sizeof(struct _globalLocations));
	
	//root path
	strcpy(GlobalPaths.BAGPLUG_ROOT, root);
	
	//set screenshot folder
	strcpy(GlobalPaths.ScreenShots, GlobalPaths.BAGPLUG_ROOT);
	strcat(GlobalPaths.ScreenShots, "screen shots");
	ScreenCapDirCheck();
	
	//set default ext folder
	strcpy(GlobalPaths.EXT_Folder, GlobalPaths.BAGPLUG_ROOT);
	strcat(GlobalPaths.EXT_Folder, "ext/");
	
	//set skin folder
	strcpy(GlobalPaths.Skin_Root_Folder, GlobalPaths.BAGPLUG_ROOT);
	strcat(GlobalPaths.Skin_Root_Folder, "skin/");

	//set favorites txt location
	strcpy(GlobalPaths.Favorites_txt, GlobalPaths.BAGPLUG_ROOT);
	strcat(GlobalPaths.Favorites_txt, "user files/favorites.txt");
	
	//set hidden files txt
	strcpy(GlobalPaths.HiddenFiles_txt, GlobalPaths.BAGPLUG_ROOT);
	strcat(GlobalPaths.HiddenFiles_txt, "user files/hiddenfiles.txt");
	
	//set hidden folders txt 
	strcpy(GlobalPaths.HiddenFolders_txt, GlobalPaths.BAGPLUG_ROOT);
	strcat(GlobalPaths.HiddenFolders_txt, "user files/hiddenfolders.txt");
	
	//set init data file
	strcpy(GlobalPaths.InitData_dat, GlobalPaths.BAGPLUG_ROOT);
	strcat(GlobalPaths.InitData_dat, "initdata.dat");
	
	//set settings location
	strcpy(GlobalPaths.Settings_ini, GlobalPaths.BAGPLUG_ROOT);
	strcat(GlobalPaths.Settings_ini, "user files/settings.ini");
	
	//set extlink dat location
	strcpy(GlobalPaths.ExtLink_dat,"/moonshl2/extlink.dat");
	
	//set last ini
	strcpy(GlobalPaths.LastSave_ini, "/_dstwo/lastsave.ini");
	
	//set load file dat
	strcpy(GlobalPaths.LoadFile_dat, "/loadfile.dat");
	
	//set supercard global settings
	strcpy(GlobalPaths.GlobalSettings_ini, "/_dstwo/globalsettings.ini");
	
	//template dtc file
	strcpy(GlobalPaths.TemplateDTC, GlobalPaths.BAGPLUG_ROOT);
	strcat(GlobalPaths.TemplateDTC, "template.dtc");
	
	//temp langauge
	strcpy(GlobalPaths.Language, GlobalPaths.BAGPLUG_ROOT);
	strcat(GlobalPaths.Language, "language.ini");
	
	//nzip unzip list
	strcpy(GlobalPaths.UnzipList, GlobalPaths.BAGPLUG_ROOT);
	strcat(GlobalPaths.UnzipList, "user files/unzipped.txt");
	
	//cover art ini path
	strcpy(GlobalPaths.CoverArtIni, GlobalPaths.BAGPLUG_ROOT);
	strcat(GlobalPaths.CoverArtIni, "user files/CoverArt.ini");	
	
	//splash screen
	strcpy(GlobalPaths.SplashScreen, GlobalPaths.BAGPLUG_ROOT);
	strcat(GlobalPaths.SplashScreen, "splash");	
}



struct _txtFile Global_FavoritesList,
				   Global_NzipFileList,
				   Global_HiddenFilesList;

void ReadHiddenFilesList(void)
{
	TxtFile_Read(GlobalPaths.HiddenFiles_txt, &Global_HiddenFilesList);
}

void ReadFavoritesList(void)
{
	TxtFile_Read(GlobalPaths.Favorites_txt, &Global_FavoritesList);
}

void ReadNzipList(void)
{
	if(TxtFile_Read(GlobalPaths.UnzipList, &Global_NzipFileList) == 1)
	{
		if(TxtFile_getLineCount(&Global_NzipFileList) > 0)
			UI_Style.gfx.ShowNzipIcon = 1;
	}
}

void ReadAllLists(void)
{
	ReadHiddenFilesList();
	ReadFavoritesList();
	ReadNzipList();
}

void ClearLists(void)
{
	TxtFile_Clean(&Global_HiddenFilesList);
	TxtFile_Clean(&Global_FavoritesList);
	TxtFile_Clean(&Global_NzipFileList);
	struct _txtFile *mainlist = &Main_Browser->FB_Dir->HidFileList;
	TxtFile_Clean(mainlist);
}

//re-combine all lists to the main hidden file list
void RefreshMainLists(void)
{
	struct _txtFile *mainlist = &Main_Browser->FB_Dir->HidFileList;
	struct stat test;
	//now to combine lists
	int i =0, pos = 0, changed = 0;
	
	//scan hidden files list
	for( i = 0; i < TxtFile_getLineCount(&Global_HiddenFilesList); i++)
	{
		if(!lstat(Global_HiddenFilesList.buffer[i], &test))//check if file exists
		{
			mainlist->buffer = (char**)realloc(mainlist->buffer, (pos + 1) * sizeof(char*));
			mainlist->buffer[pos++] = (char*)Global_HiddenFilesList.buffer[i];
		}
		else//remove files that don't exist
		{
			changed++;
			TxtFile_removeLine(&Global_HiddenFilesList, i);i--;
		}
	}
	if(changed > 0)
	{
		TxtFile_Write(GlobalPaths.HiddenFiles_txt, &Global_HiddenFilesList, "wb");
		changed = 0;
	}
	
	
	//scan favorited files list
	i = 0;
	for( i = 0; i < TxtFile_getLineCount(&Global_FavoritesList); i++)
	{
		int exists = lstat(Global_FavoritesList.buffer[i], &test);
		if(Main_Settings.CombineFavsHidFiles && exists == 0)
		{
			mainlist->buffer = (char**)realloc(mainlist->buffer, (pos + 1) * sizeof(char*));
			mainlist->buffer[pos++] = (char*)Global_FavoritesList.buffer[i];
		}
		else if(exists)//no file exists
		{
			changed++;
			TxtFile_removeLine(&Global_FavoritesList, i);i--;
		}
	}
	if(changed > 0)
	{
		TxtFile_Write(GlobalPaths.Favorites_txt, &Global_FavoritesList, "wb");
		changed = 0;
	}
	
	if(UI_Style.gfx.ShowNzipIcon)
	{
		//scan nzip lists
		i = 0;
		for( i = 0; i < TxtFile_getLineCount(&Global_NzipFileList); i++)
		{
			if(!lstat(Global_NzipFileList.buffer[i], &test))
			{
				mainlist->buffer = (char**)realloc(mainlist->buffer, (pos + 1) * sizeof(char*));
				mainlist->buffer[pos++] = (char*)Global_NzipFileList.buffer[i];
			}
			else//no file
			{
				changed++;
				TxtFile_removeLine(&Global_NzipFileList, i);i--;		
			}
		}
		if(changed > 0)
		{
			TxtFile_Write(GlobalPaths.UnzipList, &Global_NzipFileList, "wb");
			changed = 0;
			if(TxtFile_getLineCount(&Global_NzipFileList) <= 0)
				UI_Style.gfx.ShowNzipIcon = 0;
		}
	}	
	mainlist->line_count = pos;
}

