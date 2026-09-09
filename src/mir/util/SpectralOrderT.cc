// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/SpectralOrderT.h"


namespace mir::util {


static const SpectralOrderBuilder<SpectralOrderT<1> > __order1("linear");
static const SpectralOrderBuilder<SpectralOrderT<2> > __order2("quadratic");
static const SpectralOrderBuilder<SpectralOrderT<3> > __order3("cubic");
static const SpectralOrderBuilder<SpectralOrderT<4> > __order4("quartic");


template <>
void SpectralOrderT<1>::print(std::ostream& out) const {
    out << "Linear[]";
}
template <>
void SpectralOrderT<2>::print(std::ostream& out) const {
    out << "Quadratic[]";
}
template <>
void SpectralOrderT<3>::print(std::ostream& out) const {
    out << "Cubic[]";
}
template <>
void SpectralOrderT<4>::print(std::ostream& out) const {
    out << "Quartic[]";
}


}  // namespace mir::util
