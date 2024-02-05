#ifndef GAME_MODE_FUNCTIONS_HPP
#define GAME_MODE_FUNCTIONS_HPP

#include "Audio.h"
#include "state/state.hpp"

void game_mode_single_press(Audio *audio, app_state_t *app_state);

void game_mode_double_press(Audio *audio, app_state_t *app_state);

#endif