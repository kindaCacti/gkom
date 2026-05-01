#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

#include <random>
#include <utility>
#include <vector>

struct Texture {
    unsigned int id = 0;
    int width = 0;
    int height = 0;

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    Texture(Texture &&other) noexcept
        : id(other.id), width(other.width), height(other.height) {
        other.id = 0;
        other.width = 0;
        other.height = 0;
    }

    Texture &operator=(Texture &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        if (id != 0) {
            glDeleteTextures(1, &id);
        }
        id = other.id;
        width = other.width;
        height = other.height;
        other.id = 0;
        other.width = 0;
        other.height = 0;
        return *this;
    }

    Texture() {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    static Texture newNoise2D(int width, int height,
                              unsigned int seed = 1337u) {
        std::vector<unsigned char> data;
        data.resize(static_cast<std::size_t>(width) *
                    static_cast<std::size_t>(height));

        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> dist(0, 255);
        for (auto &b : data) {
            b = static_cast<unsigned char>(dist(rng));
        }

        Texture tex;
        tex.width = width;
        tex.height = height;
        tex.bind(0);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED,
                     GL_UNSIGNED_BYTE, data.data());

        // Make sampling as vec3 work nicely if you ever use it.
        GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

        tex.unbind();

        return tex;
    }

    void bind(unsigned int unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, id);
    }

    void unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

    ~Texture() {
        if (id != 0) {
            glDeleteTextures(1, &id);
        }
    }

  private:
};

#endif