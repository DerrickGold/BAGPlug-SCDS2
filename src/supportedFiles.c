#include <libBAG.h>
#include "supportedFiles.h"
#include "fb_api.h"
#include "window.h"
#include "ndsheader.h"
#include "menu_files.h"
#include "read_txt.h"
#include "music_player.h"
#include "minIni.h"
#include "interface.h"
#include "settings.h"

//supported internal files to match ext files
int NDS_ROM_LANG = 0;
const char sfiles_audio[AUDIOTYPES][4] = {"raw", "wav", "ogg", "mp3"};
const char sfiles_image[IMAGETYPES][4] = {"bmp", "jpg", "png"};
const char sfiles_exec[EXECUTABLES][5] = {"nds", "plg", "nzip"};


char * sfiles_all[ALLTYPES];
char sfiles_list[ALLTYPES][6];
int stypes_Count = 0;

const char splg_launch[PROGRAMCOUNT][MAX_PATH] = 
{
	"/_DSTwo/DSGAME.nds",
};
						

void loadAllTypes(void)
{
	//collect supported files from ext directory
	FileBrowser *EXT_List = FB_Init (ALLTYPES, 0);
	FB_HideAllFolders(EXT_List, 1);
	//populate list
	char argext[1][3] = {"arg"};
	char * extensions[1];
	extensions[0] = (char*)&argext[0];
	
	FB_ScanDir(GlobalPaths.EXT_Folder, EXT_List, 1, extensions, 0);
	//copy list of files
	int i = 0;
    char extbuf[EXT_NAME_LEN];
    memset(extbuf, 0, sizeof(extbuf));
	
	for (i = 0; i < EXT_List->File_Count; i++)
	{
		strcpy(sfiles_list[stypes_Count], EXT_List->File[i].Name);
		//remove .arg extension
		FB_StripExt(sfiles_list[stypes_Count]);
		sfiles_all[stypes_Count] = (char*)&sfiles_list[stypes_Count];
		stypes_Count++;
	}
	FB_Free (EXT_List);
}

const char EXTSCRIPT[5][16] =
{
	"$INTERNAL$",
	"$PRGMPATH$",
	"$FILEPATH$",
	"-EOSSETTINGS",
};

int _strcspn_ (const char * str1, const char * str2, int start)
{
    int len1 = strlen(str1) - start;
    int len2 = strlen(str2);
    
    int count2 = 0;
    while(count2 < len2)
    {
        int count1 = 0;
        for(count1 = 0; count1 < len1; count1++)
        {
            if(str1[count1] == str2[count2])
                return count1 + 1;
        }
        count2++;
    }
    
    return 0;
}

int Read_ProgramStruct(FILE * extfile, EXTINFO * info)
{
    char linestr[MAX_PATH];
    memset(linestr, 0, sizeof(linestr));
    
    int pathFound = 0, argsFound = 0, eosSettingsFound = 0;
    
    while(!feof(extfile))
    {
		BAG_UpdateAudio();
        char * test = fgets ( linestr, MAX_PATH, extfile );
        int len = strlen(linestr);
        //check if end of structure has been reached
        if(test == NULL || linestr[0] == ';')
            break;
            
        //check for file path
        if(!pathFound && linestr[0] == '"')
        {
            int i = 1;
            while(i < len && linestr[i] != '"')
                i++;
            
            //check if there is a complete file path format
            if(linestr[i] == '"')
            {
                info->Entry[info->num_programs].Program = calloc( i + 1, sizeof(u8));
                if(info->Entry[info->num_programs].Program)
                {
                    memcpy(info->Entry[info->num_programs].Program, &linestr[1], i - 1);
                    pathFound++;
                }
            }
        }
		
		if(!eosSettingsFound && !strncasecmp(linestr, EXTSCRIPT[EXT_EOSSETTINGS], strlen(EXTSCRIPT[EXT_EOSSETTINGS])))
		{
			info->Entry[info->num_programs].fill_eos = 1;
			eosSettingsFound = 1;
		}
        
        //check for arguments now
        if(!argsFound && linestr[0] == '[')
        {
            info->Entry[info->num_programs].num_args = 1;
            //count number of args
            int i = 0;
            while(i < len && linestr[i] != ']' && linestr[i] != '/')
            {
                if(linestr[i] == ',' && linestr[i + 1] != ' ' && linestr[i + 1] != ']')
                   info->Entry[info->num_programs].num_args++;
                i++;
            }
         
            info->Entry[info->num_programs].args = calloc(  info->Entry[info->num_programs].num_args, sizeof(u8 *));
            if(info->Entry[info->num_programs].args)
            {
                char strkeys[2];
				strcpy(strkeys, ",]");
                //sprintf(strkeys, ",]");
                
                int count = 0, oldcount = 1;
                for(i = 0; i <  info->Entry[info->num_programs].num_args; i++)
                {
                    count = _strcspn_(&linestr[oldcount], strkeys, oldcount);
                    if(count > 1)
                    {
                        info->Entry[info->num_programs].args[argsFound] = calloc(count, sizeof(u8));
                        if(info->Entry[info->num_programs].args[argsFound])
                        {
                            memcpy(info->Entry[info->num_programs].args[argsFound], &linestr[oldcount], count - 1);
                            strcat(info->Entry[info->num_programs].args[argsFound], "\0");
                            argsFound++;
                        }
                        else
                            break;
                            
                        oldcount += count;
                    }
                    else
                        break;
                }
            }
            else 
                break;
        }
    }
    if(pathFound && argsFound)
        info->num_programs++;
		
	return 1;
}


int Find_ext_Program(FILE * extfile, EXTINFO * info)
{
    char tempstr[MAX_PATH];
    memset(tempstr, 0, sizeof(tempstr));
    
     info->num_programs = 0;
    
    //scan file for a program entry
    fseek(extfile, 0, SEEK_SET);
	while(!feof(extfile))
	{
		BAG_UpdateAudio();
		fgets ( tempstr, MAX_PATH, extfile );
        int  len = strlen(tempstr);
        int i = 0;
        while( i < len)
        {
			BAG_UpdateAudio();
            if(tempstr[i] == '/' || tempstr[i] == ';')//comment found
                break;
                
            if(tempstr[i] == ':')//program entry found
            {
                info->Entry[info->num_programs].Name = calloc( i + 1, sizeof(u8));
                if(info->Entry[info->num_programs].Name)
                {
                    strncpy(info->Entry[info->num_programs].Name, tempstr, i);
                    Read_ProgramStruct(extfile, info);
                }
                break;
            }
            i++;
			
        }
    }
    
    return info->num_programs;
}

int Read_ExtFile(char *file, char *ext, EXTINFO * info)
{
    FILE * extfile = NULL;
    char temp[MAX_PATH];
    memset(temp, 0, sizeof(temp));

    //check if there is an arg file for specified file
    strcpy(temp, file);
    strcat(temp, ".arg");
    
    extfile = fat_fopen(temp, "rb");
    if(!extfile)
    {
        //if there is no args file, then use the standard one
        memset(temp, 0, sizeof(temp));
        sprintf(temp, "%s%s.arg",GlobalPaths.EXT_Folder, ext);
        extfile = fat_fopen(temp, "rb");
        if(!extfile)
            return 0;
    }
    int num = Find_ext_Program(extfile, info);

    fat_fclose(extfile);
    return num;
}
    
int Init_EXTFile(EXTINFO *info)
{
	info->Entry = calloc(sizeof(struct EXTPRGM), MAX_PROGRAMS);
	if(!info->Entry)
		return 0;	
	return 1;
}

void Free_EXTFile(EXTINFO * info)
{
	int i = 0;
	for( i = 0; i < info -> num_programs; i++)
	{
		if(info->Entry[i].Name)
		{
			free(info->Entry[i].Name);
			info->Entry[i].Name = NULL;
		}
        
        if(info->Entry[i].Program)
        {
			free(info->Entry[i].Program);
			info->Entry[i].Program = NULL;
        }
        
        int j = 0;
        for( j = 0; j < info->Entry[i].num_args; j++)
        {
			if(info->Entry[i].args[j])
			{
				free(info->Entry[i].args[j]);
				info->Entry[i].args[j] = NULL;
			}
        }
          
		if(info->Entry[i].args)
		{
			free(info->Entry[i].args);
			info->Entry[i].args = NULL;
		}
        
        info->Entry[i].num_args = 0;
    }
    info -> num_programs = 0;
    if(info->Entry)
    {
		free(info->Entry);
		info->Entry = NULL;
	}
}
	
int SetAutoBoot(int boot);

int Launch_Audio(const char *file, const char * ext)
{
	int i = 0;
	for(i = 0; i < AUDIOTYPES; i++)
	{
		if(!strcasecmp(ext, sfiles_audio[i]))
		{	
			Audio_Program(file, i);
			return 1;
		}
	}
	
	return 0;
}

extern void File_Args(const char *filedest, int argc,  ...);

void Use_EOS(const char *arg0)
{
	if(arg0 == NULL)
		return;

	SetAutoBoot(1);
	char filename[MAX_PATH];
	memset(filename, 0, sizeof(filename));
	strcpy(filename, GlobalPaths.LastSave_ini);
	
	FILE * inifile = fopen(filename, "wb");
	if(!inifile)
		return;
		
	fprintf(inifile, "[Dir Info]\n");
	fprintf(inifile, "fullName=fat1:%s\n", arg0);
	fprintf(inifile, "shortName=fat1:%s\n", arg0);
	fprintf(inifile, "RomType=%d", 0);
	fclose(inifile);
}

int Internal_Programs( const char * filepath, const char * ext, EXTINFO * info, int program_id)
{
	if(ext == NULL || info == NULL || filepath == NULL)
		return 0;

	int argCount = info->Entry[program_id].num_args;
	/*char args[2][MAX_PATH];
    memset(args, 0, sizeof(args));*/
	char **args = NULL;
	
	void cleanArgs(void)
	{
		if(args == NULL)
			return;
			
		int z = 0;
		for(z = 0; z < argCount + 1; z++)
		{
			if(args[z] != NULL)
			{
				free(args[z]);
				args[z] = NULL;
			}
		}
		free(args);
		args = NULL;
	}
	
	args = (char**)calloc(argCount + 1, sizeof(char*));
	if(args == NULL)
		return 0;
		
	//allocate arg memory
	int v = 0;
	for( v = 0; v < argCount + 1; v++)
	{
		args[v] = calloc(MAX_PATH, sizeof(char));
		if(args[v] == NULL)
		{
			cleanArgs();
			return 0;
		}
	}
	//interpret arg data
	int i = 0;
	for( i = 0; i < argCount; i++)
	{
		//check if arguement is program path
		if(!strcasecmp(EXTSCRIPT[EXT_PRGMPATH], info->Entry[program_id].args[i]))
			strcpy(args[0], filepath);
		//check if arguement is file path
		else if(!strcasecmp(EXTSCRIPT[EXT_FILEPATH], info->Entry[program_id].args[i]))
			strcpy(args[1], filepath);
		//if neither is set, then assume the custom arguement is a file path
		else
			strcpy(args[i], info->Entry[program_id].args[i]);
	}

	//internally supported audio files
	if(Launch_Audio(args[1], ext))
	{
		cleanArgs();
		return FILE_AUDIO;
	}
	//internally supported executable files
	if(!strcasecmp(ext, sfiles_exec[1]))//.plg
	{
		File_Args(GlobalPaths.LoadFile_dat, 1, args[0]);
		cleanArgs();
		return FILE_NDSPLG;
	}	
	
	if(!strcasecmp(ext, sfiles_exec[0]))//.nds
	{	
		//check if rom is homebrew
		NDSHEADER ndsrom;
		NDS_ReadBanner(args[0], NDS_ROM_LANG, &ndsrom);
	
		int isDSGAME = !strcasecmp(splg_launch[LAUNCH_DSGAME], filepath);
		//launch homebrew
		if((ndsrom.type == ROM_HB || isDSGAME) && !info->Entry[program_id].fill_eos)//DSGAME.nds is not set as a homebrew
		{
			if(isDSGAME)
				SetAutoBoot(0);
				
			File_Args(GlobalPaths.LoadFile_dat, 1, args[0]);
			cleanArgs();
			return FILE_NDSPLG;
		}
		
		//launch commercial roms using EOS auto run feature
		else
		{
            Use_EOS(args[0]);
            File_Args(GlobalPaths.LoadFile_dat, 1, splg_launch[LAUNCH_DSGAME]);
			cleanArgs();
            return FILE_NDSPLG;
		}
	}
	
	if(!strcasecmp(ext, sfiles_exec[2]))//.nzip
	{
		int tempCpu = cpuVal_old;
		Change_Cpu(SETTINGS_CPU_NZIP, &Main_Settings);
		int test = NZip_DecompressMenu(args[0]);
		Change_Cpu(tempCpu, &Main_Settings);
		if(test == 1)
		{
			cleanArgs();
			return FILE_NZIP;
		}
	}
	cleanArgs();
	return 0;
}	

//enable or disable auto booting of rom in sc menu
#define GLOBALOPTIONS 2
 const char _globalsettings[GLOBALOPTIONS][32] = {"lastRomRun", "enterLastDirWhenBoot"};
int SetAutoBoot(int boot)
{
	int rtrn = 0;
	struct _ini_file global_ini;
	memset(&global_ini, 0, sizeof(struct _ini_file));
	
	ini_File_Init(&global_ini, GLOBALOPTIONS);
	int c = 0;
	for( c = 0; c < global_ini.Key_Count; c++)
		global_ini.Keys[c] = (char*)&_globalsettings[c];
		
	if(ini_Read_File(GlobalPaths.GlobalSettings_ini, &global_ini) == 1)
	{
		global_ini.Settings[0] = boot; //set the auto boot
		global_ini.Settings[1] = 1;//set enter last dir
		ini_Write_File(&global_ini);
		rtrn = 1;
	}
	ini_File_Clean(&global_ini);
	return rtrn;
}


int Write_ExtLinkDat(const char *nds, const char * file)
{
	FILE *extdat = fopen(GlobalPaths.ExtLink_dat,  "wb");
	if(!extdat)	
		return 0;
		
	TExtLinkBody datfile;
	memset(&datfile, 0, sizeof(TExtLinkBody));
	
	char TempBuf[MAX_PATH];
	memset(TempBuf, 0, sizeof(TempBuf));
	
	//file information
	sprintf(datfile.DataFullPathFilenameAlias, "%s", file);
	FB_GetFilePath(TempBuf,  file);
	sprintf(datfile.DataPathAlias, "%s/", TempBuf);
	FB_GetFileFromPath(TempBuf, file);
	sprintf(datfile.DataFilenameAlias, "%s", TempBuf);
	//nds information
	sprintf(datfile.NDSFullPathFilenameAlias, "fat:%s", nds);
	FB_GetFilePath(TempBuf,  nds);
	sprintf(datfile.NDSPathAlias, "fat:%s", TempBuf);
	FB_GetFileFromPath(TempBuf, nds);
	sprintf(datfile.NDSFilenameAlias, "fat:%s", TempBuf);
	
	//unicode data
	//file
	utf8_to_unicode16(datfile.DataFullPathFilenameAlias,  datfile.DataFullPathFilenameUnicode);
	utf8_to_unicode16(datfile.DataPathAlias,  datfile.DataPathUnicode);
	utf8_to_unicode16(datfile.DataFilenameAlias,  datfile.DataFilenameUnicode);
	//nds
	utf8_to_unicode16(datfile.NDSFullPathFilenameAlias,  datfile.NDSFullPathFilenameUnicode);
	utf8_to_unicode16(datfile.NDSPathAlias,  datfile.NDSPathUnicode);
	utf8_to_unicode16(datfile.NDSFilenameAlias,  datfile.NDSFilenameUnicode);

	datfile.ID = ExtLinkBody_ID;
	fwrite(&datfile, 1, sizeof(TExtLinkBody), extdat);
	fclose(extdat);
	return 1;
}

