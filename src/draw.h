#ifndef DRAW_H
#define DRAW_H

#include <concepts>

#include "shaders/shader_s.h"
#include "globals.h"

template <typename T>
concept Drawable = requires(T var) {
    { var.draw(Shader) } -> void;
};

template <typename T>
concept DrawableInstanced = requires(T var) {
    { var.drawInstanced(Shader) } -> void;
};

template <Drawable T>
inline void draw(T& object, Shader& shader){
    gameStateData.addDrawCall();
    object.draw(shader);
}

template <Drawable T>
inline void drawInstanced(T& object, Shader& shader){
    gameStateData.addDrawCall();
    object.drawInstanced(shader);
}

#endif