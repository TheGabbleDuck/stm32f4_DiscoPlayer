#ifndef __AUDIOIF_H
#define __AUDIOIF_H
//	Audio API level for codec to use

//	defines
#include "i2c.h"
#include "i2s.h"

//	Prototypes
void codecReset(void);
void codecPwrUp(void);
void codecBeep(unsigned int Freq, unsigned int length);
void codecAudioBusInit(void);
void codecControlBusInit(void);

//	Data structures

//	typedef for codec initialisation
//	experimental
typedef struct{
	//	include params here for codec initialisation
	union{
		struct{
			//	CS43L22 register defs / defaults
			unsigned short regDef1;
		}csRegDefs;
		unsigned short initArray[10];
	};
}tCS43L22InitStruct;


#endif
