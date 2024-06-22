#ifndef FILE_PROTOCOL_H
# define FILE_PROTOCOL_H

# include <string>
# include <vector>

int save_binary_file(const std::string& path, const std::vector<char>& data);
int save_file(const std::string& path, const std::string& data);
int load_binary_file(const std::string& path, std::vector<char>& result);
int load_file(const std::string& path, std::string& result);

int load_file_from_dir(const std::string& dir, int file_index_in_dir, std::string& result);
int load_binary_file_from_dir(const std::string& dir, int file_index_in_dir, std::vector<char>& result);

#endif // FILE_PROTOCOL_H
