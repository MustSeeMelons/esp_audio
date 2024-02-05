#include <Arduino.h>
#include "Audio.h"
#include "SD.h"
#include "FS.h"
#include "state/state.hpp"
#include "esp-button/esp-button.hpp"
#include "file-manager/file-manager.hpp"
#include "mode-functions/game-mode-functions.hpp"
#include "mode-functions/player-mode-functions.hpp"

// All the pins
#define SD_CS 5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18

#define I2S_DOUT 22
#define I2S_BCLK 26
#define I2S_LRC 25

#define VOL_DOWN 16
#define VOL_UP 15
#define ACTION 12
#define LED 2

app_state_t app_state = {
    .mode = app_mode_t::MODE_STANDBY,
    .game_state = guess_state_e::GUESS_STANDBY,
};

Audio audio;

int8_t volume = 10;

TaskHandle_t button_loop_handle;
TaskHandle_t led_effect_handle;

fs::SDFS *ourSD = &SD;

// LED Task functions
void slow_blink(void *params)
{
  for (;;)
  {
    digitalWrite(LED, HIGH);
    delay(1000);
    digitalWrite(LED, LOW);
    delay(1000);
  }
}

void medium_blink(void *params)
{
  for (;;)
  {
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
  }
}

void fast_blink(void *params)
{
  for (;;)
  {
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(100);
  }
}

void signal_blink(void *params)
{
  for (;;)
  {
    digitalWrite(LED, HIGH);
    delay(50);
    digitalWrite(LED, LOW);
    delay(50);
  }
}

// LED Task switchers
void start_slow_blink()
{
  if (led_effect_handle != NULL)
  {
    vTaskDelete(led_effect_handle);
    led_effect_handle = NULL;
  }

  xTaskCreatePinnedToCore(slow_blink, "slow_blink", 2048, NULL, 1, &led_effect_handle, 0);
}

void start_medium_blink()
{
  if (led_effect_handle != NULL)
  {
    vTaskDelete(led_effect_handle);
    led_effect_handle = NULL;
  }

  xTaskCreatePinnedToCore(medium_blink, "medium_blink", 2048, NULL, 1, &led_effect_handle, 0);
}

void start_fast_blink()
{
  if (led_effect_handle != NULL)
  {
    vTaskDelete(led_effect_handle);
    led_effect_handle = NULL;
  }

  xTaskCreatePinnedToCore(fast_blink, "fast_blink", 2048, NULL, 1, &led_effect_handle, 0);
}

void start_signal_blink()
{
  if (led_effect_handle != NULL)
  {
    vTaskDelete(led_effect_handle);
    led_effect_handle = NULL;
  }

  xTaskCreatePinnedToCore(signal_blink, "signal_blink", 2048, NULL, 1, &led_effect_handle, 0);

  delay(1000);

  switch (app_state.mode)
  {
  case app_mode_t::MODE_STANDBY:
    start_slow_blink();

    break;
  case app_mode_t::MODE_GUESS:
    start_medium_blink();

    break;
  case app_mode_t::PLAYER:
    start_fast_blink();
    break;
  }
}

// Button trigger handlers
void single_press()
{
  switch (app_state.mode)
  {
  case app_mode_t::MODE_GUESS:
    game_mode_single_press(&audio, &app_state);
    break;
  case app_mode_t::PLAYER:
    player_mode_single_press(&audio, &app_state);
    break;
  default:
    break;
  }
}

// Switches between modes
void long_press()
{
  switch (app_state.mode)
  {
  case app_mode_t::MODE_STANDBY:
    // From standby we move on to the game
    app_state.mode = app_mode_t::MODE_GUESS;
    app_state.game_state = guess_state_e::GUESS_CONTINUE;
    start_medium_blink();
    break;
  case app_mode_t::MODE_GUESS:
    // From game we move on to music player
    app_state.mode = app_mode_t::PLAYER;
    app_state.play_state = play_state_e::PLAY_STATE_PLAYING;
    player_mode_double_press(&audio, &app_state);

    start_fast_blink();
    break;
  case app_mode_t::PLAYER:
    // From music we go back to standby
    app_state.mode = app_mode_t::MODE_STANDBY;

    start_slow_blink();

    // Stop any music playing
    audio.stopSong();
    break;
  }
}

void double_press()
{

  switch (app_state.mode)
  {
  case app_mode_t::MODE_GUESS:
    game_mode_double_press(&audio, &app_state);
    break;
  case app_mode_t::PLAYER:
    player_mode_double_press(&audio, &app_state);
    break;
  default:
    break;
  }
}

void button_loop_task(void *params)
{
  for (;;)
  {
    esp_button_tick(&audio, &volume, &start_signal_blink, &single_press, &double_press, &long_press);
    delay(1); // Give other tasks time to do their thing
  }
}

void setup()
{
  // Show we are initializing
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  // Select out SD card
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  Serial.begin(115200);

  if (!ourSD->begin(SD_CS))
  {
    Serial.println("Error accessing microSD card!");
    digitalWrite(LED, HIGH);
    while (true)
    {
    }
  }

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

  audio.setVolume(volume);

  file_manager_init();

  esp_buttons_init(VOL_UP, VOL_DOWN, ACTION);

  xTaskCreatePinnedToCore(button_loop_task, "button_loop_task", 4096, NULL, 1, &button_loop_handle, 0);

  xTaskCreatePinnedToCore(slow_blink, "slow_blink", 1024, NULL, 1, &led_effect_handle, 0);

  // Init done
  digitalWrite(LED, LOW);
}

void loop()
{
  audio.loop();
}

// Updates guess state when audio is done playing
void audio_eof_mp3(const char *info)
{
  switch (app_state.mode)
  {
  case app_mode_t::MODE_GUESS:
    switch (app_state.game_state)
    {
    case guess_state_e::GUESS_PLAYING_ANSWER:
      app_state.game_state = guess_state_e::GUESS_ANSWER_REPEAT;
      break;
    default:
      app_state.game_state = guess_state_e::GUESS_CONTINUE;
      break;
    }
    break;

  case app_mode_t::PLAYER:
    player_mode_double_press(&audio, &app_state);
    break;

  default:
    break;
  }
}