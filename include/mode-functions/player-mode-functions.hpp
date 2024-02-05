#ifndef PLAYER_MODE_FUNCTIONS_HPP
#define PLAYER_MODE_FUNCTIONS_HPP

#include "Audio.h"
#include "state/state.hpp"

void player_mode_single_press(Audio *audio, app_state_t *app_state);

void player_mode_double_press(Audio *audio, app_state_t *app_state);

#endif