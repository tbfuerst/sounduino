#include <data_structures.h>

char *stateToText(SounduinoState state)
{
    char *statetext[] = {
        "notPlaying_State",
        "playingSerial_State",
        "playingShuffle_State",
        "playingCard_State",
        "playingStopdance_State",
        "progWait_State",
        "progDelete_State",
        "progPending_State",
        "deterPlayingState_State",
        "menu_State",
        "initializing_State",
        "state_count"};
    return statetext[state];
}

char *eventToText(SounduinoEvent event)
{
    char *eventtext[] = {
        "bigRedSingle_Event",
        "bigRedDouble_Event",
        "bigRedLong_Event",
        "songEnded_Event",
        "fwdSingle_Event",
        "prevSingle_Event",
        "presentCard_Event",
        "initialized_Event",
        "determinationFailed_Event",
        "cardProgrammingFailed_Event"
        "cardProgrammed_Event",
        "no_Event",
        "event_count"};
    return eventtext[event];
}
