#include "asset_manager.h"

#include <iostream>
#include <string>
#include <thread>

#include "raylib.h"
#include "json.hpp"

struct asset_data_json_t : public asset_data_base_t {
    nlohmann::json json;
};

struct asset_data_jpeg_t : public asset_data_base_t {
    Texture2D texture;
};

int main() {
    InitWindow(1600, 1200, "Asset Loader Example");

    asset_manager_t asset_manager("http", "127.0.0.1", 8081);

    asset_manager.add_asset_loader<asset_data_json_t>(
        [](asset_data_json_t* asset_data, const unsigned char* serialized_data, size_t serialized_data_size) -> int {
            try {
                asset_data->json =  nlohmann::json::parse(serialized_data, serialized_data + serialized_data_size);
            } catch (std::exception& e) {
                std::cerr << "CLIENT nlohmann::json::parse failed: '" << e.what() << "'" << std::endl;
                return 1;
            }
            return 0;
        },
        [](asset_data_json_t* asset_data) -> int {
            (void) asset_data;
            return 0;
        }
    );

    asset_manager.add_asset_loader<asset_data_jpeg_t>(
        [](asset_data_jpeg_t* asset_data, const unsigned char* serialized_data, size_t serialized_data_size) -> int {
            Image image = LoadImageFromMemory(".jpeg", serialized_data, serialized_data_size);
            if (!image.data) {
                std::cerr << "CLIENT LoadImageFromMemory failed" << std::endl;
                return 1;
            }

            asset_data->texture = LoadTextureFromImage(image);
            if (asset_data->texture.id <= 0) {
                UnloadImage(image);
                return 1;
            }

            return 0;
        },
        [](asset_data_jpeg_t* asset_data) -> int {
            (void) asset_data;
            return 0;
        }
    );

    Texture2D* texture = 0;
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_I)) {
            asset_manager.add_asset_async<asset_data_json_t>(
                "info", 1,
                [](asset_data_json_t* json) {
                    std::cout << "CLIENT successfully fetched asset: " << std::endl;
                    std::cout << json->json.dump(4) << std::endl;
                },
                []() {
                    std::cerr << "CLIENT could not fetch asset" << std::endl;
                }
            );
        }
        if (IsKeyPressed(KEY_L)) {
            std::string jpeg_path = "asset/assets/aatrox.jpeg";
            asset_manager.add_asset_async<asset_data_jpeg_t>(
                jpeg_path, 0,
                [&texture, jpeg_path](asset_data_jpeg_t* jpeg) {
                    texture = &jpeg->texture;
                    std::cout << "CLIENT successfully fetched jpeg: '" << jpeg_path << "'" << std::endl;
                },
                [jpeg_path]() {
                    std::cerr << "CLIENT could not fetch jpeg: '" << jpeg_path << "'" << std::endl;
                }
            );
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (texture) {
            DrawTexture(*texture, 0, 0, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
