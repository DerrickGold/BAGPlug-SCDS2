#ifndef _SUPPORTED_FILES_
#define _SUPPORTED_FILES_


#ifdef __cplusplus
extern "C" {
#endif

#include "fb_api.h"
extern int NDS_ROM_LANG;
#define AUDIOTYPES 4
extern const char sfiles_audio[AUDIOTYPES][4];
//extern SndFile AudioFile;

#define IMAGETYPES 3
extern const char sfiles_image[IMAGETYPES][4];


#define EXECUTABLES 5
extern const char sfiles_exec[EXECUTABLES][5];

#define MAX_PROGRAMS AUDIOTYPES + IMAGETYPES + EXECUTABLES

typedef enum
{
	NDS_EXT,
	PLG_EXT,
	NZIP_EXT,
}executable_extensions;


typedef enum
{
	LAUNCH_DSGAME,
	
	PROGRAMCOUNT,
}PROGRAMLAUNCHERS;

//internal programs
typedef enum
{
	FILE_AUDIO = 1,
	FILE_NDSPLG = 2,
	FILE_NZIP = 3,
}FILETYPES;


typedef enum
{
	ICO_PD,
	ICO_DIR,
	ICO_PLG,
	ICO_NDS,
}iconframes;

struct EXTPRGM
{
    char *Name;
    char *Program;
    char **args;
    u8 num_args, fill_eos;
};

typedef struct _extentioninfo
{
    struct EXTPRGM * Entry;
	int num_programs;
}EXTINFO;

typedef enum
{
	EXT_INTERNAL,
	EXT_PRGMPATH,
	EXT_FILEPATH,
	EXT_EOSSETTINGS,
}EXTSCRIPTS;
extern const char EXTSCRIPT[5][16];


extern int stypes_Count;
extern const char splg_launch[PROGRAMCOUNT][MAX_PATH];


#define ALLTYPES MAX_EXT
extern char sfiles_list[ALLTYPES][6];
extern char *sfiles_all[ALLTYPES];

//extlink support?
typedef u16 UnicodeChar;

#define ExtLinkBody_MaxLength (256)

#define ExtLinkBody_ID (0x30545845) // EXT0

typedef struct {
  unsigned int ID,dummy1,dummy2,dummy3; // dummy is ZERO.
  char DataFullPathFilenameAlias[ExtLinkBody_MaxLength];
  char DataPathAlias[ExtLinkBody_MaxLength];
  char DataFilenameAlias[ExtLinkBody_MaxLength];
  char NDSFullPathFilenameAlias[ExtLinkBody_MaxLength];
  char NDSPathAlias[ExtLinkBody_MaxLength];
  char NDSFilenameAlias[ExtLinkBody_MaxLength];
  UnicodeChar DataFullPathFilenameUnicode[ExtLinkBody_MaxLength];
  UnicodeChar DataPathUnicode[ExtLinkBody_MaxLength];
  UnicodeChar DataFilenameUnicode[ExtLinkBody_MaxLength];
  UnicodeChar NDSFullPathFilenameUnicode[ExtLinkBody_MaxLength];
  UnicodeChar NDSPathUnicode[ExtLinkBody_MaxLength];
  UnicodeChar NDSFilenameUnicode[ExtLinkBody_MaxLength];
} TExtLinkBody;


extern int Launch_Audio(const char *file, const char * ext);
extern void loadAllTypes(void);
extern int Read_ExtFile(char *file, char *ext, EXTINFO * info);
extern int _read_prgmlist(FILE * extfile, EXTINFO * info);
extern void _read_arglist(FILE * extfile, EXTINFO * info);
extern int Init_EXTFile(EXTINFO *info);
extern void Free_EXTFile(EXTINFO * info);
extern int Internal_Programs( const char * filepath, const char * ext, EXTINFO * info, int program_id);
extern int Write_ExtLinkDat(const char *nds, const char *file);
extern void Use_EOS(const char *arg0);

#ifdef __cplusplus
}
#endif

#endif
