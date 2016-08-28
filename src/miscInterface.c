#include "draw.h"
#include "miscInterface.h"

/*=============================================================================================
Scroll bars
=============================================================================================*/
void ScrollBar_Create(struct _scrollbar *handle, BMPOBJ *graphics, int x1, int y1, int x2, int y2, int MaxCount, int DisplayCount, int scrollMulti)
{
	handle->gfx = graphics;
	handle->x1 = x1;
	handle->y1 = y1;
	handle->x2 = x2;
	handle->y2 = y2;
	handle->scrollMulti = scrollMulti;
	handle->MaxCount = MaxCount;
	handle->DisplayCount = DisplayCount;
	handle->scrollVal = 0;
}

void ScrollBar_Draw(u16 *screen, struct _scrollbar *handle, int newScroll)
{
	handle->scrollVal = newScroll;
	int height = abs(handle->y2 - handle->y1) - handle->gfx->Height;
	int divisor = handle->MaxCount - handle->DisplayCount;
	if(divisor <= 0)
		divisor = 1;
	handle->ypos = ((handle->scrollVal * height) / (divisor)) + handle->y1;
	//int xpos = (abs(handle->x2 - handle->x1)>>1) - (handle->gfx->Width>>1);
	
	BAG_DrawObj(handle->gfx, screen, handle->x1, handle->ypos);
}

int ScrollBar_Stylus(struct _scrollbar *handle)
{
	if(BAG_StylusZone(handle->x1, handle->y1, handle->x2, handle->y2))
	{
		int height = abs(handle->y2 - handle->y1) - handle->gfx->Height;
		int divisor = handle->scrollMulti - 1;
		if(divisor <= 0)
			divisor++;
			
		handle->scrollVal= ((((Stylus.Y - handle->y1) / divisor) * (handle->MaxCount - handle->DisplayCount)) / height) * handle->scrollMulti;
		
		while(handle->scrollVal + handle->DisplayCount > handle->MaxCount)
			handle->scrollVal--;
			
		while(handle->scrollVal < 0)
			handle->scrollVal++;
			
		return handle->scrollVal;
	}
	return -1;
}		


void ScrollBar_Delete(struct _scrollbar *handle)
{
	handle->gfx = NULL;
	memset(handle, 0, sizeof(struct _scrollbar));
}


/*=============================================================================================
Progress bars
=============================================================================================*/
void ProgressBar_Reset(struct Progress_Bar * bar)
{
    bar->dest = NULL;
    bar->x = bar->y = bar->width = bar->height = 0;
    bar->boarder_col = bar->fill_col = bar->font_col = 0;
    bar->count = bar->total = 1;
    bar->fnt = NULL;
    bar->text_height = 0;
}


void ProgressBar(struct Progress_Bar * bar)
{
    //hold the percentage value written to screen
	char Percentage[16];
	memset(Percentage, 0, sizeof(Percentage));
    Screen_Draw_Line(bar->dest, bar->x, bar->y, bar->x + bar->width, bar->y, bar->boarder_col);//top line
	Screen_Draw_Line(bar->dest, bar->x, bar->y + bar->height, bar->x + bar->width, bar->y + bar->height, bar->boarder_col);//bottom line
    Screen_Draw_Line(bar->dest, bar->x + bar->width, bar->y, bar->x + bar->width, bar->y + bar->height, bar->boarder_col);//right line
    Screen_Draw_Line(bar->dest, bar->x, bar->y, bar->x, bar->y + bar->height, bar->boarder_col);//left line
     
	//percentage numbers
	unsigned long long percent = 0;
    if(bar->total > 0)
        percent = ((bar->count * 100)/bar->total);

	sprintf(Percentage," %d%% ",percent);
    
    int text_width = BAG_GetStrWd((unsigned char*)Percentage, bar->fnt);
    
	s16 text_x = (bar->width>>1)-(text_width>>1) + bar->x;
	s16 text_y = (bar->height>>1) - (bar->text_height>>1) + bar->y + 1;
	//fill the inside
	unsigned long long barPos = ((percent * bar->width)/100);
    Screen_Draw_Rect(bar->dest, bar->x + 1, bar->y + 1, barPos, bar->height, bar->fill_col);
    BAG_BmpPrintScrn(bar->dest, text_x , text_y, text_y + bar->width, text_y, (unsigned char*)Percentage, bar->fnt, bar->font_col, -1);	
}