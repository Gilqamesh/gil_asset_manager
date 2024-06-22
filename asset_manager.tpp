#include "asset_manager.h"

#include "web.h"

#include <iostream>
#include <cassert>
#include <thread>

template <typename asset_data_type_t>
asset_loader_t<asset_data_type_t>::~asset_loader_t() {
    for (const auto& path_to_data_it : asset_path_to_asset_data) {
        asset_data_type_t* asset = path_to_data_it.second;
        if (unload(asset)) {
            std::cerr << "~asset_loader_t couldn't unload asset '" << path_to_data_it.first << "', removing it from asset loader" << std::endl;
        }
        delete asset;

        int all_loading_assets_aborted = 0;
        do {
            mutex_asset_path_loading.lock();
            if (asset_path_loading.empty()) {
                all_loading_assets_aborted = 1;
            } else {
                should_abort = 1;
            }
            mutex_asset_path_loading.unlock();
            if (!all_loading_assets_aborted) {
                std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(200.0));
            }
        } while(!all_loading_assets_aborted);
    }
}

template <typename asset_data_type_t>
int asset_manager_t::add_asset_loader(
    const std::function<int(asset_data_type_t* asset_data, const unsigned char* serialized_data, size_t serialized_data_size)>& load,
    const std::function<int(asset_data_type_t* asset_data)>& unload
) {
    static_assert(std::is_base_of<asset_data_base_t, asset_data_type_t>::value, "type must derive from asset_data_base_t");

    const char* type_name = typeid(asset_data_type_t).name();
    auto type_to_loader_it = m_asset_type_to_asset_loader.find(type_name);
    if (type_to_loader_it == m_asset_type_to_asset_loader.end()) {
        asset_loader_t<asset_data_type_t>* asset_loader = new asset_loader_t<asset_data_type_t>();
        m_asset_type_to_asset_loader[type_name] = asset_loader;
        asset_loader->should_abort = 0;
        asset_loader->load = load;
        asset_loader->unload = unload;
        return 0;
    } else {
        std::cerr << "asset_manager__add_asset_loader asset_loader for type '" << type_name << "' already exists" << std::endl;
        return 1;
    }
}

template <typename asset_data_type_t>
void asset_manager_t::add_asset_async(
    const std::string& asset_path, int re_add,
    const std::function<void(asset_data_type_t* asset_data)>& on_success,
    const std::function<void()>& on_failure
) {
    static_assert(std::is_base_of<asset_data_base_t, asset_data_type_t>::value, "type must derive from asset_data_base_t");

    const char* type_name = typeid(asset_data_type_t).name();
    auto type_to_loader_it = m_asset_type_to_asset_loader.find(type_name);
    if (type_to_loader_it != m_asset_type_to_asset_loader.end()) {
        asset_loader_t<asset_data_type_t>* asset_loader = static_cast<asset_loader_t<asset_data_type_t>*>(type_to_loader_it->second);

        asset_loader->mutex_asset_path_loading.lock();
        int is_loading = asset_loader->asset_path_loading.find(asset_path) != asset_loader->asset_path_loading.end();
        asset_loader->mutex_asset_path_loading.unlock();

        if (is_loading) {
            if (re_add) {
                std::cerr << "asset_manager__add_asset_async asset '" << asset_path << "' is still loading, cannot re-add just yet.." << std::endl;
            } else {
                std::cerr << "asset_manager__add_asset_async asset '" << asset_path << "' is still loading.." << std::endl;
            }
            return ;
        }

        auto path_to_data_it = asset_loader->asset_path_to_asset_data.find(asset_path);
        if (path_to_data_it == asset_loader->asset_path_to_asset_data.end()) {
            asset_data_type_t* asset_data_type = new asset_data_type_t();
            asset_data_type->asset_path = asset_path;

            asset_loader->mutex_asset_path_loading.lock();
            asset_loader->asset_path_loading.insert(asset_path);
            asset_loader->mutex_asset_path_loading.unlock();

            async_fetch__create_async(
                m_server_scheme, m_server_host_name, m_server_port, asset_path,
                [asset_data_type, asset_loader, on_success, on_failure](const unsigned char* serialized_data, size_t serialized_data_size) {
                    asset_loader->mutex_asset_path_loading.lock();
                    int should_abort = asset_loader->should_abort;
                    asset_loader->asset_path_loading.erase(asset_data_type->asset_path);
                    asset_loader->mutex_asset_path_loading.unlock();
                    if (should_abort) {
                        delete asset_data_type;
                        return ;
                    }

                    if (asset_loader->load(asset_data_type, serialized_data, serialized_data_size)) {
                        delete asset_data_type;
                        on_failure();
                    } else {
                        asset_loader->asset_path_to_asset_data[asset_data_type->asset_path] = asset_data_type;
                        on_success(asset_data_type);
                    }
                },
                [asset_data_type, asset_loader, on_failure]() {
                    asset_loader->mutex_asset_path_loading.lock();
                    asset_loader->asset_path_loading.erase(asset_data_type->asset_path);
                    asset_loader->mutex_asset_path_loading.unlock();

                    delete asset_data_type;

                    on_failure();
                }
            );
        } else {
            asset_data_type_t* asset_data = path_to_data_it->second;
            if (re_add) {
                // todo: implement implace
                if (remove_asset<asset_data_type_t>(asset_data)) {
                    on_failure();
                    return ;
                }
                add_asset_async<asset_data_type_t>(asset_path, 0, on_success, on_failure);
            } else {
                std::cerr << "asset_manager__add_asset_async asset '" << asset_path << "' already exists" << std::endl;
                on_failure();
            }
        }
    } else {
        std::cerr << "asset_manager__add_asset_async asset loader does not exist for type '" << type_name << "'" << std::endl;
        on_failure();
    }
}

template <typename asset_data_type_t>
int asset_manager_t::remove_asset(asset_data_type_t* asset_to_remove) {
    static_assert(std::is_base_of<asset_data_base_t, asset_data_type_t>::value, "type must derive from asset_data_base_t");

    const char* type_name = typeid(asset_data_type_t).name();
    auto type_to_loader_it = m_asset_type_to_asset_loader.find(type_name);
    if (type_to_loader_it != m_asset_type_to_asset_loader.end()) {
        asset_loader_t<asset_data_type_t>* asset_loader = static_cast<asset_loader_t<asset_data_type_t>*>(type_to_loader_it->second);
        
        asset_loader->mutex_asset_path_loading.lock();
        int is_loading = asset_loader->asset_path_loading.find(asset_to_remove->asset_path) != asset_loader->asset_path_loading.end();
        asset_loader->mutex_asset_path_loading.unlock();
        assert(!is_loading && "asset should not be loading at this point..");

        auto path_to_data_it = asset_loader->asset_path_to_asset_data.find(asset_to_remove->asset_path);
        if (path_to_data_it != asset_loader->asset_path_to_asset_data.end()) {
            asset_data_type_t* found_asset = path_to_data_it->second;
            assert(found_asset == asset_to_remove);
            if (!asset_loader->unload(found_asset)) {
                asset_loader->asset_path_to_asset_data.erase(path_to_data_it);
                delete found_asset;
                return 0;
            } else {
                std::cerr << "asset_manager__remove_asset asset '" << asset_to_remove->asset_path << "' could not be removed" << std::endl;
                return 1;
            }
        } else {
            assert(0 && "asset should exist");
            return 1;
        }
    } else {
        assert(0 && "assert loader should exist for asset");
        return 1;
    }
}
