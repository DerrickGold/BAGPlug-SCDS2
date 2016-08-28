#ifndef _menu_input_
#define _menu_input_


#ifdef __cplusplus
extern "C" {
#endif

//default button escape assignments
#define SELECT_PAD FB_ESCAPE4
#define START_PAD FB_ESCAPE3
#define Y_PAD FB_ESCAPE1
#define X_PAD FB_ESCAPE2

//define menu escapes
#define FAVORITE_ESCAPE Y_PAD
#define SETTINGS_ESCAPE START_PAD
#define MUSIC_ESCAPE FB_ESCAPE5
#define FILEOPS_ESCAPE SELECT_PAD
#define NZIP_MANAGER_ESCAPE FB_ESCAPE6

#ifdef __cplusplus
}
#endif

#endif
