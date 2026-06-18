/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file            : usb_host.c
  * @version         : v2.0_Cube
  * @brief           : This file implements the USB Host
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_msc.h"

/* USER CODE BEGIN Includes */
#include "ff.h"
/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USB Host core handle declaration */
USBH_HandleTypeDef hUsbHostFS;
ApplicationTypeDef Appli_state = APPLICATION_IDLE;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */
extern char USBHPath[4];   /* USBH logical drive path */
extern FATFS USBHFatFS;    /* File system object for USBH logical drive */

FRESULT res;
FIL myFile;
char buffer[256];


extern uint32_t Baud_Rate;
extern uint16_t LED_Delay;
extern int baudChanged;

/* USER CODE END 0 */

/*
 * user callback declaration
 */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * Init USB host library, add supported class and start the library
  * @retval None
  */
void MX_USB_HOST_Init(void)
{
  /* USER CODE BEGIN USB_HOST_Init_PreTreatment */

  /* USER CODE END USB_HOST_Init_PreTreatment */

  /* Init host Library, add supported class and start the library. */
  if (USBH_Init(&hUsbHostFS, USBH_UserProcess, HOST_FS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_RegisterClass(&hUsbHostFS, USBH_MSC_CLASS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_Start(&hUsbHostFS) != USBH_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_HOST_Init_PostTreatment */

  /* USER CODE END USB_HOST_Init_PostTreatment */
}

/*
 * user callback definition
 */
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id)
{
  /* USER CODE BEGIN CALL_BACK_1 */
	switch(id)
	{
	case HOST_USER_SELECT_CONFIGURATION:
		break;

	case HOST_USER_DISCONNECTION:
		Appli_state = APPLICATION_DISCONNECT;
		printf ("Application Disconnect\r\n");
		if (f_mount(NULL, USBHPath, 0) == FR_OK)
		{
			printf ("UnMount Successfully\r\n");
		}
		break;

	case HOST_USER_CLASS_ACTIVE:
		Appli_state = APPLICATION_READY;
		printf ("Application Ready\r\n");
		res = f_mount(&USBHFatFS, USBHPath, 0);
		if (res == FR_OK)
		{
			printf ("Mount Successful\r\n");

			UINT bytesRead;

			res = f_open(&myFile, "config.txt", FA_READ);
			if (res == FR_OK)
			{
				res = f_read(&myFile, buffer, sizeof(buffer)-1, &bytesRead);
				if (res == FR_OK)
				{
					buffer[bytesRead] = '\0';
					printf ("File Content\n");
					printf ("%s\r\n", buffer);

					char key[32];
					char value[32];

					char *line = strtok (buffer, "\r\n");

					while (line != NULL)
					{
						if (sscanf (line, " %31[^:]: \"%31[^\"]\"", key, value) == 2)
						{
							if (strcmp (key, "BAUD RATE") == 0)
							{
								Baud_Rate = atoi(value);
								baudChanged = 1;
								printf ("Baud Rate changed to %lu\r\n", Baud_Rate);
							}

							if (strcmp(key, "LED DELAY") == 0)
							{
								LED_Delay = atoi (value);
								printf ("LED Blink Delay is now %u\r\n", LED_Delay);
							}
						}

						line = strtok (NULL, "\r\n");

					}

				}
				else printf ("Unable to read config.txt, ERROR %d\r\n", res);

				f_close(&myFile);
			}

			else printf ("Unable to open config.txt, ERROR %d\r\n", res);


			if (f_open(&myFile, "test.TXT", FA_CREATE_ALWAYS|FA_WRITE) == FR_OK)
			{
				UINT bytesWritten;
				if (f_write(&myFile, "This is a test file\n", 20, &bytesWritten) == FR_OK)
				{
					printf ("%d bytes written to file test.TXT\r\n", bytesWritten);
				}

				f_close(&myFile);
			}
		}
		else
		{
			printf ("Unable to Mount\r\n");
		}
		break;

	case HOST_USER_CONNECTION:
		Appli_state = APPLICATION_START;
		printf ("Application Start\r\n");
		break;

	default:
		break;
	}
  /* USER CODE END CALL_BACK_1 */
}

/**
  * @}
  */

/**
  * @}
  */

