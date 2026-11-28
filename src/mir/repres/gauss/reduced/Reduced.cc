// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/gauss/reduced/Reduced.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <memory>
#include <numeric>
#include <set>
#include <utility>
#include <variant>

#include "eckit/geo/Grid.h"
#include "eckit/geo/grid/Reduced.h"
#include "eckit/geo/range/Regular.h"
#include "eckit/geo/util.h"
#include "eckit/log/JSON.h"
#include "eckit/spec/Custom.h"
#include "eckit/types/FloatCompare.h"

#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Angles.h"
#include "mir/util/Atlas.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/GridBox.h"
#include "mir/util/LongitudeDouble.h"


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


namespace {


constexpr auto EPS = PointLonLat::EPS;


bool lt(double a, double b) {
    return eckit::types::is_strictly_greater(b, a, EPS);
}


bool le(double a, double b) {
    return eckit::types::is_approximately_lesser_or_equal(a, b, EPS);
}


/// The row increment (of Ni longitudes around)
double row_increment(long Ni) {
    ASSERT(Ni > 0);
    return LONGITUDE_GLOBE / static_cast<double>(Ni);
}


}  // namespace


Reduced::Reduced(const param::MIRParametrisation& parametrisation) : Gaussian(parametrisation), k_(0), Nj_(N_ * 2) {

    // adjust latitudes, longitudes and re-set bounding box
    auto n = bbox_.north();
    auto s = bbox_.south();
    correctSouthNorth(s, n);

    std::vector<long> pl;
    ASSERT(parametrisation.get("pl", pl));

    // if pl isn't global (from file!) insert leading/trailing 0's
    const auto& lats = latitudes();
    if (pl.size() != N_ * 2 && (lt(n, lats.front()) || lt(lats.back(), s))) {
        size_t k  = 0;
        size_t nj = 0;
        for (const auto lat : lats) {
            if (lt(n, lat)) {
                ++k;
            }
            else if (le(s, lat)) {
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
    ASSERT(le(w, e));

    const auto smallestIncrement = getSmallestIncrement();
    ASSERT(smallestIncrement > 0);

    if (angleApproximatelyEqual(0., w) &&
        (angleApproximatelyEqual(LONGITUDE_GLOBE - smallestIncrement, e - w) ||
         LONGITUDE_GLOBE - smallestIncrement < e - w ||
         (!eckit::types::is_approximately_equal(e, w, EPS) &&
          eckit::types::is_approximately_equal(util::normalise_longitude(e, w), w, EPS)))) {

        w = 0.;
        e = LONGITUDE_GLOBE - smallestIncrement;
    }
    else {
        // to the longitudes of the rows within the bounds ('inwards'), the widest of all rows
        auto W = w;
        auto E = e;

        bool first = true;
        std::set<long> NiTried;

        const auto& pl = pls();
        for (size_t j = k_; j < k_ + Nj_; ++j) {

            // crop longitude-wise, track distinct attempts
            const long Ni(pl[j]);
            ASSERT(Ni >= 2);
            if (NiTried.insert(Ni).second) {
                const auto x = row(Ni, w, e);

                if (x.size > 0) {
                    const auto w1 = x.first;
                    const auto e1 = x.last;

                    if (W > w1 || first) {
                        W = w1;
                    }
                    if (E < e1 || first) {
                        E = e1;
                    }
                    first = false;
                }
            }
        }

        ASSERT(!first);
        ASSERT(le(W, E));
        w = W;
        e = E;
    }
}


bool Reduced::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const Reduced*>(&other);
    return (o != nullptr) && Gaussian::sameAs(other);
}


double Reduced::getSmallestIncrement() const {
    ASSERT(N_);
    using distance_t = std::make_signed<size_t>::type;

    const auto& pl = pls();
    auto maxpl =
        *std::max_element(pl.begin() + static_cast<distance_t>(k_), pl.begin() + static_cast<distance_t>(k_ + Nj_));
    ASSERT(maxpl >= 2);

    return row_increment(maxpl);
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
    if (lt(n, lats.front()) || lt(lats.back(), s)) {
        Nj_ = 0;
        for (const auto ll : lats) {
            if (lt(n, ll) && !angleApproximatelyEqual(n, ll)) {
                ++k_;
            }
            else if (lt(s, ll) || angleApproximatelyEqual(s, ll)) {
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
    info.grid.Nj        = static_cast<long>(Nj_);
    info.grid.N         = static_cast<long>(N_);
    info.grid.pl        = &pl[k_];
    info.grid.pl_size   = static_cast<long>(Nj_);

    for (size_t i = k_; i < k_ + Nj_; i++) {
        ASSERT(pl[i] > 0);
    }

    // the first/last grid points (of the rows with points), not the bounding box: decoding takes the rows from the
    // latitude of the first point, which a bounding box latitude between rows shifts
    const auto& g    = dynamic_cast<const eckit::geo::grid::Reduced&>(grid());
    const auto& lats = g.latitudes();

    auto first   = true;
    double north = 0;
    double south = 0;
    double west  = 0;
    double east  = 0;
    for (size_t j = 0; j < lats.size(); ++j) {
        if (const auto& lons = g.longitudes(j); !lons.empty()) {
            north = first ? lats[j] : std::max(north, lats[j]);
            south = first ? lats[j] : std::min(south, lats[j]);
            west  = first ? lons.front() : std::min(west, lons.front());
            east  = first ? lons.back() : std::max(east, lons.back());
            first = false;
        }
    }
    ASSERT(!first);

    util::BoundingBox(north, west, south, east).fillGrib(info);
}


std::vector<util::GridBox> Reduced::gridBoxes() const {
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
        const auto inc = row_increment(pl[j]);

        // latitude edges
        const auto n = includesNorthPole() ? latEdges[j] : std::min(north, latEdges[j]);
        const auto s = includesSouthPole() ? latEdges[j + 1] : std::max(south, latEdges[j + 1]);
        ASSERT(n >= s);

        // longitude edges, of the row's longitudes within West/East
        const auto x = row(pl[j], west, east);
        const auto N = std::min(static_cast<size_t>(pl[j]), x.size);

        const auto lon0 = x.first - inc / 2.;
        for (size_t i = 0; i < N; ++i) {
            auto w1 = lon0 + static_cast<double>(i) * inc;
            auto e1 = lon0 + static_cast<double>(i + 1) * inc;
            if (!periodic) {
                w1 = std::max(west, w1);
                e1 = std::min(east, e1);
            }
            r.emplace_back(n, w1, s, e1);
        }
    }

    ASSERT(r.size() == numberOfPoints());
    return r;
}


void Reduced::fillSpec(CustomSpec& spec) const {
    Gaussian::fillSpec(spec);

    // only the rows of the grid compare (a regional pl, eg. from GRIB, is zero elsewhere)
    const auto& pl = pls();
    auto rows      = [this, &pl](const auto& other) {
        return std::equal(pl.begin() + k_, pl.begin() + k_ + Nj_, other.begin() + k_);
    };

    if (rows(eckit::geo::util::reduced_octahedral_pl(N_))) {
        spec.set("grid", "O" + std::to_string(N_));
        return;
    }

    spec.set("grid", "N" + std::to_string(N_));
    if (!eckit::geo::util::reduced_classical_pl_known(N_) || !rows(eckit::geo::util::reduced_classical_pl(N_))) {
        spec.set("pl", pl);
    }
}


size_t Reduced::frame(MIRValuesVector& values, size_t size, double missingValue) const {
    validate(values);

    size_t count = 0;

    // TODO: Check if that logic cannot also be used for other grid, and therefor move it to a higher class

    std::map<size_t, size_t> shape;

    auto prev_lat = std::numeric_limits<double>::max();
    auto prev_lon = std::numeric_limits<double>::lowest();

    size_t rows  = 0;
    size_t dummy = 0;  // Used to keep static analyser quiet
    size_t* col  = &dummy;

    // Collect the 'shape' of the gaussian field
    // This could be done with the latitudes() and pls(), maybe more efficeintly
    // but this code could also be used for all grids
    // and even be cached (md5 of iterators)
    // NOTE: assumes scanning mode

    // points of the grid frame (unrotated)
    for (const auto& point : canonicalGridUnrotated()) {
        const auto p   = std::get<PointLonLat>(point);
        const auto lon = p.lon();
        const auto lat = p.lat();

        if (!eckit::types::is_approximately_equal(lat, prev_lat)) {
            ASSERT(lat < prev_lat);
            prev_lat = lat;
            prev_lon = std::numeric_limits<double>::lowest();

            col    = &shape[rows++];
            (*col) = 0;
        }

        ASSERT(lon > prev_lon);
        prev_lon = lon;
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
        return static_cast<size_t>(std::accumulate(pl.begin(), pl.end(), 0L));
    }

    size_t total = 0;
    for (const auto& point : canonicalGrid()) {
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

        const auto l1 = lats[j - 1];
        const auto l2 = lats[j];

        const auto we                 = row_increment(std::min(pl[j - 1], pl[j]));
        const auto latAwayFromEquator = std::abs(l1) > std::abs(l2) ? l1 : l2;
        const auto latCloserToEquator = std::abs(l1) > std::abs(l2) ? l2 : l1;

        d = std::max(d,
                     util::Earth::distance(PointLonLat{0., latCloserToEquator}, PointLonLat{we, latAwayFromEquator}));
    }

    ASSERT(d > 0.);
    return true;
}


util::BoundingBox Reduced::extendBoundingBox(const util::BoundingBox& bbox) const {

    // adjust West/East to include bbox's West/East (one more longitude of the rows each way, if not around)
    using eckit::geo::range::Regular;

    auto w = bbox.west();
    auto e = bbox.east();
    {
        bool first = true;
        std::set<long> NiTried;

        const auto& pl = pls();
        for (size_t j = k_; j < k_ + Nj_; ++j) {

            // extend longitude-wise, track distinct attempts
            const long Ni(pl[j]);
            if (NiTried.insert(Ni).second) {

                ASSERT(Ni >= 2);
                const auto inc = row_increment(Ni);

                // from the longitudes truncated towards zero
                const auto w1 = Regular::adjust(bbox.west(), inc, 0., bbox.west() < 0., EPS) - inc;
                const auto e1 = Regular::adjust(bbox.east(), inc, 0., bbox.east() < 0., EPS) + inc;

                if (w > w1 || first) {
                    w = w1;
                }
                if (e < e1 || first) {
                    e = e1;
                }
                first = false;
            }
        }

        ASSERT(!first);

        long NiMax = *std::max_element(NiTried.begin(), NiTried.end());
        ASSERT(NiMax > 0);

        if (le(LONGITUDE_GLOBE, e - w + row_increment(NiMax))) {
            w = 0.;
            e = LONGITUDE_GLOBE;
        }

        ASSERT(lt(w, e));
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
    return le(LONGITUDE_GLOBE, bbox_.east() - bbox_.west() + getSmallestIncrement());
}


const Representation* Reduced::globalise(data::MIRField& field) const {
    ASSERT(field.representation() == this);

    if (isGlobal()) {
        return nullptr;
    }

    // the global index of each point, by Gaussian latitude (row) and longitude (column) in the grid coordinates
    const auto& pl   = pls();
    const auto& lats = latitudes();
    ASSERT(lats.size() == pl.size());

    std::vector<size_t> first(pl.size() + 1, 0);
    for (size_t j = 0; j < pl.size(); ++j) {
        ASSERT(pl[j] > 0);
        first[j + 1] = first[j] + static_cast<size_t>(pl[j]);
    }

    const auto n = numberOfPoints();
    std::vector<size_t> index(n);

    const auto& points = canonicalGridUnrotated();
    for (auto it = points.cbegin(); it; ++it) {
        // (unrotated: in the grid coordinates)
        const auto p    = std::get<PointLonLat>(*it);
        const auto plat = p.lat();
        const auto plon = p.lon();

        // the nearest latitude (in decreasing order)
        auto lat = std::lower_bound(lats.begin(), lats.end(), plat, std::greater<>());
        if (lat == lats.end() || (lat != lats.begin() && std::abs(*(lat - 1) - plat) < std::abs(*lat - plat))) {
            --lat;
        }
        const auto j = static_cast<size_t>(lat - lats.begin());

        const auto Ni = pl[j];
        const auto i =
            std::lround(util::normalise_longitude(plon, 0.) * static_cast<double>(Ni) / LONGITUDE_GLOBE) % Ni;

        ASSERT(it.index() < n);
        index[it.index()] = first[j] + static_cast<size_t>(i);
    }

    // the values in place, missing values elsewhere
    for (size_t d = 0; d < field.dimensions(); ++d) {
        const MIRValuesVector& values = field.direct(d);
        ASSERT(values.size() == n);

        MIRValuesVector newvalues(first.back(), field.missingValue());
        for (size_t k = 0; k < n; ++k) {
            newvalues[index[k]] = values[k];
        }

        field.update(newvalues, d);
    }

    field.hasMissing(true);

    // the global grid (of the same N, pl and rotation, if any)
    const auto* global = croppedRepresentation(util::BoundingBox());
    ASSERT(global->numberOfPoints() == first.back());

    return global;
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
