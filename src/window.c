#include "window.h"
#include "minIni.h"

void Create_Window( windowObj * window, u16 * dest)
{
	window->gfx = calloc(sizeof(BMPOBJ), 1);
	if(!window->gfx)
	{
		printf("failed to allocate window\n");
		while(1);
	}
	
	BMPOBJ * GFX = window->gfx;
	int err = BAG_Display_CreateBlankObj(GFX, 16, window->width, window->height, window->width, window->height);
    if(err != NO_ERR)
	{
		printf("error: %d\n", err);
		while(1);
	}
	window->useBG = 0;
	//draw window
	int x = 0;
		
    do
    {
        int y = 0;
        do
		{
			//draw boarder
			if(y < window->barHt || y >=window->height - 1 || x < 1 || x >= window->width - 1)
            {
                if(window->BoarderCol == RGB15(31,0,31))
                    break;
                    
                //round window corners   
                if((x == 0 && y == 0) || (x == window->width -1 && y == 0))
                    BAG_Display_SetObjPix( GFX, x, y, dest[ window->x + x + ((window->y + y) * SCREEN_WIDTH)]);
				//window hilight color
                else if ((y < 2) || (x == window->width - 2 && y < window->barHt - 1) || (x > window->width - 2 && y < window->barHt))
                     BAG_Display_SetObjPix( GFX, x,  y, window->HilightCol);
				 else
					BAG_Display_SetObjPix( GFX, x,  y, window->BoarderCol);
            }
			//draw window interior
			else
			{
                if(window->BGCol == RGB15(31,0,31))
                    break;
                    
                int newcol = BAG_Effects_CalcAlpha(window->BGCol, dest[ (window->x + x) + ((window->y + y) * SCREEN_WIDTH)], window->BGAlpha);
				BAG_Display_SetObjPix( GFX, x,  y, newcol);
			}
		}while(y++ < window->height);
    }while(x++ < window->width);
		
	//draw name to window
	int titleX = 0, titleY = 0;
	switch(window->unicode)
	{
		case 0:
			titleX = (GFX->Width >> 1) - (BAG_GetStrWd((unsigned char*)window->Name, window->font) >> 1);
			titleY = (window->barHt >> 1) - (BAG_GetStrHt((unsigned char*)window->Name, window->font) >> 1);
			BAG_BmpPrint((u16*)GFX->Buffer, window->width, titleX,  3,  GFX->Width,  GFX-> Height, (unsigned char*)window->Name, window->font, window->NameCol,-2);
		break;
		case 1:
			titleX = (GFX->Width >> 1) - (BAG_UniGetStrWd(window->UniName, window->font) >> 1);
			titleY = (window->barHt >> 1) - (BAG_UniGetStrHt(window->UniName, window->font) >> 1);
			BAG_UniBmpPrint((u16*)GFX->Buffer, window->width, titleX,  3,  GFX->Width,  GFX-> Height, window->UniName, window->font, window->NameCol,-2);
		break;
	}
}

void Create_WindowBG( windowObj * window, BMPOBJ * bg, u16 * dest)
{
	window->gfx = bg;
	BMPOBJ * GFX = window->gfx;
	BAG_ObjSize(GFX, window->width, window->height);
	window->useBG = 1;
	//draw window
	int x = 0;
		
    do
    {
        int y = 0;
        do
		{
			//draw boarder
			if(y < window->barHt || y >=window->height - 1 || x < 1 || x >= window->width - 1)
            {
                if(window->BoarderCol == RGB15(31,0,31))
                    break;
                    
                //round window corners   
                if((x == 0 && y == 0) || (x == window->width -1 && y == 0))
                    BAG_Display_SetObjPix( GFX, x,  y, dest[ x + (y * SCREEN_WIDTH)]);
				//window hilight color
                else if ((y < 2) || (x == window->width - 2 && y < window->barHt - 1) || (x > window->width - 2 && y < window->barHt))
                     BAG_Display_SetObjPix( GFX, x,  y, window->BoarderCol + window->HilightCol);
				 else
					BAG_Display_SetObjPix( GFX, x,  y, window->BoarderCol);
			}
			else
				break;//don't draw interior
		}while(y++ < window->height);
    }while(x++ < window->width);
		
	//draw name to window
	int titleX = 0, titleY = 0;
	switch(window->unicode)
	{
		case 0:
			titleX = (GFX->Width >> 1) - (BAG_GetStrWd((unsigned char*)window->Name, window->font) >> 1);
			titleY = (window->barHt >> 1) - (BAG_GetStrHt((unsigned char*)window->Name, window->font) >> 1);
			BAG_BmpPrint((u16*)GFX->Buffer, window->width, titleX,  3,  GFX->Width,  GFX-> Height, (unsigned char*)window->Name, window->font, window->NameCol,-2);
		break;
		case 1:
			titleX = (GFX->Width >> 1) - (BAG_UniGetStrWd(window->UniName, window->font) >> 1);
			titleY = (window->barHt >> 1) - (BAG_UniGetStrHt(window->UniName, window->font) >> 1);
			BAG_UniBmpPrint((u16*)GFX->Buffer, window->width, titleX,  3,  GFX->Width,  GFX-> Height, window->UniName, window->font, window->NameCol,-2);
		break;
	}
}	

void Create_Window_Scrn( windowObj * window, enum SCREEN_ID screen)
{
    window->screen_val = screen;
    
    if(window->screen_val == DOWN_SCREEN)
        window->Screen = (u16*)down_screen_addr;
    else
        window->Screen = (u16*)up_screen_addr;
        
    Create_Window( window,  window->Screen);
}

void Create_WindowBG_Scrn( windowObj * window, BMPOBJ * bg, enum SCREEN_ID screen)
{
    window->screen_val = screen;
    
    if(window->screen_val == DOWN_SCREEN)
        window->Screen = (u16*)down_screen_addr;
    else
        window->Screen = (u16*)up_screen_addr;
        
    Create_WindowBG( window, bg, window->Screen);
}


void Set_Window_Dim1( windowObj * window, int barHt, int x1, int y1, int x2, int y2)
{
	window->barHt = barHt;
	window->x = x1;
	window->y = y1;
	window->width = x2 - x1;
	window->height = y2 - y1;
}

void Set_Window_Dim2( windowObj * window, int barHt, int x1, int y1, int width, int height)
{
	window->barHt = barHt;
	window->x = x1;
	if(window->x < 0)
		window->x = 0;
		
	window->y = y1;
	
	window->width = SCREEN_WIDTH;
	if(width < SCREEN_WIDTH)
		window->width = width;
		
	window->height = SCREEN_HEIGHT;
	if(height < SCREEN_HEIGHT)
		window->height = height;
}

void Set_Window_Name( windowObj * window, const char *name)
{
	strcpy(window->Name, name);
	window->unicode = 0;
}

void Set_Window_UniName( windowObj * window, const u16 * name)
{
	wcscpy(window->UniName, name);
	window->unicode = 1;
}

void Set_Window_Colors ( windowObj * window, u16 Boarder, u16 Name, u16 BG, u16 Font, u16 hilight, u16 txthilight, u16 extra)
{
	window->BoarderCol = Boarder;
	window->NameCol = Name;
	window->BGCol = BG;
	window->FontCol = Font;
    window->HilightCol = hilight;
    window->HilightTxtCol = txthilight;
    window->ExtraCol = extra;
}

void Set_Window_Font (windowObj * window, BMP_FNT * font)
{
	window->font = font;
}
				
void Delete_Window( windowObj * window)
{   
	if(window->useBG == 0)
	{
		if(window->gfx != NULL)
		{
			BAG_Display_DeleteObj(window->gfx);
			free(window->gfx);
			window->gfx = NULL;
		}
	}

	//memset(window, 0, sizeof(windowObj));
}

void Draw_WindowEx(windowObj * window, u8 fast)
{
    if(window->Screen == NULL || window->screen_val > 0)
    {
        if(window->screen_val == DOWN_SCREEN)
            window->Screen = (u16*)down_screen_addr;
        else
            window->Screen = (u16*)up_screen_addr;
    }

    BAG_FastDrawObj(window->gfx, window->Screen, window->x, window->y);
}

void Draw_Window(windowObj * window)
{
    Draw_WindowEx(window, 0);
}

//configure window
inline void Load_Window_Ini(windowObj * window, const char *menu_ini, const char *menu_name, const char *file)
{
	if(menu_name != NULL)
		Set_Window_Name( window, menu_name);
	
	int option[8];
	
	option[0] = ini_getl(menu_ini, "Window_Boarder_Col", RGB15(31,0,31), file); BAG_UpdateAudio();
	option[1] =  ini_getl(menu_ini, "Window_Title_Col", RGB15(31,0,31), file); BAG_UpdateAudio();
	option[2] = ini_getl(menu_ini, "Window_BG_Col", RGB15(31,0,31), file); BAG_UpdateAudio();
	option[3] = ini_getl(menu_ini, "Window_Font_Col", RGB15(31,0,31), file); BAG_UpdateAudio();
	option[4] = ini_getl(menu_ini, "Window_Hilight_Col", RGB15(31,0,31), file); BAG_UpdateAudio();
	option[5] = ini_getl(menu_ini, "Window_Hilight_Text_Col", RGB15(31,0,31), file); BAG_UpdateAudio();
	option[6] =   ini_getl(menu_ini, "Window_Extra_Col", RGB15(31,0,31), file); BAG_UpdateAudio();
	//set colors
	Set_Window_Colors (window, option[0], option[1], option[2], option[3], option[4], option[5], option[6]);

	window->BGAlpha = ini_getl(menu_ini, "Window_BG_Alpha", 255, file);
}

inline void Load_UniWindow_Ini(windowObj * window, const char *menu_ini, const u16 *menu_name, const char *file)
{
	if(menu_name != NULL)
		Set_Window_UniName( window, menu_name);
	
	//set colors
	int option[8];
	
	option[0] = ini_getl(menu_ini, "Window_Boarder_Col", RGB15(31,0,31), file); BAG_UpdateAudio();
	option[1] =  ini_getl(menu_ini, "Window_Title_Col", RGB15(31,0,31), file); BAG_UpdateAudio();
	option[2] = ini_getl(menu_ini, "Window_BG_Col", RGB15(31,0,31), file); BAG_UpdateAudio();
	option[3] = ini_getl(menu_ini, "Window_Font_Col", RGB15(31,0,31), file); BAG_UpdateAudio();
	option[4] = ini_getl(menu_ini, "Window_Hilight_Col", RGB15(31,0,31), file); BAG_UpdateAudio();
	option[5] = ini_getl(menu_ini, "Window_Hilight_Text_Col", RGB15(31,0,31), file); BAG_UpdateAudio();
	option[6] =   ini_getl(menu_ini, "Window_Extra_Col", RGB15(31,0,31), file); BAG_UpdateAudio();
	//set colors
	Set_Window_Colors (window, option[0], option[1], option[2], option[3], option[4], option[5], option[6]);

	window->BGAlpha = ini_getl(menu_ini, "Window_BG_Alpha", 255, file);
}

