//ds2_main.c
#include "screen_cap.h"
#include "draw.h"

int Screen_Cap_Method = 0;

void ScreenCap(void)
{
	//check for input
	if(!(Pad.Held.L && Pad.Held.R))
		return;

	ds2_setBacklight(0);
	
	//force update the top screen
	ForceTopScreenRefresh();
	Render_TopScreen();
	//pause music
	if(MusicPlayer->BGplay)
		BAG_PauseSnd(&MusicPlayer->AudioFile, 1);

	//the screen capture process
	switch(Screen_Cap_Method)
	{
		case 0:
			Flip_Screen_Fun(DOWN_SCREEN, 0);
			BAG_Display_ScrnCap(DUAL_SCREEN, GlobalPaths.ScreenShots);
			Flip_Screen_Fun(DOWN_SCREEN, 0);	
		break;
		default:
			BAG_Display_ScrnCap(DUAL_SCREEN, GlobalPaths.ScreenShots);
		break;
	}
			
	//unpause music
	if(MusicPlayer->BGplay)
		BAG_PauseSnd(&MusicPlayer->AudioFile, 0);
		
	 ds2_setBacklight(3);
}


void SetScreenCap(int method)
{
	Screen_Cap_Method = method;
}
