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


#include "mir/output/MIROutput.h"

#include <mutex>
#include <sstream>

#include "eckit/filesystem/PathName.h"

#include "mir/output/GribFileOutput.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace output {


MIROutput::MIROutput() = default;


MIROutput::~MIROutput() = default;


static std::once_flag once;
static std::recursive_mutex* fmt_mutex                        = nullptr;
static std::recursive_mutex* ext_mutex                        = nullptr;
static std::map<std::string, MIROutputFactory*>* m_formats    = nullptr;
static std::map<std::string, MIROutputFactory*>* m_extensions = nullptr;
static void init() {
    fmt_mutex    = new std::recursive_mutex();
    ext_mutex    = new std::recursive_mutex();
    m_formats    = new std::map<std::string, MIROutputFactory*>();
    m_extensions = new std::map<std::string, MIROutputFactory*>();
}


struct OutputFromExtension : public MIROutputFactory {

    MIROutput* make(const std::string& path) override {
        const eckit::PathName p(path);
        const std::string ext = p.extension();

        auto j = m_extensions->find(ext);
        if (j == m_extensions->cend()) {
            list(Log::debug() << "OutputFromExtension: unknown extension '" << ext << "', choices are: ");
            Log::debug() << ", returning 'grib'" << std::endl;

            return new GribFileOutput(p);
        }

        Log::debug() << "MIROutputFactory: returning '" << ext << "' for '" << path << "'" << std::endl;
        return j->second->make(path);
    }

    static void list(std::ostream& out) {
        std::call_once(once, init);
        std::lock_guard<std::recursive_mutex> lock(*ext_mutex);

        const char* sep = "";
        for (const auto& j : *m_extensions) {
            out << sep << j.first;
            sep = ", ";
        }
    }

    OutputFromExtension() : MIROutputFactory("extension") {}

    ~OutputFromExtension() override {
        std::lock_guard<std::recursive_mutex> lock(*ext_mutex);
        m_extensions->clear();
    }

} static _extension;


MIROutputFactory::MIROutputFactory(const std::string& name, const std::vector<std::string>& extensions) : name_(name) {
    std::call_once(once, init);
    std::lock_guard<std::recursive_mutex> lock(*fmt_mutex);

    if (m_formats->find(name) != m_formats->end()) {
        throw exception::SeriousBug("MIROutputFactory: duplicate '" + name + "'");
    }

    ASSERT(m_formats->find(name) == m_formats->end());
    (*m_formats)[name] = this;

    for (auto& ext : extensions) {
        ASSERT(!ext.empty());
        if (m_extensions->find(ext) != m_extensions->end()) {
            throw exception::SeriousBug("MIROutputFactory: duplicate extension '" + ext + "'");
        }

        ASSERT(m_extensions->find(name) == m_extensions->end());
        (*m_extensions)[ext] = this;
    }
}


MIROutputFactory::~MIROutputFactory() {
    std::lock_guard<std::recursive_mutex> lock(*fmt_mutex);

    if (m_formats != nullptr) {
        m_formats->erase(name_);
    }
}


MIROutput* MIROutputFactory::build(const std::string& path, const param::MIRParametrisation& parametrisation) {
    std::call_once(once, init);
    std::lock_guard<std::recursive_mutex> lock(*fmt_mutex);

    const auto& user   = parametrisation.userParametrisation();
    std::string format = user.has("griddef") || user.has("latitudes") || user.has("longitudes")
                             ? "geopoints"
                             : "extension";  // maybe "grib"??

    user.get("format", format);

    auto j = m_formats->find(format);
    if (j == m_formats->cend()) {
        list(Log::error() << "MIROutputFactory: unknown '" << format << "', choices are: ");
        Log::error() << std::endl;
        throw exception::SeriousBug("MIROutputFactory: unknown '" + format + "'");
    }

    Log::debug() << "MIROutputFactory: returning '" << format << "' for '" << path << "'" << std::endl;
    return j->second->make(path);
}


void MIROutputFactory::list(std::ostream& out) {
    std::call_once(once, init);
    std::lock_guard<std::recursive_mutex> lock(*fmt_mutex);

    const char* sep = "";
    for (const auto& j : *m_formats) {
        out << sep << j.first;
        sep = ", ";
    }
}


size_t MIROutput::set(const param::MIRParametrisation& param, context::Context& ctx) {
    // redirect to save
    return save(param, ctx);
}


void MIROutput::prepare(const param::MIRParametrisation&, action::ActionPlan&, input::MIRInput&, output::MIROutput&) {
    // do nothing
}


void MIROutput::estimate(const param::MIRParametrisation&, api::MIREstimation&, context::Context&) const {
    std::ostringstream oss;
    oss << "MIROutput::estimate not implemented for " << *this;
    throw exception::SeriousBug(oss.str());
}


}  // namespace output
}  // namespace mir
