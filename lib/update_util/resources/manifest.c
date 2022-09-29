#include "manifest.h"

#include <toolbox/stream/buffered_file_stream.h>
#include <toolbox/hex.h>

struct ResourceManifestReader {
    Storage* storage;
    Stream* stream;
    string_t linebuf;
    ResourceManifestEntry entry;
};

ResourceManifestReader* resource_manifest_reader_alloc(Storage* storage) {
    ResourceManifestReader* resource_manifest =
        (ResourceManifestReader*)malloc(sizeof(ResourceManifestReader));
    resource_manifest->storage = storage;
    resource_manifest->stream = buffered_file_stream_alloc(resource_manifest->storage);
    memset(&resource_manifest->entry, 0, sizeof(ResourceManifestEntry));
    string_init(resource_manifest->entry.name);
    string_init(resource_manifest->linebuf);
    return resource_manifest;
}

void resource_manifest_reader_free(ResourceManifestReader* resource_manifest) {
    furi_assert(resource_manifest);

    string_clear(resource_manifest->linebuf);
    string_clear(resource_manifest->entry.name);
    buffered_file_stream_close(resource_manifest->stream);
    stream_free(resource_manifest->stream);
    free(resource_manifest);
}

bool resource_manifest_reader_open(ResourceManifestReader* resource_manifest, const char* filename) {
    furi_assert(resource_manifest);

    return buffered_file_stream_open(
        resource_manifest->stream, filename, FSAM_READ, FSOM_OPEN_EXISTING);
}

/* Read entries in format of
 * F:<hash>:<size>:<name>
 * D:<name> 
 */
ResourceManifestEntry* resource_manifest_reader_next(ResourceManifestReader* resource_manifest) {
    furi_assert(resource_manifest);

    string_reset(resource_manifest->entry.name);
    resource_manifest->entry.type = ResourceManifestEntryTypeUnknown;
    resource_manifest->entry.size = 0;
    memset(resource_manifest->entry.hash, 0, sizeof(resource_manifest->entry.hash));

    do {
        if(!stream_read_line(resource_manifest->stream, resource_manifest->linebuf)) {
            return NULL;
        }

        /* Trim end of line */
        string_strim(resource_manifest->linebuf);

        char type_code = string_get_char(resource_manifest->linebuf, 0);
        switch(type_code) {
        case 'F':
            resource_manifest->entry.type = ResourceManifestEntryTypeFile;
            break;
        case 'D':
            resource_manifest->entry.type = ResourceManifestEntryTypeDirectory;
            break;
        default: /* Skip other entries - version, timestamp, etc */
            continue;
        };

        if(resource_manifest->entry.type == ResourceManifestEntryTypeFile) {
            /* Parse file entry
              F:<hash>:<size>:<name> */

            /* Remove entry type code */
            string_right(resource_manifest->linebuf, 2);

            if(string_search_char(resource_manifest->linebuf, ':') !=
               sizeof(resource_manifest->entry.hash) * 2) {
                /* Invalid hash */
                continue;
            }

            /* Read hash */
            hex_chars_to_uint8(
                string_get_cstr(resource_manifest->linebuf), resource_manifest->entry.hash);

            /* Remove hash */
            string_right(
                resource_manifest->linebuf, sizeof(resource_manifest->entry.hash) * 2 + 1);

            resource_manifest->entry.size = atoi(string_get_cstr(resource_manifest->linebuf));

            /* Remove size */
            size_t offs = string_search_char(resource_manifest->linebuf, ':');
            string_right(resource_manifest->linebuf, offs + 1);

            string_set(resource_manifest->entry.name, resource_manifest->linebuf);
        } else if(resource_manifest->entry.type == ResourceManifestEntryTypeDirectory) {
            /* Parse directory entry
               D:<name> */

            /* Remove entry type code */
            string_right(resource_manifest->linebuf, 2);

            string_set(resource_manifest->entry.name, resource_manifest->linebuf);
        }

        return &resource_manifest->entry;
    } while(true);

    return NULL;
}
