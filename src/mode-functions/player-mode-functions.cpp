#include "mode-functions/player-mode-functions.hpp"
#include "file-manager/file-manager.hpp"

#include "SD.h"

extern fs::SDFS *ourSD;

void player_mode_single_press(Audio *audio, app_state_t *app_state)
{
    assert(audio != NULL);
    assert(app_state != NULL);

    audio->pauseResume();
}

void player_mode_double_press(Audio *audio, app_state_t *app_state)
{
    assert(audio != NULL);
    assert(app_state != NULL);

    // Ignore if we aleady are switching
    if (app_state->play_state == play_state_e::PLAY_STATE_SWITCHING)
    {
        return;
    }

    app_state->play_state = play_state_e::PLAY_STATE_SWITCHING;
    if (audio->isRunning())
    {
        audio->pauseResume();
    }

    audio->stopSong();

    // I2S error otherwise, buffer full
    delay(1000);

    const char *path = file_manager_get_music_path();

    // If we fail we do nothing, the user will just do it again - or will he?
    audio->connecttoFS(*ourSD, path);

    // Resulting state wont block this function from executing again in a fail state
    app_state->play_state = play_state_e::PLAY_STATE_PLAYING;
}