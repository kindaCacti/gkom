#ifndef SETTINGS_H
#define SETTINGS_H

#include <vector>
#include <string>
#include <optional>
#include <glm/glm.hpp>
#include <yaml-cpp/yaml.h>
#include <iostream>

#include "utils.h"
#include "defines.h"

namespace YAML {
    template<>
    struct convert<glm::vec3> {
        static bool decode(const Node& node, glm::vec3& rhs) {
            if(!node.IsSequence() || node.size() != 3) {
                return false;
            }
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };
}

struct MeshData {
    std::string path;
    std::string name;
    std::optional<glm::vec3> colorOverride;
    Transform transform;
};

struct TextureData {
    std::string path;
    std::string name;
};

struct ShaderData {
    std::string vertexShader;
    std::string fragmentShader;
};

struct Settings {
    std::vector<MeshData> meshes;
    std::vector<TextureData> textures;
    ShaderData gameShader;
    ShaderData textShader;
    ShaderData instancedShader;
    size_t windowWidth = SCR_WIDTH;
    size_t windowHeight = SCR_HEIGHT;
    std::string title = "Coffee Invaders";
    bool is_instanced = IS_INSTANCED;
    bool is_benchmark = BENCHMARK_ON;
};

inline void loadMeshesToSettings(Settings& settings, YAML::Node& yamlNode) {
    for(const auto& mesh : yamlNode) {
        std::string path = mesh["path"].as<std::string>();
        std::string name = mesh["name"].as<std::string>();
        std::optional<glm::vec3> color = std::nullopt;
        glm::vec3 position(0.0f);
        glm::vec3 rotation(0.0f);
        glm::vec3 scale(1.0f);

        if(mesh["color"].IsDefined()) color = mesh["color"].as<glm::vec3>();
        if(mesh["position"].IsDefined()) position = mesh["position"].as<glm::vec3>();
        if(mesh["rotation"].IsDefined()) rotation = mesh["rotation"].as<glm::vec3>();
        if(mesh["scale"].IsDefined()) scale = mesh["scale"].as<glm::vec3>();

        Transform transform;
        transform.setPosition(position);
        transform.setRotation(rotation);
        transform.setScale(scale);

        settings.meshes.push_back(
            MeshData {
                .path = path,
                .name = name,
                .colorOverride = color,
                .transform = transform
            }
        );
    }
}

inline void loadTexturesToSettings(Settings& settings, YAML::Node& yamlNode) {
    for(const auto& texture : yamlNode) {
        std::string path = texture["path"].as<std::string>();
        std::string name = texture["name"].as<std::string>();

        settings.textures.push_back(
            TextureData {
                .path = path,
                .name = name
            }
        );
    }
}

inline void loadShadersToSettings(Settings& settings, YAML::Node& yamlNode) {
    if(yamlNode["game_shader"].IsDefined()) {
        YAML::Node shaderNode = yamlNode["game_shader"];
        settings.gameShader = ShaderData{
            .vertexShader = shaderNode["vertex"].as<std::string>(),
            .fragmentShader = shaderNode["fragment"].as<std::string>()
        };
    }

    if(yamlNode["text_shader"].IsDefined()) {
        YAML::Node shaderNode = yamlNode["text_shader"];
        settings.textShader = ShaderData{
            .vertexShader = shaderNode["vertex"].as<std::string>(),
            .fragmentShader = shaderNode["fragment"].as<std::string>()
        };
    }

    if(yamlNode["instanced_shader"].IsDefined()) {
        YAML::Node shaderNode = yamlNode["instanced_shader"];
        settings.instancedShader = ShaderData{
            .vertexShader = shaderNode["vertex"].as<std::string>(),
            .fragmentShader = shaderNode["fragment"].as<std::string>()
        };
    }
}

inline void loadDefaultsToSettings(Settings& settings, YAML::Node& yamlNode) {
    if(yamlNode["window"].IsDefined()) {
        if(yamlNode["window"]["width"].IsDefined()) settings.windowWidth = yamlNode["window"]["width"].as<size_t>();
        if(yamlNode["window"]["height"].IsDefined()) settings.windowHeight = yamlNode["window"]["height"].as<size_t>();
        if(yamlNode["window"]["title"].IsDefined()) settings.title = yamlNode["window"]["title"].as<std::string>();
    }
    if(yamlNode["is_instanced"].IsDefined()) settings.is_instanced = yamlNode["is_instanced"].as<bool>();
    if(yamlNode["is_benchmark"].IsDefined()) settings.is_benchmark = yamlNode["is_benchmark"].as<bool>();
}


inline Settings loadSettingsFromYaml(std::string path) {
    auto config = YAML::LoadFile(path);

    Settings settings;

    if(!config.IsMap()) return settings;

    if(config["mesh"].IsDefined()){
        YAML::Node meshNode = config["mesh"];
        loadMeshesToSettings(settings, meshNode);
    }

    if(config["texture"].IsDefined()){
        YAML::Node textureNode = config["texture"];
        loadTexturesToSettings(settings, textureNode);
    }

    if(config["shader"].IsDefined()) {
        YAML::Node shaderNode = config["shader"];
        loadShadersToSettings(settings, shaderNode);
    }

    if(config["default_settings"].IsDefined()) {
        YAML::Node defaultSettingsNode = config["default_settings"];
        loadDefaultsToSettings(settings, defaultSettingsNode);
    }

    std::cout<<settings.meshes.size()<<std::endl;

    return settings;
}

#endif