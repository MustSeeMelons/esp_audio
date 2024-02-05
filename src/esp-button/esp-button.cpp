#include "esp-button/esp-button.hpp"

static Bounce vol_down_bounce = Bounce();
static Bounce vol_up_bounce = Bounce();
static Bounce action_bounce = Bounce();

static elapsedMillis since_vol_up;
static elapsedMillis since_vol_down;
static elapsedMillis since_action_down;

static uint16_t vol_step = 200;

static uint16_t action_long_delay = 1000;     // Time for long to trigger
static uint16_t action_double_interval = 700; // Time in between triggers to trigger double
static bool long_init = false;                // True for a single down/up cycle
static bool double_init = false;              // True after a single down/up cycle
static bool is_trigger = false;               // Something was triggered, for proper behavior on release

static void esp_button_volume_down(Audio *audio, int8_t *volume, void (*signal)())
{
    (*volume)--;
    if (*volume < 0)
    {
        *volume = 0;

        assert(signal != NULL);

        signal();
    }

    assert(audio != NULL);

    audio->setVolume(*volume);
};

static void esp_button_volume_up(Audio *audio, int8_t *volume, void (*signal)())
{
    (*volume)++;
    if (*volume > 21)
    {
        *volume = 21; // Max possible value

        assert(signal != NULL);

        signal();
    }

    assert(audio != NULL);

    audio->setVolume(*volume);
}

/**
 * @brief Initialze all the bounces required
 * @param vol_up_gpio
 * @param vol_down_gpio
 * @param action_gpio
 */
void esp_buttons_init(uint8_t vol_up_gpio, uint8_t vol_down_gpio, uint8_t action_gpio)
{
    vol_down_bounce.attach(vol_down_gpio, INPUT_PULLUP);
    vol_up_bounce.attach(vol_up_gpio, INPUT_PULLUP);
    action_bounce.attach(action_gpio, INPUT_PULLUP);

    vol_down_bounce.interval(10);
    vol_up_bounce.interval(10);
    action_bounce.interval(10);
}

/**
 * @brief Update all the bounces and act accordingly
 *
 * @param audio
 * @param volume
 * @param signal
 */
void esp_button_tick(
    Audio *audio,
    int8_t *volume,
    void (*signal)(),
    void (*single_trigger)(),
    void (*double_trigger)(),
    void (*long_trigger)())
{
    assert(audio != NULL);
    assert(volume != NULL);
    assert(signal != NULL);

    // Perform the up dance
    vol_up_bounce.update();

    if (vol_up_bounce.changed())
    {
        Serial.println("vol_up");
        if (vol_up_bounce.read() == LOW)
        {
            esp_button_volume_up(audio, volume, signal);
            since_vol_up = 0;
        }
    }
    else
    {
        if (vol_up_bounce.read() == LOW)
        {
            if (since_vol_up > vol_step)
            {
                since_vol_up = 0;
                esp_button_volume_up(audio, volume, signal);
            }
        }
    }

    // Perform the down dance
    vol_down_bounce.update();

    if (vol_down_bounce.changed())
    {
        Serial.println("vol_down");
        if (vol_down_bounce.read() == LOW)
        {
            esp_button_volume_down(audio, volume, signal);
            since_vol_down = 0;
        }
    }
    else
    {
        if (vol_down_bounce.read() == LOW)
        {
            if (since_vol_down > vol_step)
            {
                since_vol_down = 0;
                esp_button_volume_down(audio, volume, signal);
            }
        }
    }

    // Perform the action
    action_bounce.update();

    if (action_bounce.changed())
    {
        uint8_t button_state = action_bounce.read();

        // Save stamp when button was pressed down, if double is not initialized
        if (button_state == LOW && !double_init)
        {
            since_action_down = 0;
            long_init = true;
        }

        if (button_state == LOW && double_init && action_double_interval > since_action_down)
        {
            double_init = false;
            long_init = false;
            is_trigger = true;

            assert(double_trigger != NULL);

            double_trigger();
        }

        if (button_state == HIGH && !is_trigger)
        {
            // Button released, mark as double initialized
            double_init = true;
            long_init = false;
        }

        if (button_state == HIGH)
        {
            is_trigger = false;
        }
    }
    else
    {
        uint8_t button_state = action_bounce.read();

        // If button is kept down for long enough - trigger long
        if (button_state == LOW && long_init && since_action_down >= action_long_delay)
        {
            double_init = false;
            long_init = false;
            is_trigger = true;

            assert(long_trigger != NULL);

            long_trigger();
        }

        if (button_state == HIGH && double_init && action_double_interval < since_action_down)
        {
            double_init = false;
            long_init = false;

            assert(single_trigger != NULL);

            single_trigger();
        }
    }
}