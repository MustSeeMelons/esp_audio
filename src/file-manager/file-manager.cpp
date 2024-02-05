#include "file-manager/file-manager.hpp"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "SD.h"
#include "FS.h"
#include <memory.h>
#include "esp_system.h"

/**
 * Notes:
 * 0. Hardcoding everything might be fastest, but no. Adding new would require rebuild. No.
 * 1. We could scan all folders on startup.
 * 2. We could stash all we find, but then we would need to validate it all on startup on prolly rescan - seems better to just scan on startup.
 */

extern fs::SDFS *ourSD;

// Jus the folder path
static char clip_folder_path[MAX_FULL_PATH_LEN] = {0};
// Folder path which we quickly modify with a .mp3 to get the answer clip
static char clip_temp_path[MAX_FULL_PATH_LEN] = {0};
// Where we shall put the resulting full path to a file
static char clip_file_path[MAX_FULL_PATH_LEN] = {0};
// Where we put all the files found in a folder
static char clip_paths[FOLDER_COUNT][FILE_NAME_MAX_LENGTH] = {0};
// We we hide all the music file paths
static char music_paths[PLAYER_FILE_COUNT][FILE_NAME_MAX_LENGTH] = {0};

// The clip_root folder
char clip_root[] = "/lamuvardi/";
// The other root folder
char player_root[] = "/music";

static uint8_t music_file_count = 0;
static uint8_t last_music_idx = 0;
static uint8_t last_folder_idx = 0;

// SD did not like a trailing slash, so removed it
// Subfolders with the meanings
char atrsirdigs_parsteigts[] = "atrsirdigs_parsteigts";
char ausigs_vieglpratigs_akstigs[] = "ausigs_vieglpratigs_akstigs";
char bailulis_glevulis[] = "bailulis_glevulis";
char bledis[] = "bledis";
char cikstulis_gaudulis_igna[] = "cikstulis_gaudulis_igna";
char dzerajs[] = "dzerajs";
char gardedis[] = "gardedis";
char gauss_lenigs[] = "gauss_lenigs";
char iedomigs[] = "iedomigs";
char izklaidigs_netikls[] = "izklaidigs_netikls";
char kaskigs_stridigs[] = "kaskigs_stridigs";
char launs_nikns[] = "launs_nikns";
char lieligs[] = "lieligs";
char mantrausis[] = "mantrausis";
char melis[] = "melis";
char miegains[] = "miegains";
char nabadzigs[] = "nabadzigs";
char neizdarigs_neuznemigs_nevarigs[] = "neizdarigs_neuznemigs_nevarigs";
char netirigs_nevizigs[] = "netirigs_nevizigs";
char pielidejs[] = "pielidejs";
char plapa[] = "plapa";
char rima[] = "rima";
char skops[] = "skops";
char slinkis[] = "slinkis";
char stulbs[] = "stulbs";
char tukspratis[] = "tukspratis";
char tula_lempis[] = "tula_lempis";
char uzbazigs[] = "uzbazigs";
char zaglis[] = "zaglis";
char zinkarigs[] = "zinkarigs";

// Some sort of index for all the folders
char *folders[FOLDER_COUNT] = {
    atrsirdigs_parsteigts,
    ausigs_vieglpratigs_akstigs,
    bailulis_glevulis,
    bledis,
    cikstulis_gaudulis_igna,
    dzerajs,
    gardedis,
    gauss_lenigs,
    iedomigs,
    izklaidigs_netikls,
    kaskigs_stridigs,
    launs_nikns,
    lieligs,
    mantrausis,
    melis,
    miegains,
    nabadzigs,
    neizdarigs_neuznemigs_nevarigs,
    netirigs_nevizigs,
    pielidejs,
    plapa,
    rima,
    skops,
    slinkis,
    stulbs,
    tukspratis,
    tula_lempis,
    uzbazigs,
    zaglis,
    zinkarigs,
};

void file_manager_init()
{
    File dir = ourSD->open(player_root);

    while (true)
    {
        File entry = dir.openNextFile();

        if (!entry)
        {
            break;
        }

        if (!entry.isDirectory())
        {
            const char *name = entry.name();

            if (name != NULL)
            {
                // Stick the full file path
                char full_path[MAX_FULL_PATH_LEN] = {};
                int8_t result = sprintf(full_path, "%s/%s", player_root, name);

                if (result > 0)
                {
                    // Store it safely
                    memccpy(music_paths[music_file_count], full_path, '\0', FILE_NAME_MAX_LENGTH);
                    music_file_count++;
                }
            }

            entry.close();
        }

        // In case there are too many files
        if (music_file_count >= PLAYER_FILE_COUNT)
        {
            break;
        }
    }
}

char *file_manager_get_music_path()
{
    uint8_t idx = esp_random() % music_file_count;

    // This will never play the first file, but will wrap around nicely
    if (idx == last_music_idx)
    {
        idx++;
        idx %= music_file_count;
    }

    last_music_idx = idx;

    return music_paths[idx];
}

char *file_manager_get_answer_path()
{
    if (clip_folder_path[0] != 0)
    {
        sprintf(clip_temp_path, "%s%s", clip_folder_path, ".mp3");
        return clip_temp_path;
    }

    return NULL;
}

char *file_manager_get_current_file_path()
{
    if (clip_file_path[0] != 0)
    {
        return clip_file_path;
    }

    return NULL;
}

char *file_manager_get_new_file_path()
{
    // Yeet the possibly old
    memset(clip_folder_path, 0, sizeof(char) * MAX_FULL_PATH_LEN);
    memset(clip_temp_path, 0, sizeof(char) * MAX_FULL_PATH_LEN);
    memset(clip_file_path, 0, sizeof(char) * MAX_FULL_PATH_LEN);

    // Select a folder
    uint8_t folder_index = esp_random() % FOLDER_COUNT;

    if (folder_index == last_folder_idx)
    {
        folder_index++;
        folder_index %= FOLDER_COUNT;
    }

    last_folder_idx = folder_index;

    char *folder = folders[folder_index];

    // Get our folder
    uint8_t result = sprintf(clip_folder_path, "%s%s", clip_root, folder);

    if (result < 0)
    {
        return NULL;
    }

    // Open up the folder
    File dir = ourSD->open(clip_folder_path);

    uint8_t file_counter = 0;

    // Iterate all files in it
    while (true)
    {
        File entry = dir.openNextFile();

        if (!entry)
        {
            break;
        }

        if (!entry.isDirectory())
        {
            const char *name = entry.name();

            if (name != NULL)
            {
                memccpy(clip_paths[file_counter], entry.name(), '\0', FILE_NAME_MAX_LENGTH);

                file_counter++;
            }

            entry.close();
        }
    }

    uint8_t file_index = esp_random() % file_counter;

    result = sprintf(clip_file_path, "%s%s/%s", clip_root, folder, clip_paths[file_index]);

    if (result < 0)
    {
        return NULL;
    }

    return clip_file_path;
}
