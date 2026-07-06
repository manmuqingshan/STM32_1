/* Includes ------------------------------------------------------------------ */
#include "main.h"
#include "stdio.h"
#include "defines.h"
#include "ff.h"
#include "usbh_audio.h"
#include "audio.h"

/* Private define ------------------------------------------------------------ */
#define AUDIO_BLOCK_SIZE  512
#define AUDIO_BLOCK_NBR   33

extern USBH_HandleTypeDef hUsbHostFS;
extern FILELIST_FileTypeDef FileList;
AUDIO_PLAYBACK_StateTypeDef audio_state;
AUDIO_HandleTypeDef *AUDIO_Handle = NULL;

/* Private macro ------------------------------------------------------------- */
/* Private typedef ----------------------------------------------------------- */
typedef struct
{
  uint8_t buff[AUDIO_BLOCK_SIZE * AUDIO_BLOCK_NBR];
  int32_t out_ptr;
  uint32_t in_ptr;
} Audio_BufferTypeDef;

/* Private variables --------------------------------------------------------- */
static WAV_InfoTypedef WavInfo;
static FIL WavFile;
static Audio_BufferTypeDef BufferCtl;
static int16_t FilePos = 0;

/* Private function prototypes ----------------------------------------------- */
static AUDIO_ErrorTypeDef AUDIO_GetFileInfo(uint16_t file_idx, WAV_InfoTypedef * info);
static void AUDIO_ResetPlaybackContext (void);

/* Private functions --------------------------------------------------------- */

/**
  * @brief  Starts Audio streaming.    
  * @param  idx: File index
  * @retval Audio error
  */
AUDIO_ErrorTypeDef AUDIO_Start(uint8_t idx)
{
  uint32_t bytesread;
  AUDIO_Handle = hUsbHostFS.pActiveClass->pData;

  if (FileList.ptr > idx)
  {
    f_close(&WavFile);

    AUDIO_GetFileInfo(idx, &WavInfo);

    if (WavInfo.AudioFormat != 0x01)
    {
      audio_state = AUDIO_STATE_ERROR;
      return AUDIO_ERROR_NONE;
    }
    else
    {
      audio_state = AUDIO_STATE_CONFIG;
    }

    /* Reset states before configuring Track */
    AUDIO_ResetPlaybackContext();

    /* Set Frequency */
    USBH_AUDIO_SetFrequency(&hUsbHostFS, WavInfo.SampleRate, WavInfo.NbrChannels, WavInfo.BitPerSample);

    /* Fill whole buffer at first time */
    if (f_read(&WavFile, &BufferCtl.buff[0], AUDIO_BLOCK_SIZE * AUDIO_BLOCK_NBR, (void *)&bytesread) == FR_OK)
    {
      if (bytesread != 0)
      {
    	  return AUDIO_ERROR_NONE;
      }
    }

  }
  return AUDIO_ERROR_IO;
}

/**
  * @brief  Manages Audio process. 
  * @param  None
  * @retval Audio error
  */
AUDIO_ErrorTypeDef AUDIO_Process(void)
{
	int32_t diff;
	uint32_t bytesread, elapsed_time;
	static uint32_t prev_elapsed_time = 0xFFFFFFFF;
	AUDIO_ErrorTypeDef error_state = AUDIO_ERROR_NONE;

	switch (audio_state)
	{
	case AUDIO_STATE_PLAY:
		BufferCtl.out_ptr = USBH_AUDIO_GetOutOffset(&hUsbHostFS);
		if (BufferCtl.out_ptr >= (AUDIO_BLOCK_SIZE * AUDIO_BLOCK_NBR)) /* End of buffer */
		{
			USBH_AUDIO_ChangeOutBuffer(&hUsbHostFS, &BufferCtl.buff[0]);
		}
		else
		{
			diff = BufferCtl.out_ptr - BufferCtl.in_ptr;

			if (diff < 0)
			{
				diff = AUDIO_BLOCK_SIZE * AUDIO_BLOCK_NBR + diff;
			}

			if (diff >= (AUDIO_BLOCK_SIZE * AUDIO_BLOCK_NBR / 2))
			{
				BufferCtl.in_ptr += AUDIO_BLOCK_SIZE;

				if (BufferCtl.in_ptr >= (AUDIO_BLOCK_SIZE * AUDIO_BLOCK_NBR))
				{
					BufferCtl.in_ptr = 0;
				}

				if (f_read(&WavFile, &BufferCtl.buff[BufferCtl.in_ptr], AUDIO_BLOCK_SIZE, (void *)&bytesread) != FR_OK)
				{
					f_close(&WavFile);
					error_state = AUDIO_ERROR_NONE;

				}
			}
		}

		/* Print elapsed time */
		elapsed_time = WavFile.fptr / WavInfo.ByteRate;
		if (prev_elapsed_time != elapsed_time)
		{
			prev_elapsed_time = elapsed_time;
			printf("[%02lu:%02lu]\r\n", elapsed_time / 60, elapsed_time % 60);
		}
		break;

	case AUDIO_STATE_NEXT:
		if (++FilePos >= FileList.ptr)
		{
			FilePos = 0;
		}
//		USBH_AUDIO_Stop(&hUsbHostFS);
		AUDIO_Start(FilePos);
		break;

	case AUDIO_STATE_PREVIOUS:
		if (--FilePos < 0)
		{
			FilePos = FileList.ptr - 1;
		}
//		USBH_AUDIO_Stop(&hUsbHostFS);
		AUDIO_Start(FilePos);
		break;

	case AUDIO_STATE_PAUSE:
		USBH_AUDIO_Suspend(&hUsbHostFS);
		audio_state = AUDIO_STATE_WAIT;
		break;

	case AUDIO_STATE_RESUME:
		USBH_AUDIO_Resume(&hUsbHostFS);
		audio_state = AUDIO_STATE_PLAY;
		break;

	case AUDIO_STATE_VOLUME_UP:
		USBH_AUDIO_SetVolume(&hUsbHostFS, VOLUME_UP);
		audio_state = AUDIO_STATE_PLAY;
		break;

	case AUDIO_STATE_VOLUME_DOWN:
		USBH_AUDIO_SetVolume(&hUsbHostFS, VOLUME_DOWN);
		audio_state = AUDIO_STATE_PLAY;
		break;

	case AUDIO_STATE_ERROR:
		printf ("[Unsupported Format]\r\n");
		audio_state = AUDIO_STATE_INIT;
		AUDIO_Handle->play_state = AUDIO_PLAYBACK_INIT;
		break;

	case AUDIO_STATE_WAIT:
	case AUDIO_STATE_CONFIG:
	case AUDIO_STATE_IDLE:
	case AUDIO_STATE_INIT:
	default:
		break;
	}
	return error_state;
}

/**
  * @brief  Stops Audio streaming.
  * @param  None
  * @retval Audio error
  */
AUDIO_ErrorTypeDef AUDIO_Stop(void)
{
  audio_state = AUDIO_STATE_IDLE;
  USBH_AUDIO_Stop(&hUsbHostFS);
  FilePos = 0;
  f_close(&WavFile);
  return AUDIO_ERROR_NONE;
}

/**
  * @brief  Probes the play back joystick state.
  * @param  state: Joystick state
  * @retval None
  */
void AUDIO_PlaybackKeys(AUDIO_Playback_KeysTypeDef key)
{
  /* Handle File List Selection */
  if (key == AUDIO_KEY_NEXT)
  {
	  audio_state = AUDIO_STATE_NEXT;
  }
  else if (key == AUDIO_KEY_PREVIOUS)
  {
	  audio_state = AUDIO_STATE_PREVIOUS;
  }
  else if (key == AUDIO_KEY_PLAY_PAUSE)
  {
    if (audio_state == AUDIO_STATE_WAIT)
    {
      audio_state = AUDIO_STATE_RESUME;
    }

    if (audio_state == AUDIO_STATE_PLAY)
    {
      audio_state = AUDIO_STATE_PAUSE;
    }
  }
  else if (key == AUDIO_KEY_STOP)
  {
	  audio_state = AUDIO_STATE_IDLE;
	  AUDIO_Stop();
  }
}

/**
  * @brief  Gets the file info.
  * @param  file_idx: File index
  * @param  info: Pointer to WAV file info
  * @retval Audio error
  */
static AUDIO_ErrorTypeDef AUDIO_GetFileInfo(uint16_t file_idx, WAV_InfoTypedef * info)
{
  uint32_t bytesread;
  uint32_t duration;

  if (f_open (&WavFile, (char *)FileList.file[file_idx].name, FA_OPEN_EXISTING | FA_READ) == FR_OK)
  {
    /* Fill the buffer to Send */
    if (f_read(&WavFile, info, sizeof(WAV_InfoTypedef), (void *)&bytesread) == FR_OK)
    {
      printf("Playing file (%d/%d): %s\r\n", file_idx + 1, FileList.ptr, (char *)FileList.file[file_idx].name);

      printf("Sample rate : %lu Hz\r\n", info->SampleRate);

      printf("Channels number : %d\r\n", info->NbrChannels);

      duration = info->FileSize / info->ByteRate;
      printf("File Size : %lu MB [%02lu:%02lu]\r\n", info->FileSize / 1024 / 1024, duration / 60, duration % 60);

      return AUDIO_ERROR_NONE;
    }
    f_close(&WavFile);
  }
  return AUDIO_ERROR_IO;
}

/**
  * @brief  Informs user that settings have been changed.
  * @param  phost: Host Handle
  * @retval None
  */
void USBH_AUDIO_FrequencySet(USBH_HandleTypeDef * phost)
{
	if (audio_state == AUDIO_STATE_CONFIG)
	{
		/* Start first read */
		USBH_AUDIO_Play(&hUsbHostFS, &BufferCtl.buff[0], WavInfo.FileSize);
		audio_state = AUDIO_STATE_PLAY;
	}
}


static void AUDIO_ResetPlaybackContext (void)
{
    AUDIO_Handle->headphone.global_ptr  = 0;
    AUDIO_Handle->headphone.partial_ptr = 0;
    AUDIO_Handle->headphone.cbuf        = NULL;
    AUDIO_Handle->headphone.buf         = NULL;

    AUDIO_Handle->play_state = AUDIO_PLAYBACK_IDLE;
}

void USBH_AUDIO_BufferEmptyCallback(USBH_HandleTypeDef *phost)
{
	audio_state = AUDIO_STATE_NEXT;
}

