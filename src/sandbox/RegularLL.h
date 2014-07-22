#ifndef RegularLL_H
#define RegularLL_H

#include "GridSpec.h"

class RegularLL : public GridSpec {
private:
    double north_;
    double west_;
    double east_;
    double south_;
    double northSouthIncrement_;
    double westEastIncrement_;

    void print(std::ostream& s) const ;

public:
    static const char* name() { return "regular_ll"; }

    RegularLL(const Params& p);
};

#endif
