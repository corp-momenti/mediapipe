#ifndef MOFACE_STATE_H
#define MOFACE_STATE_H

#include <cstdlib>

namespace moface {
    enum MoFaceState {
        eInit,
        eStart,
        eReady,
        eDragTracking,
        eDragAnalyzing,
        eNop
    };

    std::string state_to_string(MoFaceState const& in_code) {
        std::string ret_string;
        switch (in_code) {
            case eInit:
                ret_string = "init";
                break;
            case eStart:
                ret_string = "start";
                break;
            case eReady:
                ret_string = "ready";
                break;
            case eDragTracking:
                ret_string = "drag-tracking";
                break;
            case eDragAnalyzing:
                ret_string = "drag-analyzing";
                break;
            case eNop:
                ret_string = "nop";
                break;
            default:
                ret_string = "not-available";
        }
        return ret_string;
    }
}

#endif
