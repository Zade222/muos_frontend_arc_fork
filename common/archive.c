#include "muxshare.h"
#include "archive.h"
#include <string.h>
#include "json/json.h"

static const ArchiveHandler handlers[] = {
    {
        .extension = ".ssmc", 
        .type = HANDLER_TYPE_COMMAND, 
        .interface = {
            .commands = { 
                .list_command = "-m -j -i \"%s\"",
                .extract_command = "-e \"%s\" -i \"%s\" -o \"%s\"",
                .key_map = {
                    .root_path = NULL,
                    .filename_key = "filename",
                    .index_key = "index",
                    .size_key = "size(bytes)"
                }
            }
        }
    },
};


const ArchiveHandler* get_archive_handler(const char *filename) {
    for (int i = 0; handlers[i].extension != NULL; i++) {
        if (ends_with(filename, handlers[i].extension)) {
            return &handlers[i];
        }
    }
    return NULL; //None found
}


char *get_list_cmd(const char *filename) {
    for (int i = 0; handlers[i].interface.commands.list_command != NULL; i++){
        if (end_with(filename, handlers[i].interface.commands.list_command)){
            return &handlers[i].interface.commands.list_command;
        }
    }
    return NULL; //None found
}


struct json parse_archive_json(const char *exec_result, const JsonKeyMap *map) {
    struct json parsed = json_parse(exec_result);

    int count = json_array_count(parsed);
    
    size_t buffer_size = count * 256;
    char *buffer = malloc(buffer_size);
    if (!buffer){
        //handle error
    }

    int offset = snprintf(buffer, buffer_size, "[");

    for (int i = 0; i < count; i++){
        struct json item = json_array_get(parsed, i);

        struct json filename_json = json_object_get(item, map->filename_key);
        struct json index_json = json_object_get(item, map->index_key);
        struct json size_json = json_object_get(item, map->size_key);

        char filename[MAX]
    }
}