/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/repres/gauss/reduced/Reduced.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <numeric>
#include <set>
#include <utility>

#include "eckit/log/JSON.h"
#include "eckit/types/Fraction.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/GaussianIterator.h"
#include "mir/util/Atlas.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/Earth.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/GridBox.h"


namespace mir::repres::gauss::reduced {


template <typename T>
std::vector<long> pl_convert(const T& nx) {
    ASSERT(!nx.empty());
    std::vector<long> pl(nx.size());
    std::transform(nx.begin(), nx.end(), pl.begin(), [](typename T::value_type p) { return static_cast<long>(p); });
    return pl;
}


template <>
std::vector<long> pl_convert(const std::vector<long>& nx) {
    ASSERT(!nx.empty());
    return nx;
}


Reduced::Reduced(const param::MIRParametrisation& parametrisation) : Gaussian(parametrisation), k_(0), Nj_(N_ * 2) {

    // adjust latitudes, longitudes and re-set bounding box
    auto n = bbox_.north();
    auto s = bbox_.south();
    correctSouthNorth(s, n);

    std::vector<long> pl;
    ASSERT(parametrisation.get("pl", pl));

    // if pl isn't global (from file!) insert leading/trailing 0's
    const auto& lats = latitudes();
    if (n < lats.front() || s > lats.back()) {
        size_t k  = 0;
        size_t nj = 0;
        for (auto lat : lats) {
            if (n < lat) {
                ++k;
            }
            else if (s <= lat) {
                ASSERT(pl[nj] >= 2);
                ++nj;
            }
            else {
                break;
            }
        }
        ASSERT(k + nj <= N_ * 2);

        if (k > 0) {
            pl.reserve(N_ * 2);
            pl.insert(pl.begin(), k, 0);
        }
        pl.resize(N_ * 2, 0);
    }

    setNj(pl, s, n);

    auto w = bbox_.west();
    auto e = bbox_.east();
    correctWestEast(w, e);

    bbox_ = util::BoundingBox(n, w, s, e);
}


Reduced::Reduced(size_t N, const std::vector<long>& pl, const util::BoundingBox& bbox, double angularPrecision) :
    Gaussian(N, bbox, angularPrecision), k_(0), Nj_(N_ * 2) {
    setNj(pl, bbox.south(), bbox.north());
}


Reduced::Reduced(size_t N, const util::BoundingBox& bbox, double angularPrecision) :
    Gaussian(N, bbox, angularPrecision), k_(0), Nj_(N * 2) {
    // derived classes must set k_, Nj_ using this constructor
}


Reduced::~Reduced() = default;


void Reduced::correctWestEast(double& w, double& e) const {
    using eckit::Fraction;
    ASSERT(w <= e);

    const Fraction smallestIncrement = getSmallestIncrement();
    const Fraction globe(360, 1);

    ASSERT(smallestIncrement > 0);

    if (angleApproximatelyEqual(0, w) &&
        (angleApproximatelyEqual(globe - smallestIncrement, e - w) || globe - smallestIncrement < e - w ||
         (e != w && PointLonLat::normalise_angle_to_minimum(e, w) == w))) {

        w = 0;
        e = globe - smallestIncrement;
    }
    else {

        const Fraction west(w);
        const Fraction east(e);
        Fraction W          = west;
        Fraction E          = east;

        bool first = true;
        std::set<long> NiTried;

        const auto& pl = pls();
        for (size_t j = k_; j < k_ + Nj_; ++j) {

            // crop longitude-wise, track distinct attempts
            const long Ni(pl[j]);
            ASSERT(Ni >= 2);
            if (NiTried.insert(Ni).second) {

                Fraction inc = globe / Ni;

                Fraction::value_type Nw = (west / inc).integralPart();
                if (Nw * inc < west) {
                    Nw += 1;
                }

                Fraction::value_type Ne = (east / inc).integralPart();
                if (Ne * inc > east || Nw + Ne == Ni) {
                    Ne -= 1;
                }

                if (Nw <= Ne) {
                    ASSERT(w <= Fraction(Nw * inc));
                    ASSERT(Fraction(Ne * inc) <= e);

                    if (W > double(Nw * inc) || first) {
                        W = Nw * inc;
                    }
                    if (E < double(Ne * inc) || first) {
                        E = Ne * inc;
                    }
                    first = false;
                }
            }
        }

        ASSERT(!first);
        ASSERT(W <= E);
        w = W;
        e = E;
    }
}


bool Reduced::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const Reduced*>(&other);
    return (o != nullptr) && Gaussian::sameAs(other);
}


eckit::Fraction Reduced::getSmallestIncrement() const {
    ASSERT(N_);
    using distance_t = std::make_signed<size_t>::type;

    const auto& pl = pls();
    auto maxpl     = *std::max_element(pl.begin() + distance_t(k_), pl.begin() + distance_t(k_ + Nj_));
    ASSERT(maxpl >= 2);

    return {360, maxpl};
}


Iterator* Reduced::unrotatedIterator() const {
    auto pl = pls();
    return new GaussianIterator(latitudes(), std::move(pl), bbox_, N_, Nj_, k_);
}


Iterator* Reduced::rotatedIterator(const util::Rotation& rotation) const {
    auto pl = pls();
    return new GaussianIterator(latitudes(), std::move(pl), bbox_, N_, Nj_, k_, rotation);
}


const std::vector<long>& Reduced::pls() const {
    ASSERT(pl_.size() == N_ * 2);
    ASSERT(pl_.size() >= k_ + Nj_);
    ASSERT(Nj_ > 0);

    return pl_;
}


std::vector<long> Reduced::pls(const std::string& name) {
    atlas::ReducedGaussianGrid grid(name);
    ASSERT(grid);
    return pl_convert(grid.nx());
}


void Reduced::setNj(std::vector<long> pl, double s, double n) {
    ASSERT(0 < N_ && N_ * 2 == pl.size());

    // position to first latitude and first/last longitude
    // NOTE: latitudes() spans the globe, sorted from North-to-South, k_ positions the North
    // NOTE: pl spans the globe
    pl_ = pl;
    k_  = 0;
    Nj_ = N_ * 2;

    const auto& lats = latitudes();
    if (n < lats.front() || s > lats.back()) {
        Nj_ = 0;
        for (const auto& lat : lats) {
            if (n < lat && !angleApproximatelyEqual(n, lat)) {
                ++k_;
            }
            else if (s < lat || angleApproximatelyEqual(s, lat)) {
                ASSERT(pl[k_ + Nj_] >= 2);
                ++Nj_;
            }
            else {
                break;
            }
        }
    }

    // check internal assumptions
    pls();
}


void Reduced::fillGrib(grib_info& info) const {

    // See copy_spec_from_ksec.c in libemos for info

    const auto& pl = pls();

    info.grid.grid_type = CODES_UTIL_GRID_SPEC_REDUCED_GG;
    info.grid.Nj        = long(Nj_);
    info.grid.N         = long(N_);
    info.grid.pl        = &pl[k_];
    info.grid.pl_size   = long(Nj_);

    for (size_t i = k_; i < k_ + Nj_; i++) {
        ASSERT(pl[i] > 0);
    }

    bbox_.fillGrib(info);
}


std::vector<util::GridBox> Reduced::gridBoxes() const {
    using eckit::Fraction;

    ASSERT(1 <= Nj_);


    // latitude edges
    std::vector<double> latEdges = calculateUnrotatedGridBoxLatitudeEdges();

    const auto dom   = domain();
    const auto north = dom.north();
    const auto west  = dom.west();
    const auto south = dom.south();
    const auto east  = dom.east();

    const auto periodic = isPeriodicWestEast();


    // grid boxes
    std::vector<util::GridBox> r;
    r.reserve(numberOfPoints());

    const auto& pl = pls();

    for (size_t j = k_; j < k_ + Nj_; ++j) {
        ASSERT(pl[j] > 0);
        const Fraction inc(360, pl[j]);

        // latitude edges
        const auto n = includesNorthPole() ? latEdges[j] : std::min(north, latEdges[j]);
        const auto s = includesSouthPole() ? latEdges[j + 1] : std::max(south, latEdges[j + 1]);
        ASSERT(n >= s);

        // longitude edges
        const Fraction w(west);
        auto Nw      = (w / inc).integralPart();
        if (Nw * inc < w) {
            Nw += 1;
        }

        const Fraction e(east);
        auto Ne      = (e / inc).integralPart();
        if (Ne * inc > e) {
            Ne -= 1;
        }

        const auto N = std::min(static_cast<size_t>(pl[j]), static_cast<size_t>(Ne - Nw + 1));

        auto lon0 = (Nw * inc) - (inc / 2);
        auto lon1 = lon0;

        if (periodic) {
            for (size_t i = 0; i < N; ++i) {
                auto w = static_cast<double>(lon1);
                lon1 += inc;
                r.emplace_back(n, w, s, static_cast<double>(lon1));
            }

            ASSERT(lon0 == PointLonLat::normalise_angle_to_minimum(lon1, lon0));
        }
        else {
            for (size_t i = 0; i < N; ++i) {
                auto w = std::max(west, static_cast<double>(lon1));
                lon1 += inc;
                r.emplace_back(n, w, s, std::min(east, static_cast<double>(lon1)));
            }

            ASSERT(lon0 <= PointLonLat::normalise_angle_to_minimum(lon1, lon0));
        }
    }

    ASSERT(r.size() == numberOfPoints());
    return r;
}


void Reduced::fillJob(api::MIRJob& job) const {
    Gaussian::fillJob(job);
    job.set("pl", pls());
}


size_t Reduced::frame(MIRValuesVector& values, size_t size, double missingValue) const {
    validate(values);

    size_t count = 0;

    // TODO: Check if that logic cannot also be used for other grid, and therefor move it to a higher class

    std::map<size_t, size_t> shape;

    auto prev_lat = std::numeric_limits<double>::max();
    auto prev_lon = -std::numeric_limits<double>::max();

    size_t rows  = 0;
    size_t dummy = 0;  // Used to keep static analyser quiet
    size_t* col  = &dummy;

    // Collect the 'shape' of the gaussian field
    // This could be done with the latitudes() and pls(), maybe more efficeintly
    // but this code could also be used for all grids
    // and even be cached (md5 of iterators)

    // Iterator is 'unrotated'
    for (const std::unique_ptr<Iterator> it(iterator()); it->next();) {
        const auto& p = it->pointUnrotated();

        if (p.lat != prev_lat) {
            ASSERT(p.lat < prev_lat);  // Assumes scanning mode
            prev_lat = p.lat;
            prev_lon = std::numeric_limits<double>::lowest();

            col    = &shape[rows++];
            (*col) = 0;
        }

        ASSERT(p.lon > prev_lon);  // Assumes scanning mode
        prev_lon = p.lon;
        (*col)++;
    }

    size_t k = 0;
    for (size_t j = 0; j < rows; j++) {
        size_t cols = shape[j];
        for (size_t i = 0; i < cols; i++) {
            if (!((i < size) || (j < size) || (i >= cols - size) || (j >= rows - size))) {
                values[k] = missingValue;
                count++;
            }
            k++;
        }
    }

    ASSERT(k == values.size());
    return count;
}


size_t Reduced::numberOfPoints() const {
    if (isGlobal()) {
        const auto& pl = pls();
        return size_t(std::accumulate(pl.begin(), pl.end(), 0L));
    }

    size_t total = 0;
    for (const std::unique_ptr<Iterator> it(iterator()); it->next();) {
        total++;
    }
    return total;
}


bool Reduced::getLongestElementDiagonal(double& d) const {
    // Look for a majorant of all element diagonals, using the difference of
    // latitudes closest/furthest from equator and longitude furthest from
    // Greenwich

    const auto& pl   = pls();
    const auto& lats = latitudes();

    d = 0.;
    for (size_t j = k_ + 1; j < k_ + Nj_; ++j) {
        auto l1(lats[j - 1]);
        auto l2(lats[j]);

        const eckit::Fraction we(360, std::min(pl[j - 1], pl[j]));

        auto& latAwayFromEquator(std::abs(l1) > std::abs(l2) ? l1 : l2);
        auto& latCloserToEquator(std::abs(l1) > std::abs(l2) ? l2 : l1);

        d = std::max(d,
                     util::Earth::distance(PointLonLat(0., latCloserToEquator), PointLonLat(we, latAwayFromEquator)));
    }

    ASSERT(d > 0.);
    return true;
}


util::BoundingBox Reduced::extendBoundingBox(const util::BoundingBox& bbox) const {
    using eckit::Fraction;

    // adjust West/East to include bbox's West/East
    auto w = bbox.west();
    auto e = bbox.east();
    {
        Fraction globe(360, 1);
        Fraction west(w);
        Fraction east(e);

        bool first = true;
        std::set<long> NiTried;

        const auto& pl = pls();
        for (size_t j = k_; j < k_ + Nj_; ++j) {

            // extend longitude-wise, track distinct attempts
            const long Ni(pl[j]);
            if (NiTried.insert(Ni).second) {

                ASSERT(Ni >= 2);
                auto inc = Fraction(360, Ni);

                auto Nw = (west / inc).integralPart() - 1;
                auto Ne = (east / inc).integralPart() + 1;

                if (w > Fraction(Nw * inc) || first) {
                    w = Nw * inc;
                }
                if (e < Fraction(Ne * inc) || first) {
                    e = Ne * inc;
                }
                first = false;
            }
        }

        ASSERT(!first);

        long NiMax = *std::max_element(NiTried.begin(), NiTried.end());
        ASSERT(NiMax > 0);
        auto inc = Fraction(360, NiMax);

        if (e - w + inc >= globe) {
            w = 0;
            e = globe;
        }

        ASSERT(w < e);
    }


    // adjust South/North to include bbox's South/North ('outwards')
    auto s = bbox.south();
    auto n = bbox.north();
    correctSouthNorth(s, n, false);


    // set bounding box
    const util::BoundingBox extended(n, w, s, e);
    ASSERT(extended.contains(bbox));

    return extended;
}


bool Reduced::isPeriodicWestEast() const {
    auto inc = getSmallestIncrement();
    return bbox_.east() - bbox_.west() + inc >= PointLonLat::FULL_ANGLE;
}


std::string Reduced::factory() const {
    return "reduced_gg";
}


void Reduced::json(eckit::JSON& s) const {
    s.startObject();
    s << "type"
      << "reduced_gg";
    s << "pl" << pls();
    Gaussian::json(s);
    s.endObject();
}


}  // namespace mir::repres::gauss::reduced
