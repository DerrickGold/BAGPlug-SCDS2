#ifndef _MISCINTERFACE_
#define _MISCINTERFACE_

#include <libBAG.h>

#ifdef __cplusplus
extern "C" {
#endif

struct _scrollbar
{
	BMPOBJ *gfx;
	s16 x1, x2, y1, y2, DisplayCount, scrollVal, scrollMulti, ypos;
	int  MaxCount;
};

extern void ScrollBar_Create(struct _scrollbar *handle, BMPOBJ *graphics, int x1, int y1, int x2, int y2, int MaxCount, int DisplayCount, int scrollVal);
extern void ScrollBar_Draw(u16 *screen, struct _scrollbar *handle, int newScroll);
extern int ScrollBar_Stylus(struct _scrollbar *handle);
extern void ScrollBar_Delete(struct _scrollbar *handle);


struct Progress_Bar
{
    u16 * dest;
    s16 x, y, width, height;
    u16 boarder_col,
                          fill_col,
                          font_col;
                          
    unsigned long long count,
                                total;
                                
    BMP_FNT * fnt;
    int text_height;
};

extern void ProgressBar_Reset(struct Progress_Bar * bar);
extern void ProgressBar(struct Progress_Bar * bar);

#ifdef __cplusplus
}
#endif

#endif
