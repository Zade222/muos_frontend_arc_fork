#pragma once

typedef enum {
    HANDLER_TYPE_COMMAND,
    HANDLER_TYPE_LIBRARY
} ArchiveInterfaceType;

typedef struct {
    const char *root_path;// Path to the array if nested (e.g., "files"), or NULL
    const char *filename_key;
    const char *identifier_key;
    const char *size_key;
} JsonKeyMap;

typedef union {
    //For a format that will use binary command integration
    struct {
        const char *tool_command;
        const char *list_command;
        const char *extract_command;
        JsonKeyMap key_map;
    } commands;

    //For a format that will use library integration
    struct {
        /*Should return a null-terminated array of strings (filenames)
        The second argument `count` is an out-parameter for the number of
        files.*/
        char** (*list_files)(const char *archive_path, int *count);

        /*Should extract a single file to a destination path. Returns 0 on
        success.*/
        int (*extract_file)(const char *archive_path, 
            const char *file_inside_archive, 
            const char *destination_path
        );
    } library;
} ArchiveInterface;

typedef struct {
    const char *extension; //e.g., ".zip"
    ArchiveInterfaceType type;
    ArchiveInterface interface;
} ArchiveHandler;


const ArchiveHandler* get_archive_handler(const char *filename);

char* get_list_cmd(const char *filename);

char* parse_archive_json(const char *exec_result, const JsonKeyMap *map);

char* construct_extract_cmd(
    const ArchiveHandler *handler, 
    const char *archive_path,
    const char *file_identifier,
    const char *output_path
);