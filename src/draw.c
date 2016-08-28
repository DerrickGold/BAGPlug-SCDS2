#include <libBAG.h>
//General direct screen drawing functions

inline void Screen_Flip(enum SCREEN_ID screen_num, int done)
{
	BAG_FlipScreen(screen_num);
}


void Screen_Put_Pixel(u16 *Dest, s16 x, s16 y, u16 color)
{
	if(x>=0 && x< SCREEN_WIDTH && y>=0 && y<SCREEN_HEIGHT)
		Dest[x+(y <<8)] = color;
}


void Screen_Draw_Line(u16* Screen, s16 x, s16 y, s16 x2, s16 y2, u16 color)
{
   	bool yLonger=false;
	int shortLen=y2-y;
	int longLen=x2-x;
	int j = 0;
	if (abs(shortLen)>abs(longLen)) 
	{
		int swap=shortLen;
		shortLen=longLen;
		longLen=swap;				
		yLonger=true;
	}
	int decInc = 0;
	
	if (longLen==0) decInc=0;
	else decInc = (shortLen << 16) / longLen;

	if (yLonger) 
	{
		if (longLen>0)
		{
			longLen+=y;
			for (j=0x8000+(x<<16);y<=longLen;++y) 
			{
				Screen_Put_Pixel(Screen, j>>16, y, color);
				j+=decInc;
			}
			return;
		}
		longLen+=y;
		for ( j=0x8000+(x<<16);y>=longLen;--y) 
		{
			Screen_Put_Pixel(Screen, j>>16, y, color);
			j-=decInc;
		}
		return;	
	}

	if (longLen>0)
	{
		longLen+=x;
		for (j=0x8000+(y<<16);x<=longLen;++x) 
		{
			Screen_Put_Pixel(Screen,x, j>>16, color);
			j+=decInc;
		}
		return;
	}
	longLen+=x;
	for ( j=0x8000+(y<<16);x>=longLen;--x) 
	{
		Screen_Put_Pixel(Screen,x, j>>16, color);
		j-=decInc;
	}

}


void Screen_Draw_LineEx(u16 *Screen, s16 basex, s16 basey, s16 endx, s16 endy, u16 color, u8 size) 
{
	if(basex < 0) basex = 0;
	if(endx> SCREEN_WIDTH) endx = SCREEN_WIDTH;
	
	if(basey<0) basey = 0;
	if(endy >SCREEN_HEIGHT)endy = SCREEN_HEIGHT;
	
	u8 low = (size >> 1) - size + 1;
	u8 high = (size >> 1) + 1;
	s16 i, j;
	s16 x1, x2, y1, y2;

	for (i = low; i < high; i++) 
	{
		for (j = low; j < high; j++) 
		{
			if ((basex + i >= 0) && (basey + j >= 0) && (basex + i < endx) && (basey + j < endy))
				Screen_Put_Pixel(Screen,basex + i, basey + j, color);
		}
	}

	for (i = low; i < high; i++) 
	{
		j = low;
		x1 = basex + i; x2 = endx + i; y1 = basey + j; y2 = endy + j;
		Screen_Draw_Line(Screen, x1, y1, x2,  y2, color);

		j = high - 1;
		x1 = basex + i; x2 = endx + i; y1 = basey + j; y2 = endy + j;
		Screen_Draw_Line(Screen, x1, y1, x2,  y2, color);
	}

	for (j = low; j < high; j++) 
	{
		i = low;
		x1 = basex + i; x2 = endx + i; y1 = basey + j; y2 = endy + j;
		Screen_Draw_Line(Screen, x1, y1, x2,  y2, color);
		i = high - 1;
		x1 = basex + i; x2 = endx + i; y1 = basey + j; y2 = endy + j;
		Screen_Draw_Line(Screen, x1, y1, x2,  y2, color);
	}
}



void Screen_Draw_Rect(u16* Screen, s16 x, s16 y, s16 width, s16 height, u16 color) 
{

	if(width>SCREEN_WIDTH)
		width = SCREEN_WIDTH;
	if(height>SCREEN_HEIGHT)
		height = SCREEN_HEIGHT;
	if(x<0)
	{
		width+=x;
		x = 0;
	}
	if(y<0)
	{
		height += y;
		y=0;
	}
	s16 i,j;
	for(i = y; i< y + height; ++i)
		for(j = x; j < x + width;++j)
			Screen_Put_Pixel(Screen, j, i, color);
		///memset64((u16*)&Screen[x + (i<<8)], (u16)color, width<<1);

}
