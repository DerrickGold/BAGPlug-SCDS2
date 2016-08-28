#include <libBAG.h>
#include "ndsheader.h"
#include <zlib.h>
#include "../miscInterface.h"
#include "window.h"
#include "language.h"
#include "interface.h"
#include "menu_files.h"
#include "settings.h"
#include "read_txt.h"
#include "music_player.h"
#include "miscinterface.h"
#include "draw.h"

#define BAR_HT 16
typedef enum
{
	WINDOW_TITLE,
	FILENAME_BUFFER,
	ERROR_TITLE,
	ERROR_MSG0,
	ERROR_MSG1,
	MSG_COUNT,
}nziptext;
	

int Nzip_ListAdd(const char *file)
{
	u8 isOnList = (TxtFile_cmp(file, &Global_NzipFileList) == -1) ? 0 : 1;
	if(isOnList == 0)
	{
		TxtFile_addLine(&Global_NzipFileList, file);
		TxtFile_writeAddLine(GlobalPaths.UnzipList, file);
		UI_Style.gfx.ShowNzipIcon = 1;
	}		
	return isOnList;
}
	
	
	
u8 nzipError(struct _language *msgs)
{	
	//read title
	int buffCount = ERROR_TITLE;
	Language_FileReadStr(GlobalPaths.Language, msgs->buffer[buffCount++], "Error Messages", "window_title", NULL);
	Language_FileReadStr(GlobalPaths.Language, msgs->buffer[buffCount++], "Error Messages", "nzip_msg0", NULL);
	Language_FileReadStr(GlobalPaths.Language, msgs->buffer[buffCount++], "Error Messages", "nzip_msg1", NULL);
	Unicode_Notification_Window(msgs->buffer[ERROR_TITLE], (u16**)&msgs->buffer[ERROR_MSG0], buffCount - ERROR_MSG0 - 1, 1);
	return 0;
}

u8 nzipCreateWindow(windowObj *window, struct _language *msg, char *filename)
{
	guigfx * GFX = &UI_Style.gfx;
	char skin[MAX_PATH];
	sprintf(skin, "%s/skin.ini", UI_Style.Current_Skin);
	Set_Window_Font (window, &GFX->Font);
	Load_UniWindow_Ini(window, "Window Settings", msg->buffer[0], skin);

	utf8_to_unicode16(filename, msg->buffer[1]);//convert file name to unicode for display
	//calculate window dimensions
	int width  = BAG_UniGetStrWd(msg->buffer[0], window->font);
	int width2 = BAG_UniGetStrWd(msg->buffer[1], window->font);
	//get the longer text
	if(width2 > width)
		width = width2;
	
	int line_count = 3;// file name + progress bar
    int txtheight = BAG_GetCharHt('g', window->font);
	int boxwd = width + 32,
		 boxht = (( line_count) * txtheight) + BAR_HT + txtheight;
		 
	Set_Window_Dim2(window, BAR_HT, (SCREEN_WIDTH>>1) - (boxwd >> 1), (SCREEN_HEIGHT>>1) - (boxht >> 1), boxwd, boxht);
	Create_Window_Scrn(window, DOWN_SCREEN);
	
	return line_count;
}
	
int NZip_DecompressMenu(char *inputFile)
{
	int rtrn = 0, refresh_lists = 0;
	//load languages
	struct _language nzipLang;
	if(Language_InitBuffers(&nzipLang, MSG_COUNT) != 1)
		return 0;	
		
	//default error message
	char filename[MAX_PATH];
	char outputName[MAX_PATH];
	FB_GetFileFromPath(filename, inputFile);//get the file name on its own
	strcpy(outputName, inputFile);
	FB_StripExt(outputName);
	strcat(outputName, ".");
	strcat(outputName, sfiles_exec[0]);//append .nds extension
	//add file to unzip list, and check if it is already unzipped
	if(Nzip_ListAdd(outputName))
	{
		Language_CleanBuffers(&nzipLang);
		return 1;
	}
	else 
		refresh_lists = 1;
	
	 //grab file size which is stored in the last 4 bytes of a gz file
	int FileSize=BAG_DSZIP_GetDataSize(inputFile);
    if(FileSize == 0)//corrupt zip file
        return 0;
        
	//open the input file
	gzFile inFile = gzopen(inputFile, "rb");
	if(!inFile)
	{
		nzipError(&nzipLang);
		goto CLEANUP;
	}
	
	//open output file
	static FILE *zipOutFile = NULL;
	zipOutFile = fopen(outputName, "wb");
	if(!zipOutFile)
	{
		nzipError(&nzipLang);
		goto CLEANUP;
	}
    
	Language_FileReadStr(GlobalPaths.Language, nzipLang.buffer[0], "MISC", "nzip_window_title", NULL);

	//load window
	windowObj nzipWindow;
	windowObj *window = &nzipWindow;
	nzipCreateWindow(window, &nzipLang, filename);
	int txtheight = BAG_GetCharHt('g', window->font);
	//prepare progress bar
    struct Progress_Bar bar;
    ProgressBar_Reset(&bar);

    bar.height = txtheight + 2;
    bar.fnt = window->font;
    bar.text_height = txtheight;
    bar.boarder_col = window->BoarderCol;
    bar.fill_col = window->HilightTxtCol;
    bar.font_col = window->FontCol;
	bar.total = FileSize;
    bar.x = window->x + 5;
    //bar.y = window->y + BAR_HT + (txtheight>>1) + ((line_count - 1) * (txtheight)); 	
    bar.width = window->width - 10;	
	
	void Update_Display(u16 *screen, u64 read)
	{
		Draw_Window(window);

		int TextStartX = (window->x + (window->width >> 1)) - (BAG_UniGetStrWd(nzipLang.buffer[1], window->font) >> 1);
		int TextY = window->y + window->barHt + (txtheight >>1);
		BAG_UniBmpPrintScrn(screen, TextStartX , TextY, window->x + window->width - 8, window->y + window->height, nzipLang.buffer[1], window->font, window->FontCol, -2);
		bar.y = (window->y + window->height) - ((txtheight) + bar.height);
		bar.dest = screen;     
		bar.count = read;
        ProgressBar(&bar);
		BAG_FlipScreen(DOWN_SCREEN);
	}
	
	Update_Display(down_screen_addr, 0);
	int readSize = 0, writeSize = 0, result=0;
	u64 total_read = 0;
	
	//allocate buffer
	static char *zipbuf = NULL;
	zipbuf = calloc( Main_Settings.copy_buf_len, sizeof(char));
	if(zipbuf == NULL)
	{
		nzipError(&nzipLang);
		goto CLEANUP;
	}
	//decompression loop
	int update = Main_Settings.copy_display_update;
	do
	{
		if(update >= Main_Settings.copy_display_update)
		{
			BAG_UpdateAudioBuffers();
			Update_Display(down_screen_addr, total_read);
			update = 0;
		}
		
		readSize = gzread(inFile, zipbuf, Main_Settings.copy_buf_len);
		total_read += readSize;
		BAG_UpdateAudioBuffers();
		//make sure to not read the nzip footer data
		if(total_read > FileSize)
			readSize -= (total_read - FileSize);
		
		if(readSize < 0)
		{
			gzerror(inFile, &result);
			nzipError(&nzipLang);
			goto CLEANUP;
			break;
		}
		else if(readSize == 0)
		{
			result = EOF;
			break;
		}	
		if((writeSize = fwrite(zipbuf, 1, readSize, zipOutFile)) != readSize)
		{
			result = Z_ERRNO;
			nzipError(&nzipLang);
			goto CLEANUP;
			break;
		}
		BAG_UpdateAudioBuffers();
		update++;
	}while(readSize > 0 && total_read < FileSize );
	rtrn = 1;
	
	CLEANUP:
	free(zipbuf);
		
	result = gzclose(inFile);
	fclose(zipOutFile);

	if(rtrn != 1)
		remove(outputName);
		
	Language_CleanBuffers(&nzipLang);
	Delete_Window(window);
	ProgressBar_Reset(&bar);
	BAG_UpdateIN();
	
	if(refresh_lists)
		RefreshMainLists();

	return rtrn;
}


u8 Nzip_Manager_DeleteMsg(void)
{
	typedef enum{
		__WINDOW_TITLE,
		__MESSAGE0,
		__MESSAGE1,
	}overwritemsgs;
	
	struct _language msgs;
	memset(&msgs, 0, sizeof(struct _language));
	if(Language_InitBuffers(&msgs, 3) != 1)
		return 0;
		
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__WINDOW_TITLE], "Clip Board", "file_action_title2", NULL);
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__MESSAGE0], "Clip Board", "delete_msg0", NULL);
	Language_FileReadStr(GlobalPaths.Language, msgs.buffer[__MESSAGE1], "Clip Board", "delete_msg1", NULL);

    int force = Unicode_Notification_Window(msgs.buffer[__WINDOW_TITLE], (u16**)&msgs.buffer[__MESSAGE0], 2, 2);
	Language_CleanBuffers(&msgs);
    if(!force)
        return 0;

	return 1;
}


//view and remove unzipped nds roms
void Nzip_Manager(void)
{
	guigfx * GFX = &UI_Style.gfx;

	//load languages
	struct _language nzipLang;
	struct _language *msg = &nzipLang;
	if(Language_InitBuffers(&nzipLang, 1) != 1)
		return;	
		
	Language_FileReadStr(GlobalPaths.Language, msg->buffer[0], "MISC", "nzip_manager_title", NULL);
		
	//create window
	Render_DirGfx(down_screen_addr);
	windowObj nzipWindow;
	windowObj *window = &nzipWindow;
	
	char skin[MAX_PATH];
	sprintf(skin, "%s/skin.ini", UI_Style.Current_Skin);
	Set_Window_Font (window, &GFX->Font);
	Load_UniWindow_Ini(window, "Window Settings", msg->buffer[0], skin);
	Set_Window_Dim2(window, BAR_HT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	Create_Window_Scrn(window, DOWN_SCREEN);
	

	if(TxtFile_getLineCount(&Global_NzipFileList) == 0)
		 goto CLEANUP;
		
	int iconHt = MAX(GFX->SelectAll->Height, MAX(GFX->Delete->Height, GFX->Exit->Height));
	int dockY = window->y + window->height - iconHt;
	
		
	int txtheight = BAG_GetCharHt('g', window->font);
    int  ynext =  (GFX->BoxOn->Height > txtheight) ? GFX->BoxOn->Height : (txtheight + 1);
    int max_disp =(int) ((float) ((SCREEN_HEIGHT - window->barHt - iconHt) / ynext) + 0.5);
	if(max_disp > TxtFile_getLineCount(&Global_NzipFileList))
		max_disp = TxtFile_getLineCount(&Global_NzipFileList);
	max_disp--;

	//allocate checkboxs for everything in list
	u8 *zip_Selected = NULL;
	zip_Selected = calloc(TxtFile_getLineCount(&Global_NzipFileList), sizeof(u8));
	if(!zip_Selected)	
		goto CLEANUP;
	
	struct _scrollbar ScrollBar;
	ScrollBar_Create(&ScrollBar, GFX->ScrollBar, SCREEN_WIDTH - GFX->ScrollBar->Width, window->barHt, SCREEN_WIDTH, SCREEN_HEIGHT - iconHt, TxtFile_getLineCount(&Global_NzipFileList) - 1, max_disp, 1);
	//draw function
	void draw_screen(u16 *screen, int selected, int scroll)
	{
		u16 Unicode[MAX_PATH];
		char tempbuf[MAX_PATH];
		Draw_Window(window);
		
		int x = window->x + 4,
			y = window->y + window->barHt + 1,
			i = 0;

		for( i = 0; i < max_disp + 1; i++)
		{
			if(i + scroll >=  TxtFile_getLineCount(&Global_NzipFileList))
				break;
            int filenum = i + scroll;
			//format the file name
			memset(tempbuf, 0, sizeof(tempbuf));
			FB_GetFileFromPath(tempbuf, Global_NzipFileList.buffer[filenum]);
			
			memset(Unicode, 0, sizeof(Unicode));
			utf8_to_unicode16(tempbuf,Unicode);
			
			u16 Color = window->FontCol;
			if(filenum == selected)
				Color = window->HilightTxtCol;  			
			 BAG_UniBmpPrintScrn(screen, x, y + (ynext * i ), window->x + window->width - GFX->ScrollBar->Width - GFX->BoxOn->Width, SCREEN_HEIGHT, Unicode, window->font, Color, -2);	

			BMPOBJ *zip_CheckBox = GFX->BoxOff;
			if(zip_Selected[filenum])
				zip_CheckBox = GFX->BoxOn;
				
			BAG_DrawObj(zip_CheckBox, screen, window->width - GFX->ScrollBar->Width - GFX->BoxOn->Width, window->barHt + (ynext * i));	
				
		}
		int iconX = ((window->x + window->width) >> 1) - ((GFX->Exit->Width + GFX->SelectAll->Width + GFX->Delete->Width) >>1);
		BAG_DrawObj(GFX->Exit, screen, iconX, dockY); iconX += GFX->Exit->Width;
		BAG_DrawObj(GFX->Delete, screen, iconX, dockY); iconX += GFX->Delete->Width;
		BAG_DrawObj(GFX->SelectAll, screen, iconX, dockY);
		ScrollBar_Draw(screen, &ScrollBar, scroll);
		Flip_Screen_Fun(DOWN_SCREEN, 0);
	}
	
	void Delete_Item(char *file)
	{
		remove(file);
		int lineNum = TxtFile_cmp(file, &Global_NzipFileList);
		if(lineNum < -1)
			return;
			
		int i = lineNum;
		for(i = lineNum; i < TxtFile_getLineCount(&Global_NzipFileList); i++)
			zip_Selected[i] = zip_Selected[i + 1];

		if(TxtFile_getLineCount(&Global_NzipFileList) > 0)//reallocate memory
			zip_Selected = realloc(zip_Selected, TxtFile_getLineCount(&Global_NzipFileList) * sizeof(u8));
		
		TxtFile_removeLine(&Global_NzipFileList, lineNum);
		
		if(max_disp >  TxtFile_getLineCount(&Global_NzipFileList))
		{
			max_disp--;
			if(max_disp < 0)
				max_disp = 0;
		}
	}
	
	int file_selected = 0, file_oldSelected = -1, list_scroll = 0, padTimer = 0, all_selected = 0, select_count = 0, cursor = 0;
	BAG_UpdateIN();
	
	while(1)
	{
		//controls
	    if((Pad.Held.Up || Pad.Held.Down) && padTimer < 20)
            padTimer++;
            
        //pad controls
        if(Pad.Newpress.Down || (Pad.Held.Down && padTimer >= 20))
        {
            if(cursor < max_disp)
                cursor++;
            else
                if(list_scroll + cursor < TxtFile_getLineCount(&Global_NzipFileList) - 1)
                    list_scroll++;
        }
        
        if(Pad.Newpress.Up || (Pad.Held.Up && padTimer >= 20))
        {
            if(cursor > 0)
                cursor--;
            else
                if(list_scroll + cursor > 0)
                    list_scroll--;
					
        }
		
        if(Pad.Released.Up || Pad.Released.Down)
            padTimer = 0;
	
	
		file_selected = cursor + list_scroll;
		
        if(Stylus.Newpress || Stylus.Held) 
        {
			int bary = ScrollBar_Stylus(&ScrollBar);
			if(bary != -1)
				list_scroll = bary;
            else if(BAG_StylusZone(window->x, window->barHt, window->x + window->width,  dockY))
            {
                cursor =  (Stylus.Y - window->barHt) / (ynext + 1);
				if(cursor > max_disp)
					cursor = max_disp;
                file_selected = (cursor + list_scroll < TxtFile_getLineCount(&Global_NzipFileList) - 1) ? cursor + list_scroll : TxtFile_getLineCount(&Global_NzipFileList) - 1;
            }
		}
		
		if(Pad.Newpress.B || (Stylus.Newpress && BAG_ObjTouched(GFX->Exit)))
			break;
		

		//select all
		if(Pad.Newpress.Y || (Stylus.Newpress && BAG_ObjTouched(GFX->SelectAll)))
		{
			if(!all_selected)
			{
				memset(zip_Selected, 1, sizeof(zip_Selected));
				all_selected++;
				select_count = TxtFile_getLineCount(&Global_NzipFileList);
			}
			else
			{
				memset(zip_Selected, 0, sizeof(zip_Selected));
				all_selected = 0;
				select_count = 0;
			}
			file_oldSelected = -1;
		}
		//delete
		if(Pad.Newpress.Select || (Stylus.Newpress && BAG_ObjTouched(GFX->Delete)))
		{
			if(select_count > 0 && Nzip_Manager_DeleteMsg())
			{
	
				int z = 0;
				do
				{
					if(zip_Selected[z] == 1)	
						Delete_Item(Global_NzipFileList.buffer[z]);
					else
						z++;
				}while(z < TxtFile_getLineCount(&Global_NzipFileList));
				
				TxtFile_Write(GlobalPaths.UnzipList, &Global_NzipFileList, "wb");
				RefreshMainLists();
				select_count = 0;
				cursor = 0;
				list_scroll = 0;
		
				if(TxtFile_getLineCount(&Global_NzipFileList) <= 0)
					break;
			}
			file_oldSelected = -1;
		}
				
		if(Pad.Newpress.A || (Stylus.DblClick && file_selected == file_oldSelected))
		{
			if(!zip_Selected[file_selected] )
			{
				zip_Selected[file_selected] = 1;
				select_count++;
			}
			else
			{
				zip_Selected[file_selected] = 0;
				select_count--;
			}
				
			file_oldSelected = -1;
		}
			
		//screen update
		if(file_selected != file_oldSelected || MusicPlayer->minimize)
		{
			draw_screen(down_screen_addr, file_selected, list_scroll);
			file_oldSelected = file_selected;
		}
			
		BAG_Update();
	}
	
	CLEANUP:
		ScrollBar_Delete(&ScrollBar);
		Delete_Window(window);
		if(TxtFile_getLineCount(&Global_NzipFileList) == 0)
		{
			remove(GlobalPaths.UnzipList);
			UI_Style.gfx.ShowNzipIcon = 0;
		}
		if(zip_Selected)
			free(zip_Selected);
}
