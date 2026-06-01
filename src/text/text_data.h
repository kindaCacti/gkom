#ifndef TEXT_TEXT_DATA_H
#define TEXT_TEXT_DATA_H

#include <string>
#include <glm/glm.hpp>

struct TextData {
    std::string text;
    float x, y, scale;
    glm::vec3 color;
};

#endif