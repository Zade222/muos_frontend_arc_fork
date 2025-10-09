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
                .tool_command = "sprite-shrink",
                .list_command = "-m -j -i \"%s\"",
                .extract_command = "-e \"%s\" -i \"%s\" -o \"%s\"",
                .key_map = {
                    .root_path = NULL,
                    .filename_key = "filename",
                    .identifier_key = "index",
                    .size_key = "size(bytes)"
                }
            }
        }
    },
};


const ArchiveHandler* get_archive_handler(const char *filename) {
    for (int i = 0; i < sizeof(handlers)/sizeof(handlers[0]); i++) {
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


char* parse_archive_json(const char *exec_result, const JsonKeyMap *map) {
    if (!exec_result || !json_valid(exec_result)) {
        //todo: handle invalid json
        return NULL;
    }

    struct json raw_parsed = json_parse(exec_result);

    struct json items_array = raw_parsed;

    if (map->root_path != NULL) {
        items_array = json_object_get(raw_parsed, map->root_path);
    }

    int count = json_array_count(items_array);

    if(count == 0) {
        return strdup("[]");
    }
    
    size_t buffer_size = strlen(exec_result) * 2;
    char *buffer = malloc(buffer_size);
    if (!buffer){
        //handle error
        return NULL;
    }

    int offset = snprintf(buffer, buffer_size, "[");

    for (int i = 0; i < count; i++){
        struct json raw_item = json_array_get(items_array, i);

        struct json filename_json = json_object_get(
            raw_item, 
            map->filename_key
        );
        struct json index_json = json_object_get(raw_item, map->identifier_key);
        struct json size_json = json_object_get(raw_item, map->size_key);



        char filename[MAX_BUFFER_SIZE];
        json_string_copy(filename_json, filename, sizeof(filename));
        int index = json_int(index_json);
        uint64_t size = json_uint64(size_json);

        offset += sprintf(buffer + offset, buffer_size - offset,
            "{\"filename\":\"%s\",\"index\":%d,\"size(bytes)\":%llu}%s",
            filename, index, (unsigned long long)size, 
            (i < count - 1) ? "," : "");

        if (offset >= buffer_size) {
            free(buffer);
            return NULL;
        }
    }

    sprintf(buffer + offset, buffer_size - offset, "]");

    return buffer;
}

char* construct_extract_cmd(
    const ArchiveHandler *handler, 
    const char *archive_path,
    const char *file_identifier,
    const char *output_path
) {
    const char *tool = handler->interface.commands.tool_command;
    const char *args_template = handler->interface.commands.extract_command;

    if (!tool || !args_template || !archive_path || !file_identifier || !output_path) {
        return NULL; //One or more of the parameters is invalid.
    }

    //Format the args_template with values
    int args_size = snprintf(NULL, 0, args_template,
        file_identifier, archive_path, output_path) + 1;
    
    char *formatted_args = malloc(args_size);
    if (!formatted_args) {
        return NULL; //Allocation failed
    }
    
    //Combine tool and formatted args
    int cmd_size = snprintf(NULL, 0, "%s %s", tool, formatted_args) + 1;
    char *command = malloc(cmd_size);
    if (!command) {
        free(formatted_args);
        return NULL; //Allocation failed
    }

    //Form command
    snprintf(command, cmd_size, "%s %s", tool, formatted_args);
    free(formatted_args);

    return command;
}