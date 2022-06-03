#ifndef MOFACE_STATE_H
#define MOFACE_STATE_H

#include <cstdlib>

namespace moface {
    enum MoFaceState {
        eInit,
        eStart,
        eReady,
        eDragTracking,
        eNop
    };
}

#endif
