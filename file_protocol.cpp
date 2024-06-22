#include "file_protocol.h"

#include <fstream>
#include <sstream>
#include <iterator>
#if defined(PLATFORM_WEB)
#else
#include <filesystem>
#endif

int save_binary_file(const std::string& path, const std::vector<char>& data) {
    std::ofstream ofs(path, std::ios::out | std::ios::binary);
    if (!ofs) {
        return 1;
    }

    ofs.write(data.data(), data.size());
    if (!ofs) {
        return 1;
    }

    return 0;
}

int save_file(const std::string& path, const std::string& data) {
    std::ofstream ofs(path);
    if (!ofs) {
        return 1;
    }

    ofs.write(data.data(), data.size());
    if (!ofs) {
        return 1;
    }

    return 0;
}

int load_binary_file(const std::string& path, std::vector<char>& result) {
    std::ifstream ifs(path, std::ios::in | std::ios::binary);
    if (!ifs) {
        return 1;
    }

    ifs.seekg(0, std::ios::end);
    std::streampos size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    result.resize(static_cast<size_t>(size));
    ifs.read(result.data(), size);

    if (!ifs) {
        return 1;
    }

    return 0;
}

int load_file(const std::string& path, std::string& result) {
    std::ifstream ifs(path);
    if (!ifs) {
        return 1;
    }

    std::stringstream ss;
    ss << ifs.rdbuf();

    result = ss.str();

    return 0;
}

int load_file_from_dir(const std::string& dir, int file_index_in_dir, std::string& result) {
#if defined(PLATFORM_WEB)
#else
    std::filesystem::directory_iterator dir_iter_cur = std::filesystem::directory_iterator(dir);
    std::filesystem::directory_iterator dir_iter_end;
    while (0 < file_index_in_dir) {
        if (dir_iter_cur == dir_iter_end) {
            return 1;
        }
        ++dir_iter_cur;
        --file_index_in_dir;
    }

    if (dir_iter_cur == dir_iter_end) {
        return 1;
    }

    if (load_file(dir_iter_cur->path().string(), result)) {
        return 1;
    }
#endif

    return 0;
}

int load_binary_file_from_dir(const std::string& dir, int file_index_in_dir, std::vector<char>& result) {
#if defined(PLATFORM_WEB)
#else
    std::filesystem::directory_iterator dir_iter_cur = std::filesystem::directory_iterator(dir);
    std::filesystem::directory_iterator dir_iter_end;
    while (0 < file_index_in_dir) {
        if (dir_iter_cur == dir_iter_end) {
            return 1;
        }
        ++dir_iter_cur;
        --file_index_in_dir;
    }

    if (dir_iter_cur == dir_iter_end) {
        return 1;
    }

    if (load_binary_file(dir_iter_cur->path().string(), result)) {
        return 1;
    }
#endif
    return 0;
}
