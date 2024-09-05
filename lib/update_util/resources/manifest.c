#include "manifest.h"

#include <toolbox/stream/buffered_file_stream.h>
#include <toolbox/strint.h>
#include <toolbox/hex.h>

struct ResourceManifestReader {
    Storage* storage;
    Stream* stream;
    FuriString* linebuf;
    ResourceManifestEntry entry;
};

ResourceManifestReader* resource_manifest_reader_alloc(Storage* storage) {
    ResourceManifestReader* resource_manifest =
        (ResourceManifestReader*)malloc(sizeof(ResourceManifestReader));
    resource_manifest->storage = storage;
    resource_manifest->stream = buffered_file_stream_alloc(resource_manifest->storage);
    memset(&resource_manifest->entry, 0, sizeof(ResourceManifestEntry));
    resource_manifest->entry.name = furi_string_alloc();
    resource_manifest->linebuf = furi_string_alloc();
    return resource_manifest;
}

void resource_manifest_reader_free(ResourceManifestReader* resource_manifest) {
    furi_assert(resource_manifest);

    furi_string_free(resource_manifest->linebuf);
    furi_string_free(resource_manifest->entry.name);
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

    furi_string_reset(resource_manifest->entry.name);
    resource_manifest->entry.type = ResourceManifestEntryTypeUnknown;
    resource_manifest->entry.size = 0;
    memset(resource_manifest->entry.hash, 0, sizeof(resource_manifest->entry.hash));

    do {
        if(!stream_read_line(resource_manifest->stream, resource_manifest->linebuf)) {
            return NULL;
        }

        /* Trim end of line */
        furi_string_trim(resource_manifest->linebuf);

        char type_code = furi_string_get_char(resource_manifest->linebuf, 0);
        switch(type_code) {
        case 'V':
            resource_manifest->entry.type = ResourceManifestEntryTypeVersion;
            break;
        case 'T':
            resource_manifest->entry.type = ResourceManifestEntryTypeTimestamp;
            break;
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
            furi_string_right(resource_manifest->linebuf, 2);

            if(furi_string_search_char(resource_manifest->linebuf, ':') !=
               sizeof(resource_manifest->entry.hash) * 2) {
                /* Invalid hash */
                continue;
            }

            /* Read hash */
            hex_chars_to_uint8(
                furi_string_get_cstr(resource_manifest->linebuf), resource_manifest->entry.hash);

            /* Remove hash */
            furi_string_right(
                resource_manifest->linebuf, sizeof(resource_manifest->entry.hash) * 2 + 1);

            if(strint_to_uint32(
                   furi_string_get_cstr(resource_manifest->linebuf),
                   NULL,
                   &resource_manifest->entry.size,
                   10) != StrintParseNoError)
                break;

            /* Remove size */
            size_t offs = furi_string_search_char(resource_manifest->linebuf, ':');
            furi_string_right(resource_manifest->linebuf, offs + 1);

            furi_string_set(resource_manifest->entry.name, resource_manifest->linebuf);
        } else { //-V547
            /* Everything else is plain key value. Parse version, timestamp or directory entry
               <Type>:<Value> */

            /* Remove entry type code */
            furi_string_right(resource_manifest->linebuf, 2);

            furi_string_set(resource_manifest->entry.name, resource_manifest->linebuf);
        }

        return &resource_manifest->entry;
    } while(true);

    return NULL;
}

ResourceManifestEntry*
    resource_manifest_reader_previous(ResourceManifestReader* resource_manifest) {
    furi_assert(resource_manifest);

    // Snapshot position for rollback
    const size_t previous_position = stream_tell(resource_manifest->stream);

    // We need to jump 2 lines back
    size_t jumps = 2;
    // Special case: end of the file.
    const bool was_eof = stream_eof(resource_manifest->stream);
    if(was_eof) {
        jumps = 1;
    }
    while(jumps) {
        if(!stream_seek_to_char(resource_manifest->stream, '\n', StreamDirectionBackward)) {
            break;
        }
        if(stream_tell(resource_manifest->stream) < (previous_position - 1)) {
            jumps--;
        }
    }

    // Special case: first line. Force seek to zero
    if(jumps == 1) {
        jumps = 0;
        stream_seek(resource_manifest->stream, 0, StreamOffsetFromStart);
    }

    if(jumps == 0) {
        ResourceManifestEntry* entry = resource_manifest_reader_next(resource_manifest);
        // Special case: was end of the file, prevent loop
        if(was_eof) {
            stream_seek(resource_manifest->stream, -1, StreamOffsetFromCurrent);
        }
        return entry;
    } else {
        stream_seek(resource_manifest->stream, previous_position, StreamOffsetFromStart);
        return NULL;
    }
}

bool resource_manifest_rewind(ResourceManifestReader* resource_manifest) {
    furi_assert(resource_manifest);

    return stream_seek(resource_manifest->stream, 0, StreamOffsetFromStart);
}
