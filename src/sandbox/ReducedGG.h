#ifndef GridSpec_H
#define GridSpec_H

#include "GridSpec.h"

//==========================================================================

class ReducedGG : public GridSpec {
private:
    double north_;
    double west_;
    double east_;
    double south_;
    int gaussian_;

public:
    static const char* name() { return "reduced_gg"; }

    ReducedGG(const Params& p);
};

#endif
