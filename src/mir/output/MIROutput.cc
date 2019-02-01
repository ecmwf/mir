/*
 * (C) Copyright 1996- ECMWF.
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


#include "mir/output/MIROutput.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/config/LibMir.h"
#include "mir/output/GribFileOutput.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace output {


MIROutput::MIROutput() {
}


MIROutput::~MIROutput() = default;


namespace {


static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;
static std::map<std::string, MIROutputFactory* > *m_formats = 0;
static std::map<std::string, MIROutputFactory* > *m_extensions = 0;


static void init() {
    local_mutex = new eckit::Mutex();
    m_formats = new std::map<std::string, MIROutputFactory* >();
    m_extensions = new std::map<std::string, MIROutputFactory* >();
}


// Extension handling
struct OutputFromExtension : public MIROutputFactory {

    virtual MIROutput* make(const std::string& path) {
        const eckit::PathName p(path);
        const std::string ext = p.extension();

        auto j = m_extensions->find(ext);
        if (j == m_extensions->cend()) {
            list(eckit::Log::debug<LibMir>() << "OutputFromExtension: unknown extension '" << ext << "', choices are: ");
            eckit::Log::debug<LibMir>() << ", returning 'grib'" << std::endl;

            return new GribFileOutput(p);
        }

        eckit::Log::debug<LibMir>() << "MIROutputFactory: returning '" << ext << "' for '" << path << "'" << std::endl;
        return j->second->make(path);
    }

    static void list(std::ostream& out) {
        pthread_once(&once, init);
        eckit::AutoLock<eckit::Mutex> lock(local_mutex);

        const char* sep = "";
        for (const auto& j : *m_extensions) {
            out << sep << j.first;
            sep = ", ";
        }
    }

    OutputFromExtension() : MIROutputFactory("extension", {}) {
    }

    ~OutputFromExtension() {
        m_extensions->clear();
    }

} static _extension;


}  // (anonymous namespace)


MIROutputFactory::MIROutputFactory(const std::string& name, const std::vector<std::string>& extensions) :
    name_(name),
    extensions_(extensions) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m_formats->find(name) != m_formats->end()) {
        throw eckit::SeriousBug("MIROutputFactory: duplicate '" + name + "'");
    }

    ASSERT(m_formats->find(name) == m_formats->end());
    (*m_formats)[name] = this;

    for (auto& ext : extensions) {
        ASSERT(!ext.empty());
        if (m_extensions->find(ext) != m_extensions->end()) {
            throw eckit::SeriousBug("MIROutputFactory: duplicate extension '" + ext + "'");
        }

        ASSERT(m_extensions->find(name) == m_extensions->end());
        (*m_extensions)[ext] = this;
    }
}


MIROutputFactory::~MIROutputFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    if (m_formats) {
        m_formats->erase(name_);
    }
}


MIROutput* MIROutputFactory::build(const std::string& path, const param::MIRParametrisation& parametrisation) {
    const param::MIRParametrisation& user = parametrisation.userParametrisation();

    std::string format = user.has("dryrun") ? "empty"
                       : user.has("griddef") ? "geopoints"
                       : user.has("latitudes") || user.has("longitudes") ? "geopoints"
                       : "extension"; // maybe "grib"??

    user.get("format", format);

    auto j = m_formats->find(format);
    if (j == m_formats->cend()) {
        list(eckit::Log::error() << "MIROutputFactory: unknown '" << format << "', choices are: ");
        eckit::Log::error() << std::endl;
        throw eckit::SeriousBug("MIROutputFactory: unknown '" + format + "'");
    }

    eckit::Log::debug<LibMir>() << "MIROutputFactory: returning '" << format << "' for '" << path << "'" << std::endl;
    return j->second->make(path);
}


void MIROutputFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m_formats) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace output
}  // namespace mir

