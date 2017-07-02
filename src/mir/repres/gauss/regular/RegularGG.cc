/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/repres/gauss/regular/RegularGG.h"

#include <iostream>
#include "mir/action/misc/AreaCropper.h"
#include "mir/util/Domain.h"

#include "atlas/library/config.h"

#ifdef ATLAS_HAVE_TRANS
#include "transi/trans.h"
#endif


namespace mir {
namespace repres {
namespace regular {


RegularGG::RegularGG(const param::MIRParametrisation &parametrisation):
    Regular(parametrisation) {
}


RegularGG::RegularGG(size_t N):
    Regular(N) {
}


RegularGG::RegularGG(size_t N, const util::BoundingBox &bbox):
    Regular(N, bbox) {
}


RegularGG::~RegularGG() {
}


void RegularGG::print(std::ostream &out) const {
    out << "RegularGG[N" << N_ << ",bbox=" << bbox_ << "]";
}


void RegularGG::makeName(std::ostream& out) const {
    Regular::makeName(out);
}

bool RegularGG::sameAs(const Representation& other) const {
    const RegularGG* o = dynamic_cast<const RegularGG*>(&other);
    return o && Regular::sameAs(other);
}


void RegularGG::initTrans(Trans_t &trans) const {
#ifdef ATLAS_HAVE_TRANS

    const std::vector<int> pl(Nj_, int(Ni_));
    ASSERT(trans_set_resol(&trans, int(Nj_), pl.data()) == 0);

#else
    NOTIMP;
#endif
}


Iterator* RegularGG::iterator() const {

    class RegularGGIterator : protected RegularIterator, public Iterator {
        void print(std::ostream& out) const {
            out << "RegularGGIterator[";
            Iterator::print(out);
            out << ",";
            RegularIterator::print(out);
            out << "]";
        }
        bool next(Latitude& lat, Longitude& lon) {
            return RegularIterator::next(lat, lon);
        }
    public:
        RegularGGIterator(const std::vector<double>& latitudes, size_t N, size_t Ni, size_t Nj, const util::Domain& dom) :
            RegularIterator(latitudes, N, Ni, Nj, dom) {
        }
    };

    return new RegularGGIterator(latitudes(), N_, Ni_, Nj_, domain());
}


const Gridded *RegularGG::cropped(const util::BoundingBox &bbox) const {
    return new RegularGG(N_, bbox);
}

size_t RegularGG::numberOfPoints() const {
    if (isGlobal()) {
        return Ni_ * Nj_;
    }
    else {
        size_t total = 0;
        eckit::ScopedPtr<repres::Iterator> iter(iterator());
        while (iter->next()) {
            total++;
        }
        return total;
    }
}

namespace {
static RepresentationBuilder<RegularGG> reducedGG("regular_gg"); // Name is what is returned by grib_api
}


}  // namespace regular
}  // namespace repres
}  // namespace mir

