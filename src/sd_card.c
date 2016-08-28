#include <libBAG.h>
#include "sd_card.h"

#define KB 1024
#define MB 1048576
#define GB 1073741824

inline u64 getSdSize(PARTITION *info)//get sd size in bytes
{
	s16 reserved_space = info->dataStart - info->fat.fatStart + CLUSTER_FIRST;
	u64 size = (info->numberOfSectors - reserved_space) * info->bytesPerSector;
	return size;
}

inline u64 getSdUsed(PARTITION *info)
{
	char filename[12];
	strcpy(filename, "_sd_info.dat");
	FILE * temp = fopen(filename, "wb");//create a file to fill file struct
	u64 sectors = temp->appendPosition.cluster * info->sectorsPerCluster;
	fclose(temp);
	remove(filename);
	u64 used = sectors * info->bytesPerSector;
	return used;
}

inline void SD_getSpaceInfo(u64 *used,  u64 *total, u64 *free)
{
	PARTITION *info = (PARTITION*)_FAT_partition_getPartitionFromPath ("fat:");
	(*total) = getSdSize(info);
	(*used) = getSdUsed(info);
	(*free) = (*total) - (*used);
}

inline off_t SD_getFileSize(const char *file)
{
	if(file == NULL)
		return 0;
		
	struct stat st;
	lstat(file, &st);
	return st.st_size;
}

inline unsigned int SD_convertSize(u64 size, u8 *unit)
{
	if (size > MB)
	{
		(*unit) = 2;
		return size / MB;
	}
	else if (size > KB)
	{
		(*unit) = 1;
		return size / KB;
	}

	(*unit) = 0;
	return size;
}
