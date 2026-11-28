// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/Representation.h"

#include <algorithm>
#include <memory>
#include <sstream>
#include <variant>
#include <vector>

#include "eckit/geo/Grid.h"
#include "eckit/geo/order/Scan.h"
#include "eckit/geo/projection/Rotation.h"
#include "eckit/spec/Custom.h"
#include "eckit/spec/Layered.h"

#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/key/grid/Grid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/UnstructuredGrid.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::repres {


Representation::Representation() = default;


Representation::~Representation() = default;


RepresentationHandle::RepresentationHandle(const Representation* r) : representation_(r) {
    if (representation_ != nullptr) {
        representation_->attach();
    }
}


RepresentationHandle::RepresentationHandle(const RepresentationHandle& rh) : representation_(rh) {
    if (representation_ != nullptr) {
        representation_->attach();
    }
}


RepresentationHandle::~RepresentationHandle() {
    if (representation_ != nullptr) {
        representation_->detach();
    }
}


std::vector<util::GridBox> Representation::gridBoxes() const {
    std::ostringstream os;
    os << "Representation::gridBoxes() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


bool Representation::isGlobal() const {
    return domain().isGlobal();
}


bool Representation::isPeriodicWestEast() const {
    std::ostringstream os;
    os << "Representation::isPeriodicWestEast() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


bool Representation::includesNorthPole() const {
    std::ostringstream os;
    os << "Representation::includesNorthPole() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


bool Representation::includesSouthPole() const {
    std::ostringstream os;
    os << "Representation::includesSouthPole() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Representation::validate(const MIRValuesVector& /*unused*/) const {
    std::ostringstream os;
    os << "Representation::validate() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Representation::fillGrib(grib_info& /*unused*/) const {
    std::ostringstream os;
    os << "Representation::fillGrib(grib_info&) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Representation::fillJob(api::MIRJob& job) const {
    job.set("grid", spec().str());
}


void Representation::fillSpec(CustomSpec& /*unused*/) const {
    std::ostringstream os;
    os << "Representation::fillSpec(CustomSpec&) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Representation::fillMeshGen(util::MeshGeneratorParameters& /*unused*/) const {
    std::ostringstream os;
    os << "Representation::fillMeshGen(util::MeshGeneratorParameters&) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


std::string Representation::intersectionOnCrop() const {
    std::ostringstream os;
    os << "Representation::intersectionOnCrop() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


const Representation* Representation::croppedRepresentation(const util::BoundingBox& /*unused*/) const {
    std::ostringstream os;
    os << "Representation::croppedRepresentation() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


const Representation* Representation::truncate(size_t /*unused*/, const MIRValuesVector& /*unused*/,
                                               MIRValuesVector& /*unused*/) const {
    std::ostringstream os;
    os << "Representation::truncate() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


const eckit::geo::Grid& Representation::grid() const {
    // representations are shared (across threads), the grid is built on first use
    util::lock_guard<util::recursive_mutex> lock(gridMutex_);

    if (!grid_) {
        CustomSpec custom;
        fillSpec(custom);

        grid_.reset(eckit::geo::GridFactory::build(custom));
        ASSERT(grid_);
    }

    return *grid_;
}


const eckit::geo::Grid& Representation::canonicalGrid() const {
    util::lock_guard<util::recursive_mutex> lock(gridMutex_);

    if (canonicalGrid_ == nullptr) {
        const auto& g     = grid();
        const auto& order = eckit::geo::order::Scan::order_default();

        if (valuesInDefaultOrder() && g.order() != order) {
            eckit::spec::Layered spec(g.spec());
            spec.push_front(new eckit::spec::Custom{{"order", order}});

            canonicalGridOwned_.reset(eckit::geo::GridFactory::build(spec));
            ASSERT(canonicalGridOwned_);
        }

        canonicalGrid_ = canonicalGridOwned_ ? canonicalGridOwned_.get() : &g;
    }

    return *canonicalGrid_;
}


const eckit::geo::Grid& Representation::canonicalGridUnrotated() const {
    util::lock_guard<util::recursive_mutex> lock(gridMutex_);

    if (canonicalGridUnrotated_ == nullptr) {
        const auto& g = canonicalGrid();

        if (const auto* r = dynamic_cast<const eckit::geo::projection::Rotation*>(&g.projection());
            r != nullptr && r->rotated()) {
            eckit::spec::Layered spec(g.spec());
            spec.hide("projection");
            spec.hide("rotation");

            canonicalGridUnrotatedOwned_.reset(eckit::geo::GridFactory::build(spec));
            ASSERT(canonicalGridUnrotatedOwned_);
        }

        canonicalGridUnrotated_ = canonicalGridUnrotatedOwned_ ? canonicalGridUnrotatedOwned_.get() : &g;
    }

    return *canonicalGridUnrotated_;
}


bool Representation::valuesInDefaultOrder() const {
    return false;
}


const Representation::Spec& Representation::spec() const {
    return grid().spec();
}


const std::string& Representation::uniqueName() const {
    if (uniqueName_.empty()) {
        std::ostringstream os;
        makeName(os);
        uniqueName_ = os.str();
    }

    ASSERT(!uniqueName_.empty());
    return uniqueName_;
}


void Representation::makeName(std::ostream& /*unused*/) const {
    std::ostringstream os;
    os << "Representation::makeName(std::ostream& out) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}

void Representation::json(eckit::JSON& /*unused*/) const {
    std::ostringstream os;
    os << "Representation::json(eckit::JSON& out) not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


bool Representation::sameAs(const Representation& /*unused*/) const {
    std::ostringstream os;
    os << "Representation::sameAs() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


atlas::Grid Representation::atlasGrid() const {
    std::ostringstream os;
    os << "Representation::atlasGrid() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


util::Domain Representation::domain() const {
    std::ostringstream os;
    os << "Representation::domain() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


util::Domain Representation::geographicDomain() const {
    return domain();
}


const std::string& Representation::order() const {
    std::ostringstream os;
    os << "Representation::order() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


const util::BoundingBox& Representation::boundingBox() const {
    std::ostringstream os;
    os << "Representation::boundingBox() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


util::BoundingBox Representation::extendBoundingBox(const util::BoundingBox& /*unused*/) const {
    std::ostringstream os;
    os << "Representation::extendBoundingBox() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


bool Representation::crop(util::BoundingBox& /*unused*/, util::IndexMapping& /*unused*/) const {
    std::ostringstream os;
    os << "Representation::crop() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


size_t Representation::truncation() const {
    std::ostringstream os;
    os << "Representation::truncation() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


size_t Representation::numberOfPoints() const {
    std::ostringstream os;
    os << "Representation::numberOfPoints() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


size_t Representation::numberOfValues() const {
    std::ostringstream os;
    os << "Representation::numberOfValues() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


bool Representation::getLongestElementDiagonal(double& /*unused*/) const {
    std::ostringstream os;
    os << "Representation::getLongestElementDiagonal() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Representation::comparison(std::string& /*unused*/) const {
    // do nothing
}


size_t Representation::frame(MIRValuesVector& /*unused*/, size_t /*unused*/, double /*unused*/) const {
    std::ostringstream os;
    os << "Representation::frame() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


std::string Representation::factory() const {
    std::ostringstream os;
    os << "Representation::factory() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Representation::reorderToCanonical(MIRValuesVector& /*unused*/) const {
    std::ostringstream os;
    os << "Representation::reorderToCanonical() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void Representation::reorderFromCanonical(MIRValuesVector& /*unused*/) const {
    std::ostringstream os;
    os << "Representation::reorderFromCanonical() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


const Representation* Representation::globalise(data::MIRField& field) const {
    if (domain().isGlobal()) {
        return nullptr;
    }

    // this representation's points (as the values), then the points of a global grid outside its domain (missing),
    // geographic (rotated, if so)
    const auto n = numberOfPoints();

    std::vector<double> latitudes(n);
    std::vector<double> longitudes(n);
    const auto& points = canonicalGrid();
    for (auto it = points.cbegin(); it; ++it) {
        const auto p = std::get<PointLonLat>(*it);
        ASSERT(it.index() < n);
        latitudes[it.index()]  = p.lat();
        longitudes[it.index()] = p.lon();
    }

    const auto dom = geographicDomain();

    RepresentationHandle octahedral(key::grid::Grid::lookup("O320").representation());
    for (const auto& point : octahedral->canonicalGrid()) {
        if (const auto p = std::get<PointLonLat>(point); !dom.contains(p)) {
            latitudes.push_back(p.lat());
            longitudes.push_back(p.lon());
        }
    }

    const auto size = latitudes.size();
    if (size == n) {
        return nullptr;
    }

    for (size_t i = 0; i < field.dimensions(); i++) {
        const MIRValuesVector& values = field.direct(i);
        ASSERT(values.size() == n);

        MIRValuesVector newvalues(size, field.missingValue());
        std::copy(values.begin(), values.end(), newvalues.begin());

        field.update(newvalues, i);
    }

    field.hasMissing(true);

    return new UnstructuredGrid(latitudes, longitudes);
}


static util::once_flag once;
static util::recursive_mutex* local_mutex               = nullptr;
static std::map<std::string, RepresentationFactory*>* m = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, RepresentationFactory*>();
}


RepresentationFactory::RepresentationFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("RepresentationFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


RepresentationFactory::~RepresentationFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


/// Representations not supported (by gridType): building throws
class RepresentationUnsupportedBuilder final : public RepresentationFactory {
    const std::string name_;

    Representation* make(const param::MIRParametrisation& /*unused*/) override {
        throw exception::FunctionalityNotSupported("Representation: gridType=" + name_ + " is currently unsupported");
    }

public:
    explicit RepresentationUnsupportedBuilder(const std::string& name) : RepresentationFactory(name), name_(name) {}
};


static const RepresentationUnsupportedBuilder AZIMUTH_RANGE("azimuth_range");
static const RepresentationUnsupportedBuilder EQUATORIAL_AZIMUTHAL_EQUIDISTANT("equatorial_azimuthal_equidistant");
static const RepresentationUnsupportedBuilder ROTATED_SH("rotated_sh");
static const RepresentationUnsupportedBuilder STRETCHED_GG("stretched_gg");
static const RepresentationUnsupportedBuilder STRETCHED_LL("stretched_ll");
static const RepresentationUnsupportedBuilder STRETCHED_ROTATED_GG("stretched_rotated_gg");
static const RepresentationUnsupportedBuilder STRETCHED_ROTATED_LL("stretched_rotated_ll");
static const RepresentationUnsupportedBuilder STRETCHED_ROTATED_SH("stretched_rotated_sh");
static const RepresentationUnsupportedBuilder STRETCHED_SH("stretched_sh");


const Representation* RepresentationFactory::build(const param::MIRParametrisation& params) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    std::string name;
    if (!params.get("gridType", name)) {
        throw exception::SeriousBug("RepresentationFactory: cannot get 'gridType'");
    }

    Log::debug() << "RepresentationFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "RepresentationFactory: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("RepresentationFactory: unknown '" + name + "'");
    }

    return j->second->make(params);
}


void RepresentationFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace mir::repres
