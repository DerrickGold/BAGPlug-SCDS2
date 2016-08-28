#ifndef _MUSIC_PLAYER_
#define _MUSIC_PLAYER_

#ifdef __cplusplus
extern "C" {
#endif

#include <libBAG.h>
#include "fb_api.h"

typedef struct _mediaplayer
{
	int curTrack, BGplay, minimize, enterBg;
	FileBrowser *PlayList;
	SndFile AudioFile;	
    
    //media player graphics
    int Graphics_Loaded;
    BMPOBJ GFX[10];
    BMPOBJ  *BG,
                 *Dock,
                 *ScrollBar,
                 *Play,
                 *Pause,
                 *SkipForward,
                 *SkipBackward,
                 *Minimize,
                 *Exit;
}MPlayer;

extern MPlayer *MusicPlayer;

extern int Media_Player(MPlayer *mp, int play);
extern void Background_Player(void);
extern void Initialize_Player(void);
extern void Load_Audio(const char *filename, MPlayer *mp, int format);
extern void CreatePlayList(const char * file, MPlayer *mp);
extern void FreeMediaPlayer(MPlayer * mp);
extern void Unload_MediaPlayer_Gfx(MPlayer * mp);

extern void Audio_Program(const char * filename, int format);

#ifdef __cplusplus
}
#endif

#endif
