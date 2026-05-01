#ifndef TEXTURE_FACTORY_H
#define TEXTURE_FACTORY_H

#include "texture.h"
#include <map>
#include <string>
#include <memory>
#include <stdexcept>

class TextureFactory {
    std::map<std::string, std::shared_ptr<Texture>> _textureCache;

  public:
    void registerTexture(const std::shared_ptr<Texture> &texture,
                         const std::string &name) {
        _textureCache[name] = texture;
    }

    std::weak_ptr<Texture> createTexture(const std::string &name) {
        auto it = _textureCache.find(name);
        if (it != _textureCache.end()) {
            return it->second;
        } else {
            throw std::runtime_error("Texture not found: " + name);
        }
    }
};

#endif