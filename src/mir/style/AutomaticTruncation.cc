/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date May 2017


#include "mir/style/AutomaticTruncation.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/action/transform/mapping/Mapping.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"


namespace mir {
namespace style {


AutomaticTruncation::AutomaticTruncation(const param::MIRParametrisation& parametrisation) :
    parametrisation_(parametrisation),
    mapping_("linear"),
    truncation_(0) {

    long input = 0L;
    ASSERT(parametrisation_.get("field.truncation", input));
    parametrisation_.get("spectral-mapping", mapping_);

    // TODO: this is temporary, no support yet for unstuctured grids
    if (parametrisation_.has("griddef")) {
        truncation_ = 63L;
        return;
    }

    // get truncation from points-per-latitude, limited to input
    long N = getGaussianNumber(parametrisation_);
    ASSERT(N > 0);

    using namespace action::transform::mapping;
    eckit::ScopedPtr<Mapping> map(MappingFactory::build(mapping_));
    ASSERT(map);

    truncation_ = map->getTruncationFromGaussianNumber(N);
    if (truncation_ > input) {
        eckit::Log::warning() << "AutomaticTruncation: truncation " << truncation_ << " limited by input truncation " << input << std::endl;
        truncation_ = input;
    }
}


long AutomaticTruncation::getGaussianNumber(const param::MIRParametrisation& parametrisation) {

    std::vector<double> grid;
    if (parametrisation.get("user.grid", grid)) {
        ASSERT(grid.size() == 2);

        // get Nj: only South-North increment (grid[1]) is used
        util::BoundingBox bbox;
        ASSERT(bbox.contains( 90, 0));
        ASSERT(bbox.contains(-90, 0));
        long Nj = long(bbox.computeNj(util::Increments(grid[0], grid[1])));

        // get Ni(Nj): -1 corrects periodicity
        long N = (Nj - 1) / 2L;
        return N;
    }

    long N = 0;
    if (parametrisation.get("user.reduced", N) ||
        parametrisation.get("user.regular", N) ||
        parametrisation.get("user.octahedral", N)) {
        return N;
    }

    std::string gridname;
    if (parametrisation.get("user.gridname", gridname)) {
        N = long(namedgrids::NamedGrid::lookup(gridname).gaussianNumber());
        return N;
    }

    std::ostringstream os;
    os << "AutomaticTruncation: cannot calculate Gaussian number (N) from target grid";
    throw eckit::SeriousBug(os.str());
}


bool AutomaticTruncation::get(const std::string &name, long &value) const {
    ASSERT(name == "truncation");

    value = truncation_;
    return true;
}


bool AutomaticTruncation::get(const std::string& name, size_t& value) const {
    long T;
    if (get(name, T)) {
        value = size_t(T);
        return true;
    }
    return false;
}


void AutomaticTruncation::print(std::ostream& out) const {
    out << "AutomaticTruncation["
            "mapping=" << mapping_
        << ",truncation=" << truncation_
        << "]";
}


}  // namespace style
}  // namespace mir
