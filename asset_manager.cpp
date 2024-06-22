#include "asset_manager.h"

asset_manager_t::asset_manager_t(const std::string& server_scheme, const std::string& server_host_name, size_t server_port): 
    m_server_scheme(server_scheme),
    m_server_host_name(server_host_name),
    m_server_port(server_port) {
}

asset_manager_t::~asset_manager_t() {
    for (const auto& type_to_loader_it : m_asset_type_to_asset_loader) {
        delete type_to_loader_it.second;
    }
    m_asset_type_to_asset_loader.clear();
}


void asset_manager_t::init(const std::string& server_scheme, const std::string& server_host_name, size_t server_port) {
    m_server_scheme = server_scheme;
    m_server_host_name = server_host_name;
    m_server_port = server_port;
}

