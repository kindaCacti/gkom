#ifndef STATE_H
#define STATE_H

#include <stddef.h>

struct State {
    size_t drawCallsMade;

    State(): drawCallsMade(0) {

    }

    void addDrawCall(){ ++drawCallsMade; }

    void newFrame() {
        drawCallsMade = 0;
    }
};

#endif