#ifndef _DRAW_
#define _DRAW_

#include <libBAG.h>

#ifdef __cplusplus
extern "C" {
#endif



extern void Screen_Put_Pixel(u16 *Dest, s16 x, s16 y, u16 color);
extern void Screen_Draw_Line(u16* Screen, s16 x, s16 y, s16 x2, s16 y2, u16 color);
extern void Screen_Draw_LineEx(u16 *Screen, s16 basex, s16 basey, s16 endx, s16 endy, u16 color, u8 size);
extern void Screen_Draw_Rect(u16* Screen, s16 x, s16 y, s16 width, s16 height, u16 color);
extern void SetScreenCap(int method);
extern void Screen_Flip(enum SCREEN_ID screen_num, int done);


//#define Flip_Screen_Fun  Screen_Flip
#define Flip_Screen_Fun ds2_flipScreen
#ifdef __cplusplus
}
#endif

#endif
