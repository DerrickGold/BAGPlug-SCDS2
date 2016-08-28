#ifndef _WINDOW_
#define _WINDOW_


#ifdef __cplusplus
extern "C" {
#endif

#include <libBAG.h>
#include "fb_api.h"

typedef struct _window
{
	BMPOBJ *gfx;
	u16 *BackBuf;
	BMP_FNT * font;
	char Name[64];
	u16 UniName[64];
    
    u8 screen_val, unicode;
	u16 *Screen;
	s16 barHt,x, y, width, height;
	u16 BoarderCol, BGCol, FontCol, NameCol, HilightCol, HilightTxtCol, ExtraCol;
	u8 BGAlpha;
	u8 useBG;
}windowObj;

extern void Create_Window( windowObj * window, u16 * dest);
extern void Create_Window_Scrn( windowObj * window, enum SCREEN_ID screen);
extern void Create_WindowBG_Scrn( windowObj * window, BMPOBJ * bg, enum SCREEN_ID screen);
extern void Delete_Window( windowObj * window);
extern void Draw_Window(windowObj * window);
extern void Draw_WindowEx(windowObj * window, u8 fast);
extern void Set_Window_Dim1( windowObj * window, int barHt, int x1, int y1, int x2, int y2);
extern void Set_Window_Dim2( windowObj * window, int barHt, int x1, int y1, int width, int height);
extern void Set_Window_Name( windowObj * window, const char *name);
extern void Set_Window_Colors ( windowObj * window, u16 Boarder, u16 Name, u16 BG, u16 Font, u16 hilight, u16 txthilight, u16 extra);
extern void Set_Window_Font (windowObj * window, BMP_FNT * font);
extern void Load_Window_Ini(windowObj * window, const char *menu_ini, const char *menu_name, const char *file);
extern void Load_UniWindow_Ini(windowObj * window, const char *menu_ini, const u16 *menu_name, const char *file);

#ifdef __cplusplus
}
#endif

#endif
