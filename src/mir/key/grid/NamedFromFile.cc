// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/key/grid/NamedFromFile.h"

#include <ostream>

#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::key::grid {


NamedFromFile::NamedFromFile(const std::string& name) : NamedGrid(name) {}


void NamedFromFile::print(std::ostream& out) const {
    out << "NamedFromFile[key=" << key_ << ",parametrisation=";
    SimpleParametrisation::print(out);
    out << "]";
}


size_t NamedFromFile::gaussianNumber() const {
    long N;
    return SimpleParametrisation::get("gaussianNumber", N) && N > 0 ? size_t(N) : default_gaussian_number();
}


const repres::Representation* NamedFromFile::representation() const {
    return repres::RepresentationFactory::build(*this);
}


const repres::Representation* NamedFromFile::representation(const util::Rotation& /*unused*/) const {
    NOTIMP;
}


}  // namespace mir::key::grid
