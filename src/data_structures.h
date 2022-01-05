
#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <DFPlayerMini_Fast.h>
/***************************************************
 Sounduino States and Events

 ****************************************************/

typedef enum
{
    notPlaying_State,
    playingSerial_State,
    playingShuffle_State,
    playingCard_State,
    playingStopdance_State,
    progWait_State,
    progDelete_State,
    progPending_State,
    deterPlayingState_State,
    menu_State,
    initializing_State,
    state_count
} SounduinoState;

// Events definition for Sounduino FSM
typedef enum
{
    bigRedSingle_Event,
    bigRedDouble_Event,
    bigRedLong_Event,
    songEnded_Event,
    fwdSingle_Event,
    prevSingle_Event,
    presentCard_Event,
    initialized_Event,
    determinationFailed_Event,
    cardProgrammingFailed_Event,
    cardProgrammed_Event,
    no_Event,
    event_count
} SounduinoEvent;

typedef struct
{
    bool paused;
    int currentRandomTrack;
} SounduinoStateProperties;

typedef struct
{
    SounduinoState state;
    SounduinoEvent event;
    SounduinoStateProperties stateProperties;
} SounduinoStateMachine;

//typedef of a function pointer to use in the following transition table
typedef SounduinoState (*SounduinoEventHandler)(DFPlayerMini_Fast, SounduinoStateMachine);
#endif
