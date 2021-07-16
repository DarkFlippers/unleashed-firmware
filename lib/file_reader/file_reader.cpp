#include <file_reader.h>

std::string FileReader::getline(File* file) {
    std::string str;
    size_t newline_index = 0;
    bool found_eol = false;
    bool max_length_exceeded = false;

    while(1) {
        if(file_buf_cnt > 0) {
            size_t end_index = 0;
            char* endline_ptr = (char*)memchr(file_buf, '\n', file_buf_cnt);
            newline_index = endline_ptr - file_buf;

            if(endline_ptr == 0) {
                end_index = file_buf_cnt;
            } else if(newline_index < file_buf_cnt) {
                end_index = newline_index + 1;
                found_eol = true;
            } else {
                furi_assert(0);
            }

            if (max_line_length && (str.size() + end_index > max_line_length))
                max_length_exceeded = true;

            if (!max_length_exceeded)
                str.append(file_buf, end_index);

            memmove(file_buf, &file_buf[end_index], file_buf_cnt - end_index);
            file_buf_cnt = file_buf_cnt - end_index;
            if(found_eol) break;
        }

        file_buf_cnt +=
            fs_api->file.read(file, &file_buf[file_buf_cnt], sizeof(file_buf) - file_buf_cnt);
        if(file_buf_cnt == 0) {
            break; // end of reading
        }
    }

    if (max_length_exceeded)
        str.clear();

    return str;
}


