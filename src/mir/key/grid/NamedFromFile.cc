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


#include "mir/key/grid/NamedFromFile.h"

#include <iostream>

#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace key {
namespace grid {


NamedFromFile::NamedFromFile(const std::string& name) : NamedGrid(name) {}


NamedFromFile::~NamedFromFile() = default;


void NamedFromFile::print(std::ostream& out) const {
    out << "NamedFromFile[key=" << key_ << ",parametrisation=";
    SimpleParametrisation::print(out);
    out << "]";
}


size_t NamedFromFile::gaussianNumber() const {
    long N = 64;
    if (!get("gaussianNumber", N)) {
        Log::warning() << "NamedFromFile::gaussianNumber: didn't find key 'gaussianNumber', setting N=" << N
                       << " (hardcoded!)" << std::endl;
    }
    return size_t(N);
}


const repres::Representation* NamedFromFile::representation() const {
    return repres::RepresentationFactory::build(*this);
}


const repres::Representation* NamedFromFile::representation(const util::Rotation&) const {
    NOTIMP;
}


}  // namespace grid
}  // namespace key
}  // namespace mir
