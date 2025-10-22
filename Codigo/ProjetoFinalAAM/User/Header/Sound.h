#ifndef Sounds_H_
#define Sounds_H_


#include "Sound_prm.h"
#include "SysError.h"

typedef enum {
	PLAY_ERROR,
	PLAY_NO_SOUND,   
	NUM_OF_SOUNDS,
} SOUNDS_TYPE;

//Prototipos
void Sounds__Initialize(void);
void Sounds__Background(void);
void Sounds__PlaySounds( SOUNDS_TYPE sound_id, ERROR_ID id);

SOUNDS_TYPE Sounds__GetSounds( void);   //novo




#endif /* Sounds_H_ */