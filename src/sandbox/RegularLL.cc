#include "RegularLL.h"

RegularLL::RegularLL(const Params& p):
    north_(p.get("north")),
    south_(p.get("south")),
    west_(p.get("west")),
    east_(p.get("east")),
    northSouthIncrement_(p.get("northSouthIncrement")),
    westEastIncrement_(p.get("westEastIncrement"))
{
}

void RegularLL::print(std::ostream& s) const
{
    s << "RegularLL[north="  << north_ <<
        s << ",west="  << west_ <<
        s << ",south="  << south_ <<
        s << ",east="  << east_ <<
        s << ",ew="  <<  westEastIncrement_<<
        s << ",ns="  <<  northSouthIncrement_<< "]";
}

static GridSpecMaker<RegularLL> makeRegularLL;
