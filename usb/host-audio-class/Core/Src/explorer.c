
/* Includes ------------------------------------------------------------------ */
#include "main.h"

#include "fatfs.h"
#include "sd_diskio_spi.h"
#include "sd_spi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ff.h"
#include "ffconf.h"
#include "defines.h"

char sd_path[4];
FATFS fs;
FILELIST_FileTypeDef FileList;

uint8_t SD_StorageInit(void)
{
	FRESULT res;

	printf("Linking SD driver...\r\n");
	if (FATFS_LinkDriver(&SD_Driver, sd_path) != 0) {
		printf("FATFS_LinkDriver failed\n");
		return FR_DISK_ERR;
	}

	printf("Initializing disk...\r\n");
	DSTATUS stat = disk_initialize(0);
	if (stat != 0) {
		printf("disk_initialize failed: 0x%02X\n", stat);
		printf("FR_NOT_READY\tTry Hard Reset or Check Connection/Power\r\n");
		printf("Make sure \"MX_FATFS_Init\" is not being called in the main function\n"\
				"You need to disable its call in CubeMX->Project Manager->Advance Settings->Uncheck Generate code for MX_FATFS_Init\r\n");
		return FR_NOT_READY;
	}

	printf("Attempting mount at %s...\r\n", sd_path);
	res = f_mount(&fs, sd_path, 1);
	if (res != FR_OK)
	{
		printf("Unable to Mount SD Card, ERROR %d\r\n", res);
		return res;
	}
	printf("SD card mounted successfully at %s\r\n", sd_path);
	return FR_OK;
}

/**
  * @brief  Copies disk content in the explorer list.
  * @param  None
  * @retval Operation result
  */
FRESULT SD_StorageParse(void)
{
    FRESULT res;
    FILINFO fno;
    DIR dir;
    char *fn;

#if _USE_LFN
    static char lfn[_MAX_LFN];
    fno.lfname = lfn;
    fno.lfsize = sizeof(lfn);
#endif

    printf("\r\n=====================================\r\n");
    printf("Scanning SD card for WAV files...\r\n");
    printf("=====================================\r\n");

    FileList.ptr = 0;

    res = f_opendir(&dir, sd_path);

    if (res != FR_OK)
    {
        printf("Failed to open directory! (Error: %d)\r\n", res);
        return res;
    }

    while (1)
    {
        res = f_readdir(&dir, &fno);

        if ((res != FR_OK) || (fno.fname[0] == 0))
        {
            break;
        }

        /* Ignore hidden files */
        if (fno.fattrib & (AM_DIR | AM_HID | AM_SYS))
        {
            continue;
        }

#if _USE_LFN
        fn = (*fno.lfname) ? fno.lfname : fno.fname;
#else
        fn = fno.fname;
#endif

        /* Skip directories */
        if (fno.fattrib & AM_DIR)
        {
            printf("[DIR ] %s\r\n", fn);
            continue;
        }

        printf("[FILE] %s\r\n", fn);

        if ((strstr(fn, ".wav") != NULL) ||
            (strstr(fn, ".WAV") != NULL))
        {
            if (FileList.ptr < FILEMGR_LIST_DEPDTH)
            {
                strncpy((char *)FileList.file[FileList.ptr].name, fn, FILEMGR_FILE_NAME_SIZE);

                FileList.file[FileList.ptr].type = FILETYPE_FILE;

                printf("  --> Added as Track %lu\r\n", (uint32_t)(FileList.ptr + 1));

                FileList.ptr++;
            }
            else
            {
                printf("  --> Playlist Full!\r\n");
            }
        }
    }

    f_closedir(&dir);

    printf("-------------------------------------\r\n");
    printf("Total WAV files found : %lu\r\n", (uint32_t)FileList.ptr);
    printf("-------------------------------------\r\n");

    return FR_OK;
}

