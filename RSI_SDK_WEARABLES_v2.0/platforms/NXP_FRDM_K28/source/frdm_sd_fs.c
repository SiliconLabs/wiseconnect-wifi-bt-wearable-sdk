/*
 * frdm_sd_fs.c
 *
 *  Created on: 15-Nov-2018
 *      Author: root
 */

#include "stdint.h"
#include "ff.h"
#include "diskio.h"

//! SD Card parameters
static FATFS g_fileSystem; /* File system object */

/*******************************************************************************
 * Code
 ******************************************************************************/


int8_t sd_filesystem_init()
{
	FRESULT error;

	const TCHAR driverNumberBuffer[3U] = {SDDISK + '0', ':', '/'};

	//PRINTF("\r\nPlease insert a card into board.\r\n");

	if (f_mount(&g_fileSystem, driverNumberBuffer, 0U))
	{
		//PRINTF("Mount volume failed.\r\n");
		return -1;
	}

#if (_FS_RPATH >= 2U)
	error = f_chdrive((char const *)&driverNumberBuffer[0U]);
	if (error)
	{
		//PRINTF("Change drive failed.\r\n");
		return -1;
	}
#endif
	return FR_OK;
}
