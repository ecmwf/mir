#include "ReducedGG.h"

ReducedGG::ReducedGG(const Params& p):
    north_(p.get("north")),
    south_(p.get("south")),
    west_(p.get("west")),
    east_(p.get("east")),
    gaussian_(p.get("gaussian"))
{
}


void ReducedGG::print(std::ostream& s) const
{
    s << "ReducedGGL[north="  << north_ <<
        s << ",west="  << west_ <<
        s << ",south="  << south_ <<
        s << ",east="  << east_ <<
        s << ",N="  <<  gaussian_ << "]";
}

static GridSpecMaker<ReducedGG> makeReducedGG;
