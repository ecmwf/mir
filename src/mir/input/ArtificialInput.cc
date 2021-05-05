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


#include "mir/input/ArtificialInput.h"

#include <iomanip>
#include <mutex>
#include <sstream>

#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/ValueMap.h"


namespace mir {
namespace input {


static std::once_flag once;
static std::recursive_mutex* local_mutex                 = nullptr;
static std::map<std::string, ArtificialInputFactory*>* m = nullptr;
static void init() {
    local_mutex = new std::recursive_mutex();
    m           = new std::map<std::string, ArtificialInputFactory*>();
}


ArtificialInput::ArtificialInput(const param::MIRParametrisation& /*ignored*/) : calls_(0) {}


ArtificialInput::~ArtificialInput() = default;


bool ArtificialInput::next() {
    return calls_++ == 0;
}


size_t ArtificialInput::dimensions() const {
    return 1;
}


param::SimpleParametrisation& ArtificialInput::parametrisation(size_t which) {
    ASSERT(which == 0);
    return parametrisation_;
}


const param::MIRParametrisation& ArtificialInput::parametrisation(size_t which) const {
    ASSERT(which == 0);
    return parametrisation_;
}


data::MIRField ArtificialInput::field() const {
    ASSERT(dimensions());

    data::MIRField field(parametrisation(0), false, 9999.);

    repres::RepresentationHandle repres(field.representation());
    auto n = repres->numberOfPoints();
    ASSERT(n);

    for (size_t which = 0; which < dimensions(); ++which) {
        auto values = fill(n);
        field.update(values, which);
    }

    return field;
}


void ArtificialInput::setAuxiliaryInformation(const util::ValueMap& map) {
    map.set(parametrisation_);

    // set additional keys
    if (!parametrisation_.has("grid")) {
        std::vector<double> grid(2, 0);
        if (parametrisation_.get("west_east_increment", grid[0]) &&
            parametrisation_.get("south_north_increment", grid[1])) {
            parametrisation_.set("grid", grid);
        }
    }
    if (!parametrisation_.has("rotation")) {
        std::vector<double> rotation(2, 0);
        if (parametrisation_.get("south_pole_latitude", rotation[0]) &&
            parametrisation_.get("south_pole_longitude", rotation[1])) {
            parametrisation_.set("rotation", rotation);
        }
    }
}


void ArtificialInput::print(std::ostream& out) const {
    out << "ArtificialInput[parametrisation=" << parametrisation_ << "]";
}


bool ArtificialInput::sameAs(const MIRInput& other) const {
    auto o = dynamic_cast<const ArtificialInput*>(&other);
    return (o != nullptr) && parametrisation_.matches(o->parametrisation_);
}


bool ArtificialInput::has(const std::string& name) const {
    return parametrisation_.has(name) || FieldParametrisation::has(name);
}


bool ArtificialInput::get(const std::string& name, std::string& value) const {
    return parametrisation_.get(name, value) || FieldParametrisation::get(name, value);
}


bool ArtificialInput::get(const std::string& name, bool& value) const {
    return parametrisation_.get(name, value) || FieldParametrisation::get(name, value);
}


bool ArtificialInput::get(const std::string& name, int& value) const {
    return parametrisation_.get(name, value) || FieldParametrisation::get(name, value);
}


bool ArtificialInput::get(const std::string& name, long& value) const {
    return parametrisation_.get(name, value) || FieldParametrisation::get(name, value);
}


bool ArtificialInput::get(const std::string& name, float& value) const {
    return parametrisation_.get(name, value) || FieldParametrisation::get(name, value);
}


bool ArtificialInput::get(const std::string& name, double& value) const {
    return parametrisation_.get(name, value) || FieldParametrisation::get(name, value);
}


bool ArtificialInput::get(const std::string& name, std::vector<int>& value) const {
    return parametrisation_.get(name, value) || FieldParametrisation::get(name, value);
}


bool ArtificialInput::get(const std::string& name, std::vector<long>& value) const {
    return parametrisation_.get(name, value) || FieldParametrisation::get(name, value);
}


bool ArtificialInput::get(const std::string& name, std::vector<float>& value) const {
    return parametrisation_.get(name, value) || FieldParametrisation::get(name, value);
}


bool ArtificialInput::get(const std::string& name, std::vector<double>& value) const {
    return parametrisation_.get(name, value) || FieldParametrisation::get(name, value);
}


bool ArtificialInput::get(const std::string& name, std::vector<std::string>& value) const {
    return parametrisation_.get(name, value) || FieldParametrisation::get(name, value);
}


ArtificialInputFactory::ArtificialInputFactory(const std::string& name) : name_(name) {
    std::call_once(once, init);
    std::lock_guard<std::recursive_mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        std::ostringstream oss;
        oss << "ArtificialInputFactory: duplicate '" << name << "'";
        throw exception::SeriousBug(oss.str());
    }

    (*m)[name] = this;
}


ArtificialInputFactory::~ArtificialInputFactory() {
    std::lock_guard<std::recursive_mutex> lock(*local_mutex);
    m->erase(name_);
}


ArtificialInput* ArtificialInputFactory::build(const std::string& name, const param::MIRParametrisation& param) {
    std::call_once(once, init);
    std::lock_guard<std::recursive_mutex> lock(*local_mutex);

    Log::debug() << "ArtificialInputFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "ArtificialInputFactory: unknown '" << name << "', choices are: ");
        Log::warning() << std::endl;
    }

    return j->second->make(param);
}


void ArtificialInputFactory::list(std::ostream& out) {
    std::call_once(once, init);
    std::lock_guard<std::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace input
}  // namespace mir
