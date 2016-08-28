#ifndef _NDS_HEADER_
#define _NDS_HEADER_


#include <ds2_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NDS_ICON_WIDTH 32
#define NDS_ICON_HEIGHT 32

#define BANNER_DATA 0x68
#define TILE_DATA 32 //512 bytes
#define PAL_DATA 544 //32 bytes
//all 256 bytes
#define LANG_JAP 576
#define LANG_ENG 832
#define LANG_FRE 1088
#define LANG_GER 1344
#define LANG_ITN 1600
#define LANG_SPN 1856


typedef enum
{
	ROM_HB,//homebrew
	ROM_CM,//commercial
}ROMTYPES;

typedef struct _ndshead
{
	char text[256], // English Title
                         game_title[12],
                         game_code[6];
	int type;
}NDSHEADER;

extern void NDS_ReadTitle(const char *filename, NDSHEADER * nds);
extern void NDS_ReadBanner(const char *filename, int language, NDSHEADER *nds);
extern int NDS_ReadIcon(const char *filename, BMPOBJ * Object);
extern int NDS_ReadIconEx(const char *filename, BMPOBJ * Object, int transp);


/*=================================================
Nitro Zip
==================================================*/

#define BUFSIZE 131072

typedef enum
{
	NZIP_VER,
	NZIP_HASICON,
	NZIP_ICONTRANS,
}nzipfooter_reserved;

typedef struct _nzipfooter
{
	u16 Reserved[6];
	NDSHEADER nds;
	u16 Icon[1024];
}NZIPFOOTER;

extern int BAG_DSZIP_Deflate(const char *inputFilename, const char *outputFilename);
extern int BAG_DSZIP_Inflate(const char *inputFilename, const char *outputFilename);
extern void zerr(int ret, FILE * inerr, FILE * outerr);


extern int NZIP_ReadBanner( const char *filename, NDSHEADER * nds);
extern int NZIP_ReadTitle(const char *filename, NDSHEADER * nds);
extern int NZIP_ReadIconEx(const char *filename, BMPOBJ * Object, int transp);
extern int NZIP_ReadIcon(const char *filename, BMPOBJ * Object);
extern int BAG_DSZIP_GetDataSize(const char * filename);
#ifdef __cplusplus
}
#endif

#endif

