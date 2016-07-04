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
/// @date Apr 2015


#include <iostream>

#include "mir/caching/FileLoader.h"

#include <sys/mman.h>
#include <fcntl.h>

#include "eckit/eckit.h"
#include "eckit/os/Stat.h"

#include "eckit/log/Bytes.h"
#include "eckit/io/StdFile.h"
#include "eckit/log/Timer.h"
#include "mir/log/MIR.h"
#include "mir/log/MIR.h"


namespace mir {
namespace caching {


FileLoader::FileLoader(const param::MIRParametrisation &parametrisation, const eckit::PathName &path):
    LegendreLoader(parametrisation, path),
    buffer_(path.size()) {
    eckit::TraceTimer<MIR> timer("Loading legendre coefficients from file");
    eckit::Log::trace<MIR>() << "Loading legendre coefficients from " << path << eckit::newl;

    eckit::StdFile file(path);
    ASSERT(::fread(buffer_, 1, buffer_.size(), file) == buffer_.size());
}


FileLoader::~FileLoader() {
}


void FileLoader::print(std::ostream &out) const {
    out << "FileLoader[path=" << path_ << ",size=" << eckit::Bytes(buffer_.size()) << "]";
}

const void *FileLoader::address() const {
    return buffer_;
}

size_t FileLoader::size() const {
    return buffer_.size();
}

namespace {
static LegendreLoaderBuilder<FileLoader> loader("file-io");
}


}  // namespace caching
}  // namespace mir

