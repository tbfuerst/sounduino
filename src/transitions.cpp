#include <Arduino.h>
#include <utility.h>
#include <DFPlayerMini_Fast.h>

/***************************************************
 Sounduino Transition functions

 ****************************************************/

//function pointer to store last playing State to transition to
SounduinoEventHandler transToLastPlayState;
void saveLastPlayState(SounduinoStateMachine fsm);

void changeLastPlayStateTransitionFunction(SounduinoEventHandler transitionFunctionToAssign){
    transToLastPlayState = transitionFunctionToAssign;
}

void playRandom(DFPlayerMini_Fast mp3player, SounduinoStateMachine fsm)
{
    int randomTrack = 0;

    do
    {
        randomTrack = random(1, 3 + 1);
        Serial.print("lastRandom: ");
        Serial.print(fsm.stateProperties.currentRandomTrack);
        Serial.print(" || calculated Random:");
        Serial.println(randomTrack);
    } while (randomTrack == fsm.stateProperties.currentRandomTrack);

    mp3player.playFromMP3Folder(randomTrack);
    fsm.stateProperties.currentRandomTrack = randomTrack;
}

SounduinoState transNotPlaying(DFPlayerMini_Fast mp3player, SounduinoStateMachine fsm)
{

    if (fsm.state != initializing_State)
    {
        saveLastPlayState(fsm);
        mp3player.pause();
    }
    else
    {
        mp3player.stop();
        fsm.stateProperties.paused = false;
        mp3player.resume();
    }
    Serial.println("Transition to: not Playing");
    return notPlaying_State;
}

SounduinoState transPlayingSerial(DFPlayerMini_Fast mp3player, SounduinoStateMachine fsm)
{

    // start new playlist from 1 if serial is selected via menu, otherwise just resume
    // Announce new play mode if playmode is selected via menu, but not at just leaving the menu
    if (fsm.state == menu_State && fsm.event == bigRedSingle_Event)
    {
        if (fsm.stateProperties.paused)
            mp3player.resume();
        else
            mp3player.play(1);
        mp3player.playAdvertisement(8);
    }

    // handle pause
    if (fsm.state == notPlaying_State)
    {
        if (fsm.stateProperties.paused)
            mp3player.resume();
        else
        {
            mp3player.play(1);
        }
    }

    Serial.println("Transition to: playingSerial");
    return playingSerial_State;
}
SounduinoState transPlayingShuffle(DFPlayerMini_Fast mp3player, SounduinoStateMachine fsm)
{
    // start new random playlist if shuffle is selected via menu, otherwise just resume
    // Announce new play mode if playmode is selected via menu, but not at just leaving the menu
    if (fsm.state == menu_State && fsm.event == bigRedSingle_Event)
    {
        if (fsm.stateProperties.paused)
        {
            mp3player.resume();
        }
        else
        {
            playRandom(mp3player, fsm);
        }
        //randomTrackinFolder(1);

        mp3player.playAdvertisement(8);
    }

    // if pressed on next or Previous
    if (fsm.state == playingShuffle_State && (fsm.event == fwdSingle_Event || fsm.event == prevSingle_Event))
    {

        playRandom(mp3player, fsm);
    }

    // handle pause
    if (fsm.state == notPlaying_State)
    {
        if (fsm.stateProperties.paused)
        {
            mp3player.resume();
        }
        else
        {
            Serial.println("random");
            playRandom(mp3player, fsm);
            //randomTrackinFolder(1);
        }
    }

    Serial.println("Transition to: playingShuffle");
    return playingShuffle_State;
}
SounduinoState transPlayingCard(DFPlayerMini_Fast mp3player, SounduinoStateMachine fsm)
{
    Serial.println("Transition to: playingCard");
    return playingCard_State;
}
SounduinoState transPlayingStopdance(DFPlayerMini_Fast mp3player, SounduinoStateMachine fsm)
{
    // Announce new play mode if playmode is selected via menu, but not at just leaving the menu
    if (fsm.state == menu_State && fsm.event != bigRedDouble_Event)
        mp3player.playAdvertisement(9);

    Serial.println("Transition to: playingStopdance");
    return playingStopdance_State;
}
SounduinoState transProgWait(DFPlayerMini_Fast mp3player, SounduinoStateMachine fsm)
{
    saveLastPlayState(fsm);
    mp3player.playAdvertisement(6);
    Serial.println("Transition to: progWait");
    return progWait_State;
}
SounduinoState transProgDelete(DFPlayerMini_Fast mp3player, SounduinoStateMachine fsm)
{

    Serial.println("Transition to: progDelete");
    mp3player.playAdvertisement(3);
    return progDelete_State;
}
SounduinoState transProgPending(DFPlayerMini_Fast mp3player, SounduinoStateMachine fsm)
{
    mp3player.playAdvertisement(5);
    Serial.println("Transition to: progPending");
    return progPending_State;
}
SounduinoState transDeterPlayingState(DFPlayerMini_Fast mp3player, SounduinoStateMachine fsm)
{
    // determine state to transition to
    Serial.print("via deterPlayingState_State, ");
    return transToLastPlayState(mp3player, fsm);
}
SounduinoState transMenu(DFPlayerMini_Fast mp3player, SounduinoStateMachine fsm)
{
    saveLastPlayState(fsm);
    mp3player.playAdvertisement(2);
    Serial.println("Transition to: menu");
    return menu_State;
}

SounduinoState playHelp(DFPlayerMini_Fast mp3player, SounduinoStateMachine fsm)
{
    mp3player.playAdvertisement(1);
    Serial.println("Transition to: menu");
    return menu_State;
}
SounduinoState transInitializing(DFPlayerMini_Fast mp3player, SounduinoStateMachine fsm)
{
    Serial.println("Transition to: initializing");
    return initializing_State;
}
SounduinoState doNothing(DFPlayerMini_Fast mp3player, SounduinoStateMachine fsm)
{
    Serial.println("Nothing happens");
    return fsm.state;
}

void saveLastPlayState(SounduinoStateMachine fsm)
{
    fsm.stateProperties.paused = true;
    switch (fsm.state)
    {
    case playingShuffle_State:
        transToLastPlayState = transPlayingShuffle;
        break;
    case playingSerial_State:
        transToLastPlayState = transPlayingSerial;
        break;
    case notPlaying_State:
        transToLastPlayState = transNotPlaying;
        break;
    case playingCard_State:
        transToLastPlayState = transPlayingCard;
        break;
    case playingStopdance_State:
        transToLastPlayState = transPlayingStopdance;
        break;
    default:
        fsm.stateProperties.paused = false;
        transToLastPlayState = transPlayingShuffle;
        break;
    }
}
