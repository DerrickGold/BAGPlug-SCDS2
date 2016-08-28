#include <libBAG.h>
#include <zlib.h>
#include "ndsheader.h"

//Determin if rom is Homebrew or Commercial
inline int NDS_IDCheck(const char *ID)
{
	if(!strcasecmp(ID, "####") || !strcasecmp(ID, "ENG0"))/*Supercard GAME.nds id*/
		return ROM_HB;//homebrew
	
	return ROM_CM;//commercial
}


void NDS_ReadBanner(const char *filename, int language, NDSHEADER *nds)
{
	FILE *handle = fopen(filename, "rb");
	if(!handle)
		goto end;
		
	//read title and game code
	fread(nds->game_title, 1, 12, handle);
	fread(nds->game_code, 1, 4, handle);		
	
	//determine if rom is commercial or brew
	nds->game_code[4]='\0';
	nds->type = NDS_IDCheck(nds->game_code);

		
	int banner_offset = 0;
	fseek(handle, BANNER_DATA, SEEK_SET);
	fread(&banner_offset, 1, 4, handle);
	
	if(!banner_offset)
		goto end;
		
	//determine language to read
	int seekpos = 0;
	switch(language)
	{
		case 0://jap
			seekpos = LANG_JAP;
		break;
		case 1://eng
			seekpos = LANG_ENG;
		break;
		case 2://french
			seekpos = LANG_FRE;
		break;
		case 3://german
			seekpos = LANG_GER;
		break;
		case 4://italian
			seekpos = LANG_ITN;
		break;
		case 5://spain
			seekpos = LANG_SPN;
		break;
	}
	//read language text		
	fseek(handle, banner_offset + seekpos, SEEK_SET);
	fread(nds->text, 1, 256, handle);
	
	//convert text
	int temp = 0;
	for(temp=0; temp<128; temp++)
		nds->text[temp] = nds->text[temp << 1];	
	
	end:
		if(handle)
			fclose(handle);
			
		return;
}

//Read just the game id and short title
inline void NDS_ReadTitle(const char *filename, NDSHEADER * nds)
{
	FILE *handle = fopen(filename, "rb");
	if(!handle)
		return;		
	//read title and game code
	fread(nds->game_title, 1, 12, handle);
	fread(nds->game_code, 1, 4, handle);		
	fclose(handle);
	
	//determine if rom is commercial or brew
	nds->game_code[4]='\0';
	nds->type = NDS_IDCheck(nds->game_code);
}

inline void _constructIcon(u16 *dest, const u8 *data, const u16 *pal)
{
	int yy = 0, index = 0, y = 0, x = 0, xz = 0, yz = 0;
	do{
		x = 0;
		y = 0;
		do{
			xz = ((y>>3) << 3) + x;
			yz =((yy << 3) + (y&7)) << 5;
			dest[(xz + 1) + yz] = (u16)pal[(data[ index ] >> 4) & 15 ];
			dest[ (xz) + yz] = (u16)pal[ data[ index ] & 15 ];
			
			index++;
			y+=((x+=2)>>3);
			x = x&7;
		}while(y < 32);	
	}while(yy++ <  3);
}

inline int _NDS_ReadIconEx(const char *filename, u16 * dest, int transp)
{
	int data[136];
	FILE *handle = fopen(filename, "rb");
	if(!handle)
		return -1;

	//get icon data offset
	int icon_offset = 0;
	fseek(handle, BANNER_DATA, SEEK_SET);
	fread(&icon_offset, 1, 4, handle);

	//check if icon available
	if(!icon_offset)
	{
		fclose(handle);		
		return -1;
	}
	//read palette and data in one go
	fseek(handle, (icon_offset + TILE_DATA), SEEK_SET);
	fread(&data, 4, 136, handle);	
	fclose(handle);
	
	//construct icon
	u16 *pal = (u16*)&data[128];
	if(transp==0)//no transparent color, turn it to white 
		pal[0] = 65535;

	_constructIcon(dest, (u8*)data, pal);
	return pal[0];
}


/*=======================================
Read NDS icon to Bitmap object in libBAG for DSTwo
=======================================*/
inline int NDS_ReadIconEx(const char *filename, BMPOBJ * Object, int transp)
{
	int pal = _NDS_ReadIconEx(filename, Object->Buffer, transp);
	if(pal != -1)
	{
		Object->TransCol  = pal;
		return 1;
	}
		
	return 0;
}

inline int NDS_ReadIcon(const char *filename, BMPOBJ * Object)
{
	return NDS_ReadIconEx(filename, Object, 1);
}

/*==========================================
Nitro Zip header reading
==========================================*/
//read whole ndsheader struct
inline int NZIP_ReadBanner( const char *filename, NDSHEADER * nds)
{
	FILE *DSZIP = fopen(filename, "rb");
	if(!DSZIP)
		return -1;
		
	//seek to footer
	int offset = sizeof(NZIPFOOTER) - 12;//skip the reserved bytes
	fseek(DSZIP, -offset, SEEK_END);		
	fread(nds, 1, sizeof(NDSHEADER), DSZIP);
	nds->game_code[4] = '\0';
	fclose(DSZIP);
	return 1;
}

//just read game id and short title
int NZIP_ReadTitle(const char *filename, NDSHEADER * nds)
{
	FILE *DSZIP = fopen(filename, "rb");
	if(!DSZIP)
		return -1;
		
	//seek to footer
	int offset = sizeof(NZIPFOOTER) - 12 - 256;//skip reserved and text bytes
	fseek(DSZIP, -offset , SEEK_END);	
	fread(nds->game_title, 1, 12, DSZIP);
	fread(nds->game_code, 1, 6, DSZIP);
	nds->game_code[4] = '\0';
	offset = 2048+ 4;//skip icon buf and rom type
	fseek(DSZIP, -offset , SEEK_END);
	fread(&nds->type, 1, 4, DSZIP);
	fclose(DSZIP);
	return 1;
}

/*=====================================
Read Icon from nzip file
=====================================*/
inline int _nzip_readIcon(const char *filename, u16 * dest, int transp)
{
	FILE *DSZIP = fopen(filename, "rb");
	if(!DSZIP)
		return -1;
		
	NZIPFOOTER data;
	memset(&data, 0, sizeof(NZIPFOOTER));
	
	fseek(DSZIP, 0, SEEK_END);
	fseek(DSZIP, - sizeof(NZIPFOOTER), SEEK_CUR);
	
	//check if there is an icon
	fread(data.Reserved, 1, 12, DSZIP);
	if(data.Reserved[NZIP_HASICON] == 0)
	{
		fclose(DSZIP);
		return -1;
	}
	//read the icon now
	fseek(DSZIP, sizeof(NDSHEADER), SEEK_CUR);
	fread(&data.Icon, 2, 1024, DSZIP);
	memcpy(dest, &data.Icon, sizeof(data.Icon));
	fclose(DSZIP);
	
	return data.Reserved[NZIP_ICONTRANS];
}


/*=======================================
Read nzip icon to Bitmap object in libBAG for DSTwo
=======================================*/
inline int NZIP_ReadIconEx(const char *filename, BMPOBJ * Object, int transp)
{
	int pal =  _nzip_readIcon(filename, Object->Buffer, transp);
	if(pal != -1)
	{
		Object->TransCol  = pal;
		return 1;
	}
	return 0;
}

inline int NZIP_ReadIcon(const char *filename, BMPOBJ * Object)
{
	 return NZIP_ReadIconEx(filename, Object, 1);
 }

/*================================================================================================================
File Compression Functions
//===============================================================================================================*/

/*appends bits of the nds header to the end of the zip file
Data appended:
Reserved data
[0] - zip identification version
[1] - icon check, if the nds file does have an icon
[2] - transparent color of the icon

-game title: 12 bytes
-game code: 4 bytes
-one language of the title as specified: 256 bytes
-game icon as raw 16 bit data
*/
int write_nds_header(const char *inputFilename, const char *outputFilename)
{
	FILE *NDS = fopen(inputFilename, "rb");
	if(!NDS)
		return 0;
		
	fclose(NDS);
		
	FILE *DSZIP = fopen(outputFilename, "a+");
	if(!DSZIP)
		return 0;
		
		
	NZIPFOOTER data;
	memset(&data, 0, sizeof(NZIPFOOTER));
	
	data.Reserved[NZIP_VER] = 1;
	data.Reserved[NZIP_HASICON] = 0;
	
	NDS_ReadBanner(inputFilename, 1, &data.nds);
	int pal = _NDS_ReadIconEx(inputFilename, data.Icon, 1);
	if(pal > -1)
	{
		data.Reserved[NZIP_HASICON] = 1;
		data.Reserved[NZIP_ICONTRANS] = pal;
	}
	
	//now write the data to end of zip file
	fwrite(&data, 1, sizeof(NZIPFOOTER), DSZIP);
	fclose(DSZIP);
	return 1;
}


int BAG_DSZIP_Deflate(const char *inputFilename, const char *outputFilename)
{
    int err = Z_OK;
    //open output file
	gzFile outFile = gzopen(outputFilename, "wb8");
    if(!outFile)
    {
        printf("failed to open output\n");
        gzerror(outFile, &err);
        if(err != Z_OK)
            return err;
    }    
    
    
	FILE *inFile = fopen(inputFilename, "rb");
    if(!inFile)
	{
		gzclose(outFile);
		printf("failed to open input\n");
        return -99;
	}
        
    //grab file size
    int FileSize=0;
    fseek (inFile , 0 , SEEK_END);
    FileSize = ftell (inFile);
	fseek(inFile, 0L, SEEK_SET);

    int readSize = 0;
	
	//allocate data buffer
	char *buffer = (char*)calloc( BUFSIZE, sizeof(char));
	if(buffer == NULL)
	{
		fclose(inFile);
		gzclose(outFile);
		return -98;
	}
	
	do
	{
		readSize = fread(buffer, 1, BUFSIZE, inFile);
		if(readSize == 0)
			break;
	
		int writeSize = gzwrite(outFile, buffer, readSize);
        if( writeSize != readSize)
        {
            gzerror(outFile, &err);
            printf("error! %d\n", err);
            break;
        }
    }while(readSize > 0);
	
	free(buffer);
    err = gzclose(outFile);
    fclose(inFile);
	
	write_nds_header(inputFilename, outputFilename);
	return err;
}


//get size of zipped data
int BAG_DSZIP_GetDataSize(const char * filename)
{
	int FileSize = 0;
	FILE *testFile = fopen(filename, "rb");
    if(testFile)
    {
        fseek (testFile , 0 , SEEK_END);
        fseek (testFile , -4 - sizeof(NZIPFOOTER), SEEK_CUR);
        fread (&FileSize,4, 1,testFile);
        fclose(testFile);
    }
	return FileSize;
}


int BAG_DSZIP_Inflate(const char *inputFilename, const char *outputFilename)
{
	int err = Z_OK;
	  
	 //grab file size which is stored in the last 4 bytes of a gz file
	int FileSize=BAG_DSZIP_GetDataSize(inputFilename);
    if(FileSize == 0)
        return Z_ERRNO;
        
	//open the input file
	gzFile inFile = gzopen(inputFilename, "rb");
	if(!inFile)
	{
		printf("failed to open input\n");
        gzerror(inFile, &err);
        if(err != Z_OK)
            return err;
	}
	
	//open output file
	FILE *outFile = fopen(outputFilename, "wb");
	if(!outFile)
	{
		printf("failed to open output\n");
		gzclose(inFile);
		return -99;
	}
    
    int readSize = 0, writeSize = 0, result=0;
	u64 total_read = 0;
	
	//allocate buffer
	char *buffer = (char*)calloc( BUFSIZE, sizeof(char));
	if(buffer == NULL)
	{
		fclose(inFile);
		gzclose(outFile);
		return -98;
	}
	
	//decompression loop
	do
	{
		readSize = gzread(inFile, buffer, BUFSIZE);
		total_read += readSize;
		
		//make sure to not read the nzip footer data
		if(total_read > FileSize)
			readSize -= (total_read - FileSize);
		
		if(readSize < 0)
		{
			gzerror(inFile, &result);
			break;
		}
		else if(readSize == 0)
		{
			result = EOF;
			break;
		}	
		if((writeSize = fwrite(buffer, 1, readSize, outFile)) != writeSize)
		{
			result = Z_ERRNO;
			break;
		}
	}while(readSize > 0 && total_read < FileSize );
	
	//cleanup
	free(buffer);
	result = gzclose(inFile);
	fclose(outFile);
  
    return result;
}
