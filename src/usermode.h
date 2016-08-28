#ifndef _USERMODE_
#define _USERMODE_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	USRPERM_FILEOPS = (1<<0),//file operations permissions
	USRPERM_BROWSER = (1<<1),//browser permissions
	USRPERM_SETTINGS = (1<<2),//settings permissions
	USRPERM_MUSICPLAYER = (1<<3),
}_usrmode_permissions;

struct _usrmode
{
	int Permissions;
};

extern struct _usrmode GlobalUser;

extern void User_SetMode(s8 * mode);
extern bool UserPerm_CheckFileOps(void);
extern bool UserPerm_CheckBrowser(void);
extern bool UserPerm_CheckSettings(void);
extern bool UserPerm_CheckMusicPlayer(void);


#ifdef __cplusplus
}
#endif

#endif
