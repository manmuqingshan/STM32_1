/*
 * defines.h
 *
 *  Created on: 30-Jun-2026
 *      Author: arunrawat
 */

#ifndef INC_DEFINES_H_
#define INC_DEFINES_H_

#include "main.h"
#include "usb_host.h"

/* Exported constants --------------------------------------------------------*/
#define FILEMGR_LIST_DEPDTH                     24
#define FILEMGR_FILE_NAME_SIZE                  40
#define FILEMGR_FULL_PATH_SIZE                 256
#define FILEMGR_MAX_LEVEL                        4
#define FILETYPE_DIR                             0
#define FILETYPE_FILE                            1

/* Exported types ------------------------------------------------------------*/
typedef struct AUDIO_Info_t {
  uint32_t ChunkID;       /* 0  */
  uint32_t FileSize;      /* 4  */
  uint32_t FileFormat;    /* 8  */
  uint32_t SubChunk1ID;   /* 12 */
  uint32_t SubChunk1Size; /* 16 */
  uint16_t AudioFormat;   /* 20 */
  uint16_t NbrChannels;   /* 22 */
  uint32_t SampleRate;    /* 24 */
  uint32_t ByteRate;      /* 28 */
  uint16_t BlockAlign;    /* 32 */
  uint16_t BitPerSample;  /* 34 */
  uint32_t SubChunk2ID;   /* 36 */
  uint32_t SubChunk2Size; /* 40 */
}WAV_InfoTypedef;

typedef struct _FILELIST_LineTypeDef {
  uint8_t type;
  uint8_t name[FILEMGR_FILE_NAME_SIZE];
}FILELIST_LineTypeDef;

typedef struct _FILELIST_FileTypeDef {
  FILELIST_LineTypeDef  file[FILEMGR_LIST_DEPDTH] ;
  uint16_t              ptr;
}FILELIST_FileTypeDef;

typedef enum {
  AUDIO_DEMO_IDLE = 0,
  AUDIO_DEMO_WAIT,
  AUDIO_DEMO_EXPLORE,
  AUDIO_DEMO_PLAYBACK,
  AUDIO_REENUMERATE
}AUDIO_App_State;

typedef enum {
  AUDIO_STATE_IDLE = 0,
  AUDIO_STATE_WAIT,
  AUDIO_STATE_INIT,
  AUDIO_STATE_CONFIG,
  AUDIO_STATE_PLAY,
  AUDIO_STATE_NEXT,
  AUDIO_STATE_PREVIOUS,
  AUDIO_STATE_FORWARD,
  AUDIO_STATE_BACKWARD,
  AUDIO_STATE_PAUSE,
  AUDIO_STATE_RESUME,
  AUDIO_STATE_VOLUME_UP,
  AUDIO_STATE_VOLUME_DOWN,
  AUDIO_STATE_ERROR,
}AUDIO_PLAYBACK_StateTypeDef;

typedef enum {
  AUDIO_ERROR_NONE = 0,
  AUDIO_ERROR_IO,
  AUDIO_ERROR_EOF,
}AUDIO_ErrorTypeDef;

typedef enum {
  AUDIO_KEY_NEXT = 0,
  AUDIO_KEY_PREVIOUS,
  AUDIO_KEY_PLAY_PAUSE,
  AUDIO_KEY_STOP,
}AUDIO_Playback_KeysTypeDef;



#endif /* INC_DEFINES_H_ */
