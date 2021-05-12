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


#include "mir/caching/legendre/FileLoader.h"

#include <cstdio>
#include <ostream>

#include "eckit/io/StdFile.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


namespace mir {
namespace caching {
namespace legendre {


FileLoader::FileLoader(const param::MIRParametrisation& parametrisation, const eckit::PathName& path) :
    LegendreLoader(parametrisation, path), buffer_(size_t(path.size())) {

    Log::debug() << "Loading Legendre coefficients from " << path << std::endl;

    eckit::AutoStdFile file(path);
    ASSERT(std::fread(buffer_, 1, buffer_.size(), file) == buffer_.size());
}

FileLoader::~FileLoader() = default;

void FileLoader::print(std::ostream& out) const {
    out << "FileLoader[path=" << path_ << ",size=" << Log::Bytes(buffer_.size()) << "]";
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


static LegendreLoaderBuilder<FileLoader> loader("file-io");


}  // namespace legendre
}  // namespace caching
}  // namespace mir
