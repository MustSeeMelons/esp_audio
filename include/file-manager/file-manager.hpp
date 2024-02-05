#define MAX_FULL_PATH_LEN 192
#define FILE_NAME_MAX_LENGTH 96
#define PLAYER_FILE_COUNT 110
#define FOLDER_COUNT 30

void file_manager_init();

char *file_manager_get_current_file_path();

char *file_manager_get_new_file_path();

char *file_manager_get_answer_path();

char *file_manager_get_music_path();