#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#include "file_protocol.h"
#include "json.hpp"

#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <iterator>
#include <algorithm>

static struct {
    nlohmann::json assets;
    httplib::Server server;
    int port;
} _;

static int init();

static nlohmann::json json_recurse_dirs(const std::string& dir);
static int json_search(const nlohmann::json& json, const std::string& str);

static void send_bytes(const std::string& path, const std::vector<char>& data, httplib::Response &res);

static int init() {
    _.assets = json_recurse_dirs("assets");
    _.port = 8081;
    _.server.set_mount_point("/", "public");

    _.server.Get("/", [](const httplib::Request &req, httplib::Response &res) {
        (void) req;

        const std::string file_path = "public/local.html";
        std::string content;
        if (!load_file(file_path, content)) {
            std::cout << "GET /" << std::endl;
            res.set_content(content.c_str(), "text/html");
        } else {
            std::cerr << "GET load_file failed for " << file_path << std::endl;
        }
    });

    _.server.Get("/info", [](const httplib::Request &req, httplib::Response &res) {
        std::string str = _.assets.dump();
        std::vector<char> content(str.begin(), str.end());

        send_bytes(req.path, content, res);
    });

    _.server.Get(R"(/asset/(.*))", [](const httplib::Request &req, httplib::Response &res) {
        if (req.matches.size() < 2) {
            res.status = httplib::StatusCode::NotFound_404;
            std::cerr << "GET asset '" << req.target << "' was not found" << std::endl;
            return;
        }

        const std::string asset_path = req.matches[1];
        if (!json_search(_.assets, asset_path)) {
            std::vector<char> content;
            if (load_binary_file(asset_path, content)) {
                res.status = httplib::StatusCode::InternalServerError_500;
                std::cerr << "GET could not load file for request: '" << asset_path << "'" << std::endl;
                return ;
            }
            send_bytes(req.path, content, res);
        } else {
            res.status = httplib::StatusCode::NotFound_404;
            std::cerr << "GET asset '" << asset_path << "' was not found" << std::endl;
        }
    });

    _.server.Get(R"(.*)", [](const httplib::Request &req, httplib::Response &res) {
        res.status = httplib::StatusCode::NotFound_404;
        assert(0 < req.matches.size());
        std::cerr << "GET no route on '" << req.matches[0] << "'" << std::endl;
    });

    return 0;
}

static nlohmann::json json_recurse_dirs(const std::string& dir) {
    nlohmann::json result;

    std::filesystem::directory_iterator dir_iter_cur(dir);
    std::filesystem::directory_iterator dir_iter_end;
    while (dir_iter_cur != dir_iter_end) {
        if (dir_iter_cur->is_directory()) {
            result.push_back({
                { dir_iter_cur->path().string(), json_recurse_dirs(dir_iter_cur->path().string()) }
            });
        } else if (dir_iter_cur->is_regular_file()) {
            std::string file_path = dir_iter_cur->path().string();
            std::replace(file_path.begin(), file_path.end(), '\\', '/');
            result.push_back(file_path);
        }
        ++dir_iter_cur;
    }

    return result;
}

static int json_search(const nlohmann::json& json, const std::string& str) {
    if (json.is_primitive()) {
        if (json.is_string()) {
            const std::string file_path = json.get<std::string>();
            if (file_path == str) {
                return 0;
            } else {
                return 1;
            }
        } else {
            return 1;
        }
    }


    for (const nlohmann::json& json_child : json) {
        if (!json_search(json_child, str)) {
            return 0;
        }
    }
    return 1;
}

static void send_bytes(const std::string& path, const std::vector<char>& data, httplib::Response &res) {
    auto _data = new std::vector<char>(data);
    res.set_content_provider(
        _data->size(),
        "text/plain",
        [_data](size_t offset, size_t size, httplib::DataSink& sink) {
            const size_t DATA_CHUNK_SIZE = 1024;
            sink.write(_data->data() + offset, size < DATA_CHUNK_SIZE ? size : DATA_CHUNK_SIZE);
            return true;
        },
        [_data, path](bool success) {
            if (success) {
                std::cout << "GET '" << path << ", data size sent: " << _data->size() << std::endl;
            } else {
                std::cerr << "GET failed '" << path << "'" << std::endl;
            }
            delete _data;
        }
    );
}

int main() {
    if (init()) {
        return 1;
    }

    printf("Server listening on port %d\n", _.port);
    _.server.listen("0.0.0.0", _.port);

    return 0;
}
