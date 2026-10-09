// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/grid/Grid.h"

#include <algorithm>
#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <sstream>

#include "eckit/filesystem/PathName.h"
#include "eckit/geo/Grid.h"
#include "eckit/geo/Projection.h"
#include "eckit/geo/grid/reduced/ReducedGaussian.h"
#include "eckit/geo/grid/regular/RegularGaussian.h"
#include "eckit/geo/order/Scan.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/types/Fraction.h"
#include "eckit/utils/StringTools.h"

#include "mir/config/LibMir.h"
#include "mir/key/grid/GridPattern.h"
#include "mir/key/grid/NamedGrid.h"
#include "mir/key/intgrid/Source.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/repres/gauss/reduced/ReducedClassic.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"
#include "mir/repres/gauss/reduced/RotatedClassic.h"
#include "mir/repres/gauss/reduced/RotatedOctahedral.h"
#include "mir/repres/gauss/regular/RegularGG.h"
#include "mir/repres/gauss/regular/RotatedGG.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/repres/regular/Lambert.h"
#include "mir/repres/regular/LambertAzimuthalEqualArea.h"
#include "mir/repres/regular/PolarStereographic.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"
#include "mir/util/Regex.h"
#include "mir/util/SpectralOrder.h"
#include "mir/util/Translator.h"
#include "mir/util/ValueMap.h"


namespace mir::key::grid {


static util::once_flag once;
static std::map<std::string, Grid*>* m    = nullptr;
static util::recursive_mutex* local_mutex = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, Grid*>();
}

static const util::Regex SOURCE("^[sS][oO][uU][rR][cC][eE]$");


namespace {


// Gaussian grids: classic (N), octahedral (O) and regular (F)
template <typename GLOBAL, typename ROTATED>
class NamedGaussian final : public NamedGrid {
public:
    NamedGaussian(const std::string& name, size_t N) : NamedGrid(name), N_(N) {}

private:
    const size_t N_;

    void print(std::ostream& out) const override { out << "NamedGaussian[key=" << key_ << ",N=" << N_ << "]"; }
    size_t gaussianNumber() const override { return N_; }
    bool isGaussian() const override { return true; }

    const repres::Representation* representation() const override { return new GLOBAL(N_); }
    const repres::Representation* representation(const util::Rotation& rotation) const override {
        return new ROTATED(N_, rotation);
    }
};


template <typename GLOBAL, typename ROTATED>
class NamedGaussianPattern final : public GridPattern {
public:
    NamedGaussianPattern(const std::string& pattern, char prefix) : GridPattern(pattern), prefix_(prefix) {}

private:
    const char prefix_;

    void print(std::ostream& out) const override { out << "NamedGaussianPattern[pattern=" << pattern_ << "]"; }

    const Grid* make(const std::string& name) const override {
        return new NamedGaussian<GLOBAL, ROTATED>(name, util::from_string<size_t>(name.substr(1)));
    }

    std::string canonical(const std::string& name) const override {
        ASSERT(name.size() > 1);
        return prefix_ + name.substr(1);
    }
};


const NamedGaussianPattern<repres::gauss::reduced::ReducedClassic, repres::gauss::reduced::RotatedClassic>
    CLASSIC_PATTERN("^[nN][1-9][0-9]*$", 'N');

const NamedGaussianPattern<repres::gauss::reduced::ReducedOctahedral, repres::gauss::reduced::RotatedOctahedral>
    OCTAHEDRAL_PATTERN("^[oO][1-9][0-9]*$", 'O');

const NamedGaussianPattern<repres::gauss::regular::RegularGG, repres::gauss::regular::RotatedGG> REGULAR_PATTERN(
    "^[fF][1-9][0-9]*$", 'F');


// Named grids from the configuration file (grids.yaml)
class NamedFromFile final : public NamedGrid, public param::SimpleParametrisation {
public:
    explicit NamedFromFile(const std::string& name) : NamedGrid(name) {}

private:
    void print(std::ostream& out) const override {
        out << "NamedFromFile[key=" << key_ << ",parametrisation=";
        SimpleParametrisation::print(out);
        out << "]";
    }

    size_t gaussianNumber() const override {
        long N = 0;
        return SimpleParametrisation::get("gaussianNumber", N) && N > 0 ? static_cast<size_t>(N)
                                                                        : default_gaussian_number();
    }

    const repres::Representation* representation() const override {
        return repres::RepresentationFactory::build(*this);
    }

    const repres::Representation* representation(const util::Rotation&) const override { NOTIMP; }
};


// Regular latitude/longitude grids, by increments (west-east/south-north)
class RegularLL final : public Grid {
public:
    explicit RegularLL(const std::string& key) : Grid(key, "regular-ll") {}

private:
    util::Increments increments() const {
        auto grid_str = eckit::StringTools::split("/", key_);
        ASSERT_KEYWORD_GRID_SIZE(grid_str.size());

        return util::Increments{util::from_string<double>(grid_str[0]), util::from_string<double>(grid_str[1])};
    }

    void print(std::ostream& out) const override { out << "RegularLL[key=" << key_ << "]"; }

    size_t gaussianNumber() const override {
        auto r = Latitude::GLOBE.fraction() / increments().south_north().latitude().fraction();
        auto N = r.integralPart() / 2;

        ASSERT(N >= 0);
        return static_cast<size_t>(N);
    }

    const repres::Representation* representation() const override {
        return new repres::latlon::RegularLL(increments());
    }
};


class RegularLLPattern final : public GridPattern {
public:
    explicit RegularLLPattern(const std::string& pattern) : GridPattern(pattern) {}

private:
    void print(std::ostream& out) const override { out << "RegularLLPattern[pattern=" << pattern_ << "]"; }
    const Grid* make(const std::string& name) const override { return new RegularLL(name); }

    std::string canonical(const std::string& name) const override {
        auto split = eckit::StringTools::split("/", name);
        ASSERT(split.size() == 2);

        std::ostringstream str;
        str << util::from_string<double>(split[0]) << '/'
            << util::from_string<double>(split[1]);  // better than using std::to_string
        return str.str();
    }
};


#define fp "[+]?([0-9]*[.])?[0-9]+([eE][-+][0-9]+)?"
const RegularLLPattern REGULAR_LL_PATTERN("^" fp "/" fp "$");
#undef fp


// Grids by type and key=value pairs (gridType=...;key=value;...)
template <typename REPRES>
class TypedGrid final : public Grid {
public:
    TypedGrid(const std::string& key, const std::set<std::string>& requiredKeys,
              const std::set<std::string>& optionalKeys) :
        Grid(key, "typedgrid"), requiredKeys_(requiredKeys), optionalKeys_(optionalKeys) {
        requiredKeys_.insert("gridType");
    }

private:
    std::set<std::string> requiredKeys_;
    std::set<std::string> optionalKeys_;

    void checkRequiredKeys(const param::MIRParametrisation& param) const {
        std::string missingKeys;

        const auto* sep = "";
        for (const auto& key : requiredKeys_) {
            if (!param.has(key)) {
                missingKeys += sep + key;
                sep = ", ";
            }
        }

        if (!missingKeys.empty()) {
            std::ostringstream msg;
            msg << *this << ": required keys are missing: " << missingKeys;
            Log::error() << msg.str() << std::endl;
            throw exception::UserError(msg.str());
        }
    }

    void print(std::ostream& out) const override {
        out << "TypedGrid[key=" << key_ << ",requiredKeys=[" << eckit::StringTools::join(",", requiredKeys_)
            << "],optionalKeys=[" << eckit::StringTools::join(",", optionalKeys_) << "]]";
    }

    void parametrisation(const std::string& grid, param::SimpleParametrisation& param) const override {
        // set a new parametrisation containing only required or optional keys
        param::SimpleParametrisation p;
        for (auto& kv_str : eckit::StringTools::split(";", grid)) {
            if (auto it = kv_str.find("="); it != std::string::npos) {
                if (auto k = kv_str.substr(0, it), v = kv_str.substr(it + 1); !k.empty() && !v.empty()) {
                    if (requiredKeys_.count(k) != 0 || optionalKeys_.count(k) != 0) {
                        p.set(k, v);
                        continue;
                    }
                }
            }

            throw exception::UserError("TypedGrid: invalid key=value pair, got '" + kv_str + "'");
        }

        // check for missing keys, set return parametrisation
        checkRequiredKeys(p);
        param.swap(p);
    }

    size_t gaussianNumber() const override {
        param::SimpleParametrisation param;
        parametrisation(key_, param);

        long N = 0;
        return param.get("gaussianNumber", N) && N > 0 ? static_cast<size_t>(N) : default_gaussian_number();
    }

    const repres::Representation* representation(const param::MIRParametrisation& param) const override {
        checkRequiredKeys(param);
        return new REPRES(param);
    }
};


template <typename REPRES>
class TypedGridPattern final : public GridPattern {
public:
    TypedGridPattern(const std::string& pattern, const std::set<std::string>& requiredKeys,
                     const std::set<std::string>& optionalKeys) :
        GridPattern(pattern), requiredKeys_(requiredKeys), optionalKeys_(optionalKeys) {}

private:
    const std::set<std::string> requiredKeys_;
    const std::set<std::string> optionalKeys_;

    void print(std::ostream& out) const override {
        out << "TypedGridPattern[pattern=" << pattern_ << ",requiredKeys=["
            << eckit::StringTools::join(",", requiredKeys_) << "],optionalKeys=["
            << eckit::StringTools::join(",", optionalKeys_) << "]]";
    }

    const Grid* make(const std::string& name) const override {
        return new TypedGrid<REPRES>(name, requiredKeys_, optionalKeys_);
    }

    std::string canonical(const std::string& name) const override { return name; }  // FIXME not implemented
};


const TypedGridPattern<repres::regular::Lambert> LAMBERT_PATTERN(
    "^gridType=lambert;.*$",
    {"LaDInDegrees", "LoVInDegrees", "Ni", "Nj", "grid", "latitudeOfFirstGridPointInDegrees",
     "longitudeOfFirstGridPointInDegrees"},
    {"Latin1InDegrees", "Latin2InDegrees", "writeLaDInDegrees", "writeLonPositive", "gaussianNumber", "shapeOfTheEarth",
     "radius", "earthMajorAxis", "earthMinorAxis"});


const TypedGridPattern<repres::regular::LambertAzimuthalEqualArea> LAMBERT_AZIMUTHAL_EQUAL_AREA_PATTERN(
    "^gridType=lambert_azimuthal_equal_area;.*$",
    {"standardParallelInDegrees", "centralLongitudeInDegrees", "Ni", "Nj", "grid", "latitudeOfFirstGridPointInDegrees",
     "longitudeOfFirstGridPointInDegrees"},
    {"gaussianNumber", "shapeOfTheEarth", "radius", "earthMajorAxis", "earthMinorAxis"});


const TypedGridPattern<repres::regular::PolarStereographic> POLAR_STEREOGRAPHIC_PATTERN(
    "^gridType=polar_stereographic;.*$",
    {"proj", "LaDInDegrees", "orientationOfTheGridInDegrees", "southPoleOnProjectionPlane", "Ni", "Nj", "grid",
     "latitudeOfFirstGridPointInDegrees", "longitudeOfFirstGridPointInDegrees"},
    {"gaussianNumber", "shapeOfTheEarth", "radius", "earthMajorAxis", "earthMinorAxis", "iScansNegatively",
     "jScansPositively"});


// Grids by specification, inline (eckit::geo)
class GridSpec final : public Grid {
public:
    explicit GridSpec(const std::string& key) : Grid(key, "gridspec") {
        std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(key));

        if (const auto* gg = dynamic_cast<const eckit::geo::grid::reduced::ReducedGaussian*>(grid.get())) {
            N_ = gg->N();
        }
        else if (const auto* gg = dynamic_cast<const eckit::geo::grid::regular::RegularGaussian*>(grid.get())) {
            N_ = gg->N();
        }

        gaussian_ =
            N_ > 0 && grid->projection().is_default() && grid->order() == eckit::geo::order::Scan::order_default();
    }

private:
    size_t N_      = 0;
    bool gaussian_ = false;

    void print(std::ostream& out) const override { out << "GridSpec[key=" << key_ << "]"; }
    size_t gaussianNumber() const override { return N_ > 0 ? N_ : default_gaussian_number(); }
    bool isGaussian() const override { return gaussian_; }
};


class GridSpecPattern final : public GridPattern {
public:
    explicit GridSpecPattern(const std::string& pattern) : GridPattern(pattern) {}

private:
    bool matches(const std::string& name) const override {
        // "^[{].*[}]$" only asks whether name looks like "{...}". Answering that with std::regex_match lets
        // libstdc++'s default matcher recurse once or twice per character of name, overflowing the stack for
        // long inline grid specs. The check itself needs no regex, so it runs first; the regex remains the fallback.
        return (!name.empty() && name.front() == '{' && name.back() == '}') || GridPattern::matches(name);
    }

    void print(std::ostream& out) const override { out << "GridSpecPattern[pattern=" << pattern_ << "]"; }
    const Grid* make(const std::string& name) const override { return new GridSpec(name); }

    std::string canonical(const std::string& name) const override {
        std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(name));
        return grid->spec_str();
    }
};


const GridSpecPattern GRIDSPEC_PATTERN("^[{].*[}]$");


}  // namespace


static void read_configuration_files() {
    static bool files_read = false;
    if (files_read) {
        return;
    }
    files_read = true;

    // Read config file, attaching new Grid's grids to parametrisations
    const auto path = LibMir::configFile(LibMir::config_file::GRIDS);
    if (path.exists()) {
        Log::debug() << "Grid: reading from '" << path << "'" << std::endl;

        util::ValueMap grids(eckit::YAMLParser::decodeFile(path));
        for (const auto& g : grids) {

            // This registers a new Grid (don't delete pointer)
            auto* ng = new NamedFromFile(g.first);
            ASSERT(ng);

            util::ValueMap map(g.second);
            map.set(*ng);

            Log::debug() << static_cast<const Grid&>(*ng) << std::endl;
        }
    }
}


Grid::Grid(const std::string& key, const std::string& type) : key_(key), type_(type) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(local_mutex);

    ASSERT(m->insert({key, this}).second);
}


Grid::~Grid() {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(local_mutex);

    ASSERT(m->find(key_) != m->end());
    m->erase(key_);
}


void Grid::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const auto* sep = "";
    for (auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
    out << std::endl;
}


const repres::Representation* Grid::representation() const {
    std::ostringstream os;
    os << "Grid::representation() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


const repres::Representation* Grid::representation(const util::Rotation& /*unused*/) const {
    std::ostringstream os;
    os << "Grid::representation(Rotation&) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


const repres::Representation* Grid::representation(const param::MIRParametrisation& /*unused*/) const {
    std::ostringstream os;
    os << "Grid::representation(MIRParametrisation&) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Grid::parametrisation(const std::string& /*unused*/, param::SimpleParametrisation& /*unused*/) const {
    std::ostringstream os;
    os << "Grid::parametrisation() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


size_t Grid::gaussianNumber() const {
    std::ostringstream os;
    os << "Grid::gaussianNumber() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


bool Grid::isGaussian() const {
    return false;
}


std::string Grid::gridname() const {
    std::ostringstream os;
    os << "Grid::gridname() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


Target::Target(const param::MIRParametrisation& param) {
    const auto& user = param.userParametrisation();
    rotated          = user.has("rotation");

    long N = 0;
    if (user.has("grid") && Grid::get("grid", grid, param)) {
        const auto& g = Grid::lookup(grid);
        type          = g.type();
        gaussian      = g.isGaussian();
        N             = static_cast<long>(g.gaussianNumber());
    }
    else if (user.get("reduced", N)) {
        type     = "reduced-gg";
        gaussian = true;
    }
    else if (user.get("regular", N)) {
        type     = "regular-gg";
        gaussian = true;
    }
    else if (user.get("octahedral", N)) {
        type     = "octahedral-gg";
        gaussian = true;
    }
    else if (user.has("pl")) {
        type     = "reduced-gg-pl-given";
        gaussian = true;
    }
    else if (user.has("griddef")) {
        type = "griddef";
        N    = static_cast<long>(Grid::default_gaussian_number());
    }
    else if (user.has("latitudes") || user.has("longitudes")) {
        type = "points";
        N    = static_cast<long>(Grid::default_gaussian_number());
    }

    // limited by the spectral input truncation
    if (bool spectral = false; param.fieldParametrisation().get("spectral", spectral) && spectral) {
        long T = 0;
        ASSERT(param.fieldParametrisation().get("truncation", T) && T > 0);

        std::unique_ptr<util::SpectralOrder> order(util::SpectralOrderFactory::build("cubic"));
        auto Ninput = order->getGaussianNumberFromTruncation(T);
        N           = N > 0 ? std::min(N, Ninput) : Ninput;
    }

    ASSERT(N >= 0);
    gaussianNumber = N;
}


std::string Grid::canonical(const std::string& name, const param::MIRParametrisation& param) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    read_configuration_files();

    if (m->find(name) != m->end()) {
        return name;
    }

    // grid name from the spectral truncation
    if (SOURCE.match(name)) {
        return param.fieldParametrisation().has("truncation") ? intgrid::Source(param).gridname() : "";
    }

    return GridPattern::match(name);
}


bool Grid::get(const std::string& key, std::string& value, const param::MIRParametrisation& param) {
    std::string name;
    if (!param.get(key, name)) {
        return false;
    }

    value = canonical(name, param);
    return !value.empty();
}


const Grid& Grid::lookup(const std::string& key) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    read_configuration_files();

    Log::debug() << "Grid: looking for '" << key << "'" << std::endl;

    // Look for specific key matches
    if (auto j = m->find(key); j != m->end()) {
        return *(j->second);
    }

    // Look for pattern matchings
    // This will automatically add the new Grid to the map
    auto match = GridPattern::match(key);
    if (!match.empty()) {
        const auto* gp = GridPattern::lookup(match);
        ASSERT(gp != nullptr);
        return *gp;
    }

    list(Log::error() << "Grid: unknown '" << key << "', choices are:\n");
    throw exception::SeriousBug("Grid: unknown '" + key + "'");
}


}  // namespace mir::key::grid
