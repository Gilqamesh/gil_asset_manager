#ifndef ASSET_MANAGER_H
# define ASSET_MANAGER_H

# include <unordered_map>
# include <unordered_set>
# include <string>
# include <functional>
# include <mutex>

struct asset_data_base_t {
    std::string asset_path;
};

struct asset_loader_base_t {
    virtual ~asset_loader_base_t() = default;

    std::mutex mutex_asset_path_loading;
    int should_abort;
    std::unordered_set<std::string> asset_path_loading;
};

template <typename asset_data_type_t>
struct asset_loader_t : asset_loader_base_t {
    ~asset_loader_t();
    std::function<int(asset_data_type_t* asset_data, const unsigned char* serialized_data, size_t serialized_data_size)> load;
    std::function<int(asset_data_type_t* asset_data)> unload;
    std::unordered_map<std::string, asset_data_type_t*> asset_path_to_asset_data;
};

struct asset_manager_t {
    asset_manager_t(const std::string& server_scheme, const std::string& server_host_name, size_t server_port);
    ~asset_manager_t();

    std::string m_server_scheme;
    std::string m_server_host_name;
    size_t      m_server_port;
    std::unordered_map<const char*, asset_loader_base_t*> m_asset_type_to_asset_loader;

    template <typename asset_data_type_t>
    int add_asset_loader(
        const std::function<int(asset_data_type_t* asset_data, const unsigned char* serialized_data, size_t serialized_data_size)>& load,
        const std::function<int(asset_data_type_t* asset_data)>& unload
    );

    template <typename asset_data_type_t>
    void add_asset_async(
        const std::string& asset_path, int re_add,
        const std::function<void(asset_data_type_t* asset_data)>& on_success,
        const std::function<void()>& on_failure
    );

    template <typename asset_data_type_t>
    int remove_asset(asset_data_type_t* asset);
};

# include "asset_manager.tpp"

#endif // ASSET_MANAGER_H
