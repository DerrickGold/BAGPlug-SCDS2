#include <libBAG.h>
#include "usermode.h"
struct _usrmode GlobalUser;

void User_SetMode(s8 * mode)
{
	GlobalUser.Permissions = 0; //reset all flags
	if(!strcmp(mode, "a"))//admin - all permissions
	{
		SET_FLAG(GlobalUser.Permissions, USRPERM_FILEOPS);
		SET_FLAG(GlobalUser.Permissions, USRPERM_BROWSER);
		SET_FLAG(GlobalUser.Permissions, USRPERM_SETTINGS);
		SET_FLAG(GlobalUser.Permissions, USRPERM_MUSICPLAYER);
	}
	else if(!strcmp(mode, "s"))//standard user - no file operations
	{
		SET_FLAG(GlobalUser.Permissions, USRPERM_BROWSER);
		SET_FLAG(GlobalUser.Permissions, USRPERM_SETTINGS);
		SET_FLAG(GlobalUser.Permissions, USRPERM_MUSICPLAYER);	
	}
	else if(!strcmp(mode, "r"))//restricted - just favorites access
	{
	
	}
}

bool UserPerm_CheckFileOps(void)
{
	return GET_FLAG(GlobalUser.Permissions, USRPERM_FILEOPS);
}

bool UserPerm_CheckBrowser(void)
{
	return GET_FLAG(GlobalUser.Permissions, USRPERM_BROWSER);
}

bool UserPerm_CheckSettings(void)
{
	return GET_FLAG(GlobalUser.Permissions, USRPERM_SETTINGS);
}

bool UserPerm_CheckMusicPlayer(void)
{
	return GET_FLAG(GlobalUser.Permissions, USRPERM_MUSICPLAYER);
}
