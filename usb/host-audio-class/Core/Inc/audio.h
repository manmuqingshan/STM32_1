/*
 * usb_audio.h
 *
 *  Created on: 30-Jun-2026
 *      Author: arunrawat
 */

#ifndef INC_AUDIO_H_
#define INC_AUDIO_H_

#include "defines.h"

AUDIO_ErrorTypeDef AUDIO_Start(uint8_t idx);
AUDIO_ErrorTypeDef AUDIO_Process(void);
AUDIO_ErrorTypeDef AUDIO_Stop(void);
void AUDIO_PlaybackKeys(AUDIO_Playback_KeysTypeDef key);

#endif /* INC_USB_AUDIO_H_ */
