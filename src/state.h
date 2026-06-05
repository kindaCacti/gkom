#ifndef STATE_H
#define STATE_H

#include <stddef.h>
#include <vector>
#include <GLFW/glfw3.h>
#include <bitset>

struct State {
    size_t drawCallsMade;
    std::bitset<GLFW_KEY_LAST + 1> keyDown;

    State(): drawCallsMade(0) {
        keyDown.reset();        
    }
    
    void holdDownKey(size_t id){
        if(id > GLFW_KEY_LAST) return;
        keyDown.set(id);
    }

    void releaseKey(size_t id) {
        if(id > GLFW_KEY_LAST) return;
        keyDown.reset(id);
    }

    bool wasPressedBefore(size_t id) {
        if(id > GLFW_KEY_LAST) return false;
        return keyDown.test(id);
    }

    void addDrawCall(){ ++drawCallsMade; }

    void newFrame() {
        drawCallsMade = 0;
    }
};

#endif