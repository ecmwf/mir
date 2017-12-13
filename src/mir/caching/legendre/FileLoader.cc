/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @author Tiago Quintino
///
/// @date Apr 2015

#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>

#include "mir/caching/legendre/FileLoader.h"

#include "eckit/eckit.h"
#include "eckit/os/Stat.h"
#include "eckit/io/StdFile.h"
#include "eckit/log/Bytes.h"
#include "eckit/log/Timer.h"

#include "mir/config/LibMir.h"

namespace mir {
namespace caching {
namespace legendre {

FileLoader::FileLoader(const param::MIRParametrisation& parametrisation, const eckit::PathName& path)
    : LegendreLoader(parametrisation, path), buffer_(path.size()) {

    //    eckit::TraceTimer<LibMir> timer("Loading legendre coefficients from file");
    eckit::Log::debug<LibMir>() << "Loading legendre coefficients from " << path << std::endl;

    eckit::StdFile file(path);
    ASSERT(::fread(buffer_, 1, buffer_.size(), file) == buffer_.size());
}

FileLoader::~FileLoader() {}

void FileLoader::print(std::ostream& out) const {
    out << "FileLoader[path=" << path_ << ",size=" << eckit::Bytes(buffer_.size()) << "]";
}

const void* FileLoader::address() const {
    return buffer_;
}

size_t FileLoader::size() const {
    return buffer_.size();
}

bool FileLoader::inSharedMemory() const {
    return false;
}

bool FileLoader::shared() {
    return false;
}


namespace {
static LegendreLoaderBuilder<FileLoader> loader("file-io");
}

} // namespace legendre
} // namespace caching
} // namespace mir
