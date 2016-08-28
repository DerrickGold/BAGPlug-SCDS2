#include <libBAG.h>
#include "draw.h"
#include "fb_api.h"
#include "music_player.h"
#include "supportedFiles.h"
#include "interface.h"
#include "window.h"
#include "settings.h"
#include "usermode.h"
#include "miscInterface.h"
#include "language.h"
#include "menu_files.h"

//previous directory folders in filebrowser listing

MPlayer *MusicPlayer;

extern void Stop_Audio(SndFile *snd);
extern void Background_Player(void);

int GetCurTrack(const char *filename, FileBrowser *fb)
{
	int i = 0;
	char listfile[MAX_PATH];
	memset(listfile, 0, sizeof(listfile));
	
	for(i = 0; i < fb->File_Count; i++)
	{
        listfile[0] = '\0';
        strcpy(listfile, fb->Current_Dir);
        strcat(listfile, fb->File[i].Name);
		if(!strcasecmp( filename, listfile))
			return i;
	}
	return 0;
}


MPlayer * LoadMediaPlayer(void)
{
	MPlayer * mp;
	mp = (MPlayer*) calloc(sizeof(MPlayer), 1);
	if(!mp)
		return NULL;
		
	mp->PlayList = FB_Init ( MAX_FILES_PER_DIR, 0);
	if(!mp->PlayList)
	{
		free(mp);
		return NULL;
	}
	
	return mp;
}

void FreeMediaPlayer(MPlayer * mp)
{
	Stop_Audio(&mp->AudioFile);
	BAG_UnloadSnd(&mp->AudioFile);
	BAG_RmSync(&Background_Player);
	if(mp->PlayList)
	{
		FB_Free (mp->PlayList);
		mp->PlayList = NULL;
	}

	free(mp);
	mp = NULL;
}

void CreatePlayList(const char *file, MPlayer *mp)
{
	//generate a list of all audio files in the directory of the launched file
	FileBrowser * FB = mp->PlayList;
	
	char *audio_types[AUDIOTYPES];
	int j = 0;
	for(j = 0; j < AUDIOTYPES; j++)
		audio_types[j] = (char*)&sfiles_audio[j];
	
	//scan directory for audio files
	char path[MAX_PATH];
    memset(path, 0, sizeof(path));
	FB_GetFilePath(path, file);
	FB_ScanDir( path, FB, AUDIOTYPES, audio_types, 0);
    QSort_Alpha(FB->File, 0, FB->File_Count-1);
	
	//get the current track number
	int i = 0;
	char listfile[MAX_PATH];
	memset(listfile, 0, sizeof(listfile));
	
	
	int FileCount = FB->File_Count;
	
	for(i = 0; i < FileCount; i++)
	{
		sprintf(listfile,"%s/%s", FB->Current_Dir,FB->File[i].Name);
		if(!strcasecmp( file, listfile))
		{
			mp->curTrack = i;
			break;
		}
	}
}


void Load_Audio(const char *filename, MPlayer *mp, int format)
{
	SndFile * File = &mp->AudioFile;
	Stop_Audio(File);
	BAG_LoadSnd(filename, File, format);
}

void Audio_Program(const char * filename, int format)
{
	CreatePlayList(filename, MusicPlayer);
	Load_Audio(filename, MusicPlayer, format);
}

void Stop_Audio(SndFile *snd)
{
	BAG_StopSnd(snd);
	BAG_UnloadSnd(snd);
}

void Play_Audio(SndFile *snd)
{
	BAG_PlaySnd(snd, 0);
}	


void New_Track(MPlayer * mp, int value)
{
	char *CurDir = mp->PlayList->Current_Dir;
	int FileCount = mp->PlayList->File_Count;
	struct FILEINFO *FileList =  mp->PlayList->File;
	
	mp->curTrack += value;
		
	if(mp->curTrack >= FileCount)
		mp->curTrack = FileCount - 1;
		
	if(mp->curTrack < 0)
		mp->curTrack = 0;
       
	char ext[EXT_NAME_LEN];
    memset(ext, 0, sizeof(ext));
    
	FB_GetExt(FileList[mp->curTrack].Name, ext, EXT_NAME_LEN);
	
	char file[MAX_PATH];
    memset(file, 0, sizeof(file));
	sprintf(file,"%s/%s", CurDir, FileList[mp->curTrack].Name);	
	
	int i = 0;
	for(i = 0; i < AUDIOTYPES; i++)
	{
		if(!strcasecmp(ext, sfiles_audio[i]))
		{	
			Load_Audio(file, mp, i);
			break;
		}
	}
	
	Play_Audio(&mp->AudioFile);
}

int Load_MediaPlayer_Gfx(MPlayer * mp, const char *folder)
{
	if(!UserPerm_CheckMusicPlayer())//user doesn't have permission to use player
		return 0;
	
    if(mp->Graphics_Loaded > 0)
        return NO_ERR;
        

    char file[MAX_PATH];
    memset(file, 0, sizeof(file));

    int gfxnum = 0;
    int err = NO_ERR;
    
    
    if(mp->BGplay)
    {
          BAG_Audio_UpdateAudio();
          BAG_Audio_UpdateAudio();
     }

    //background
    sprintf(file, "%s/media/bg", folder);
	err = Load_Image(file, &mp->GFX[gfxnum], 256, 192);
	BAG_ResetFrameDim(&mp->GFX[gfxnum]);
    mp->BG = &mp->GFX[gfxnum++];
   
    //dock
    sprintf(file, "%s/media/dock", folder);
	err = Load_Image(file, &mp->GFX[gfxnum], 256, 34);
	BAG_ResetFrameDim(&mp->GFX[gfxnum]);
    mp->Dock = &mp->GFX[gfxnum++];
    
    //ScrollBar
    mp->ScrollBar = UI_Style.gfx.ScrollBar;
    
    //play button
    sprintf(file, "%s/media/play", folder);
	err = Load_Image(file, &mp->GFX[gfxnum], 32, 32);
	BAG_ResetFrameDim(&mp->GFX[gfxnum]);
    mp->Play = &mp->GFX[gfxnum++];

     //pause button
    sprintf(file, "%s/media/play", folder);
	err = Load_Image(file, &mp->GFX[gfxnum], 32, 32);
	BAG_ResetFrameDim(&mp->GFX[gfxnum]);
    mp->Pause = &mp->GFX[gfxnum++];
    
    //skip forward button
    sprintf(file, "%s/media/skip", folder);
	err = Load_Image(file, &mp->GFX[gfxnum], 32, 32);
	BAG_ResetFrameDim(&mp->GFX[gfxnum]);
    mp->SkipBackward = &mp->GFX[gfxnum++];
    
        
    //skip backwards button, just a reveresed skip forward
    BAG_Display_CloneObj(&mp->GFX[gfxnum - 1], &mp->GFX[gfxnum]);
	BAG_ResetFrameDim(&mp->GFX[gfxnum]);
    BAG_FlipObj(&mp->GFX[gfxnum], FRAME_HOR);
    mp->SkipForward = &mp->GFX[gfxnum++];

    //minimize button
    sprintf(file, "%s/media/minimize", folder);
	err = Load_Image(file, &mp->GFX[gfxnum], 32, 32);
	BAG_ResetFrameDim(&mp->GFX[gfxnum]);
    mp->Minimize = &mp->GFX[gfxnum++];

    //exit button
    sprintf(file, "%s/media/exit", folder);
	err = Load_Image(file, &mp->GFX[gfxnum], 32, 32);
	BAG_ResetFrameDim(&mp->GFX[gfxnum]);
    mp->Exit = &mp->GFX[gfxnum++];
    
    mp->Graphics_Loaded = gfxnum;

         
    return err;
}

void Unload_MediaPlayer_Gfx(MPlayer * mp)
{
    int i = 0;
    for(i = 0; i < mp->Graphics_Loaded; i++)
        BAG_DeleteObj(&mp->GFX[i]);
        
    mp->Graphics_Loaded = 0;
}



void Visualizer(u16 * screen)
{
    int i = 0, textcol = 0;
	u16 Unicode[MAX_PATH];
	memset(Unicode, 0, sizeof(Unicode));
	
    do
    {
        int color =  BAG_Audio_s.Mix_Buf_Addr[i << 3] + 32767;
        int height = (color >>9);
        
        int color2=  BAG_Audio_s.Mix_Buf_Addr[(i+1)<< 3] + 32767;
        int height2 = color2 >>9;
        
        int finalcol = (color + color2)>>1;
        
        //make text color the quietest color
        textcol += color;
        textcol += color2;

        Screen_Draw_LineEx(screen, i , height + 32, i + 1, height2 + 32, finalcol, 2);

    }
    while(++i < SCREEN_WIDTH - 1);
    textcol = textcol >> 10;
	utf8_to_unicode16(MusicPlayer->PlayList->File[MusicPlayer->curTrack].Name, Unicode);
    BAG_UniBmpPrintScrn(screen, 0, 0, 256, 32, Unicode, &UI_Style.gfx.Font, textcol, 64);	
}

void newSongUpdate(void)
{
	if(GET_FLAG(MusicPlayer->AudioFile.Flags, AUDIO_STREAM_END))
		New_Track(MusicPlayer, 1);	
}

u8 Media_CreateWindow(windowObj *window)
{
	u16 WindowTitle[MAX_PATH];
	memset(WindowTitle, 0, sizeof(WindowTitle));
	Language_FileReadStr(GlobalPaths.Language, WindowTitle, "MISC", "media_player_title", NULL);

    Render_DirGfx(down_screen_addr);
	char filename[MAX_PATH];
    memset(filename, 0, sizeof(filename));
	sprintf(filename, "%s/skin.ini", UI_Style.Current_Skin);
	
	Set_Window_Font (window, &UI_Style.gfx.Font);
	Load_UniWindow_Ini(window, "Media Player", WindowTitle, filename);
	Set_Window_Dim2(window, 16, 0, 0, 256, 192);
	Create_WindowBG_Scrn(window, MusicPlayer->BG, DOWN_SCREEN);

	return 1;
}

int Media_Player(MPlayer * mp, int play)
{
	BAG_RmSync(&Background_Player);
	BAG_RmSync(&Render_TopScreen);
	
    if(Load_MediaPlayer_Gfx(mp, UI_Style.Current_Skin) != NO_ERR)
        return 0;
    
    //create media player window
	windowObj media_window;
	windowObj *window = &media_window;
	if(!Media_CreateWindow(window))
		return 0;

    //calculate number of files to display
    int max_disp = 9;//default of nine
    int  ynext = BAG_GetCharHt('g', media_window.font) + 1;
    max_disp = ((SCREEN_HEIGHT - mp->Dock->Height - media_window.barHt) / ynext) - 1;
    
	struct _scrollbar MusicScroll;
	
    void Draw_QuList(u16 * screen, int scroll, int selected, int disp_max)
    {
		Draw_Window(&media_window);
        BAG_DrawObj(mp->Dock, screen, 0, SCREEN_HEIGHT - mp->Dock->Height);
        int z = 0,
            x = media_window.x + 4,
            y = media_window.y + media_window.barHt + 1;
            
		u16 Unicode[MAX_PATH];
		
        do
        {
            if(z  > disp_max)
                break;
            int filenum = z + scroll;
			
			memset(Unicode, 0, sizeof(Unicode));
			utf8_to_unicode16(MusicPlayer->PlayList->File[filenum].Name,Unicode);
			
			u16 Color = media_window.FontCol;
            //if track isn't being played and it isn't selected
            if(MusicPlayer->curTrack != filenum && z != selected)
				Color = media_window.FontCol;
            //if track is being played and isn't selected
            else if(MusicPlayer->curTrack == filenum && z != selected)
				Color = media_window.ExtraCol;  
            //if track is selected
            else 
				Color = media_window.HilightTxtCol;  			
			 BAG_UniBmpPrintScrn(screen, x, y + (ynext * z ), media_window.x + media_window.width - mp->ScrollBar->Width, SCREEN_HEIGHT, Unicode, media_window.font, Color, -2);	
        }
        while((z++) + scroll < MusicPlayer->PlayList->File_Count -1);
		
		
        //draw buttons
        int buttonx = (media_window.width - (mp->SkipBackward->Width << 1) - mp->Play->Width - mp->Minimize->Width - mp->Exit->Width) >> 1,
             buttony = SCREEN_HEIGHT - mp->Dock->Height;
        //skip back
        BAG_DrawObj(mp->SkipBackward, screen, buttonx, buttony); buttonx += mp->SkipBackward->Width;
        //play button
        if(GET_FLAG(mp->AudioFile.Flags,AUDIO_PAUSE))
		{
            BAG_DrawObj(mp->Pause, screen, buttonx, buttony); 
			buttonx += mp->Play->Width;
		}
        else
		{
            BAG_DrawObj(mp->Play, screen, buttonx, buttony);
			buttonx += mp->Play->Width;	
		}

        //skip forward
        BAG_DrawObj(mp->SkipForward, screen, buttonx, buttony); buttonx += mp->SkipForward->Width;
        
        //exit
        BAG_DrawObj(mp->Exit, screen, buttonx, buttony); buttonx += mp->Minimize->Width;
        //minimize
        BAG_DrawObj(mp->Minimize, screen, buttonx, buttony);
        
        //draw the scroll bar
		ScrollBar_Draw(screen, &MusicScroll, scroll);
    }
    int listScroll = 0, fileSelected = 0;
    
    //launch the audio file
	mp->BGplay = 0;
	SndFile *Song = &mp->AudioFile;
	if(mp->enterBg == 0 && play)
		Play_Audio(Song);
	else
		mp->enterBg = 0;
		
	int enable = 1;
	BAG_UpdateIN();
	
	ScrollBar_Create(&MusicScroll, mp->ScrollBar, SCREEN_WIDTH - mp->ScrollBar->Width, media_window.barHt, SCREEN_WIDTH, SCREEN_HEIGHT - mp->Dock->Height, mp->PlayList->File_Count - 1, max_disp, 1);
    Draw_QuList(down_screen_addr, listScroll, fileSelected ,  max_disp);
    Flip_Screen_Fun(DOWN_SCREEN, 0);	
	 
    int cur_file = 0, old_file = 0, padTimer = 0, old_track = 0, letterht = BAG_GetCharHt('g', media_window.font);
	while(enable && !mp->BGplay)
	{
        cur_file = listScroll + fileSelected;
		//play audio file
		if(Pad.Newpress.A || (Stylus.Newpress && BAG_ObjTouched(mp->Play)))
		{
            if(cur_file == mp->curTrack)
            {
                //if audio hasn't started playing, then start it
                if(GET_FLAG(mp->AudioFile.Flags,AUDIO_STOP))
                    Play_Audio(Song);
                //if audio is playing, then pause it
                else if(GET_FLAG(mp->AudioFile.Flags,AUDIO_PLAY))
                    BAG_PauseSnd(Song, 1);
                //if audio is paused, then unpause it it
                else if(GET_FLAG(mp->AudioFile.Flags,AUDIO_PAUSE))
                    BAG_PauseSnd(Song, 0);
            }
            else
            {
                int trackDiff = cur_file - mp->curTrack;
                New_Track(mp, trackDiff);            
            }
        }
		
		newSongUpdate();
		if(Pad.Newpress.B || GET_FLAG(mp->AudioFile.Flags,AUDIO_STOP) || (Stylus.Newpress && BAG_ObjTouched(mp->Exit)))
		{
			Stop_Audio(Song);
			enable = 0;
			break;
		}
		
		if(Pad.Newpress.X || (Stylus.Newpress && BAG_ObjTouched(mp->Minimize)))
		{
			enable = 0; //exit media player but play music
			mp->BGplay = 1;
			break;
		}
		
		if(Pad.Newpress.R || (Stylus.Newpress && BAG_ObjTouched(mp->SkipForward)))
			New_Track(mp, 1);
			
		if(Pad.Newpress.L || (Stylus.Newpress && BAG_ObjTouched(mp->SkipBackward)))
			New_Track(mp, -1);
		
        
        if((Pad.Held.Up || Pad.Held.Down) && padTimer < 20)
            padTimer++;
            
        //pad controls
        if(Pad.Newpress.Down || (Pad.Held.Down && padTimer >= 20))
        {
            if(fileSelected < max_disp)
                fileSelected++;
            else
                if(listScroll + fileSelected < mp->PlayList->File_Count - 1)
                    listScroll++;
                
        }
        
        if(Pad.Newpress.Up || (Pad.Held.Up && padTimer >= 20))
        {
            if(fileSelected > 0)
                fileSelected--;
            else
                if(listScroll + fileSelected > 0)
                    listScroll--;
        }
        if(Pad.Released.Up || Pad.Released.Down)
            padTimer = 0;
        
        if(Stylus.Newpress || Stylus.Held) 
        {
			int bary = ScrollBar_Stylus(&MusicScroll);
			if(bary != -1)
				listScroll = bary;
            else if(BAG_StylusZone(media_window.x, media_window.barHt, media_window.x + media_window.width,  mp->Dock->Y))
            {
                int cursor =  (Stylus.Y - media_window.barHt) / (letterht + 1);
                fileSelected = (cursor < max_disp) ? cursor : max_disp;
            }
        }

		BAG_Update();
        if(cur_file != old_file || mp->curTrack != old_track || Pad.key != 0)
        {
            Draw_QuList(down_screen_addr, listScroll, fileSelected,  max_disp);
            Flip_Screen_Fun(DOWN_SCREEN, 0);	
            old_file = cur_file;
            old_track = mp->curTrack;
        }
		else//visualizer
        {
            Flip_Screen_Fun(UP_SCREEN, 0);
            ds2_clearScreen(UP_SCREEN,0);
            Visualizer(up_screen_addr);
        }

	}
	ScrollBar_Delete(&MusicScroll);
	mp->minimize = 1;
	ForceTopScreenRefresh();
	BAG_SetSync(&Background_Player);
	BAG_SetSync(&Render_TopScreen);
    Delete_Window(&media_window);
    Render_DirGfx(down_screen_addr);
	BAG_UpdateIN();
	return 0;
}

void Background_Player(void)
{
	MusicPlayer->minimize = 0;
	if(MusicPlayer && MusicPlayer->BGplay)//simple check if an audio file is loaded
	{
		newSongUpdate();

		if(Pad.Newpress.R)
			New_Track(MusicPlayer, 1);
		
		if(Pad.Newpress.L)
			New_Track(MusicPlayer, -1);
		
		if(Pad.Newpress.X)
		{
			MusicPlayer->enterBg= 1;
			Media_Player(MusicPlayer,0);
			BAG_UpdateIN();
		}
	}
}

void Initialize_Player(void)
{
	MusicPlayer = LoadMediaPlayer();
}

