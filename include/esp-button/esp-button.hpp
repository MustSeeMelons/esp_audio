#include "Arduino.h"
#include "Bounce2.h"
#include "Audio.h"
#include "elapsedMillis.h"
#include "../state/state.hpp"

void esp_buttons_init(uint8_t vol_up_gpio, uint8_t vol_down_gpio, uint8_t action_gpio);

void esp_button_tick(
    Audio *audio,
    int8_t *volume,
    void (*signal)(),
    void (*single_trigger)(),
    void (*double_trigger)(),
    void (*long_trigger)());