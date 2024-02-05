#include "mode-functions/game-mode-functions.hpp"
#include "file-manager/file-manager.hpp"
#include "SD.h"

extern fs::SDFS *ourSD;

void game_mode_single_press(Audio *audio, app_state_t *app_state)
{
    assert(audio != NULL);
    assert(app_state != NULL);

    // Do nothing if clip is playing or we are in standby mode
    if (
        app_state->mode == app_mode_t::MODE_STANDBY ||
        app_state->game_state == guess_state_e::GUESS_PLAYING_CLIP ||
        app_state->game_state == guess_state_e::GUESS_PLAYING_ANSWER)
    {
        return;
    }

    // If we are in answer repeat mode - repeat the answer and leave, duh
    if (app_state->game_state == guess_state_e::GUESS_ANSWER_REPEAT)
    {
        // Play answer, wait - continue with the next
        const char *answer_path = file_manager_get_answer_path();
        if (answer_path)
        {
            bool result = audio->connecttoFS(*ourSD, answer_path);

            if (result)
            {
                app_state->game_state = guess_state_e::GUESS_PLAYING_ANSWER;
            }

            return;
        }
    }

    // If nothing is playing check if we have something to repeat, get new otherwise
    const char *path = file_manager_get_current_file_path();

    if (path != NULL)
    {
        bool result = audio->connecttoFS(*ourSD, path);
        if (result)
        {
            app_state->game_state = guess_state_e::GUESS_PLAYING_CLIP;
        }
    }
    else
    {
        const char *path = file_manager_get_new_file_path();

        if (path != NULL)
        {
            bool result = audio->connecttoFS(*ourSD, path);

            if (result)
            {
                app_state->game_state = guess_state_e::GUESS_PLAYING_CLIP;
            }
        }
    }
}

void game_mode_double_press(Audio *audio, app_state_t *app_state)
{
    assert(audio != NULL);
    assert(app_state != NULL);

    // Do nothing if clip is playing or we are in standby mode
    if (app_state->mode == app_mode_t::MODE_STANDBY || app_state->game_state == guess_state_e::GUESS_PLAYING_CLIP)
    {
        return;
    }

    // If we are waiting on continue - enter answer repeat mode
    if (app_state->game_state == guess_state_e::GUESS_CONTINUE)
    {
        // Play answer, wait - continue with the next
        const char *answer_path = file_manager_get_answer_path();
        if (answer_path)
        {
            bool result = audio->connecttoFS(*ourSD, answer_path);

            if (result)
            {
                app_state->game_state = guess_state_e::GUESS_PLAYING_ANSWER;
            }

            return;
        }
    }

    // Time to move on to the next
    if (app_state->game_state == guess_state_e::GUESS_ANSWER_REPEAT)
    {
        const char *path = file_manager_get_new_file_path();

        if (path != NULL)
        {
            bool result = audio->connecttoFS(*ourSD, path);
            if (result)
            {
                app_state->game_state = guess_state_e::GUESS_PLAYING_CLIP;
            }
        }
    }
}