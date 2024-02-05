#ifndef STATE_HPP
#define STATE_HPP

// Global modes
typedef enum
{
    MODE_STANDBY,
    MODE_GUESS,
    PLAYER
} app_mode_t;

// Guess game states
typedef enum
{
    GUESS_STANDBY,        // Idle, doing nothing
    GUESS_PLAYING_CLIP,   // Clip is being played, locked till it completes
    GUESS_ANSWER_REPEAT,  // Answer is being repeated, lock
    GUESS_PLAYING_ANSWER, // Answer is being played, locked till it completes
    GUESS_CONTINUE,       // Repeat or go on to next clip

} guess_state_e;

typedef enum
{
    PLAY_STATE_PLAYING,
    PLAY_STATE_SWITCHING
} play_state_e;

typedef struct
{
    app_mode_t mode;
    union
    {
        guess_state_e game_state;
        play_state_e play_state;
    };

} app_state_t;

#endif