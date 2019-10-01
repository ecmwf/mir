/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "mir/style/intgrid/Source.h"

#include <iostream>
#include <memory>

#include "eckit/exception/Exceptions.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/style/SpectralOrder.h"


namespace mir {
namespace style {
namespace intgrid {


static IntgridBuilder< Source > __intgrid1("source");
static IntgridBuilder< Source > __intgrid2("SOURCE");


Source::Source(const param::MIRParametrisation& parametrisation, long) :
    style::Intgrid(parametrisation) {

    std::unique_ptr<SpectralOrder> spectralOrder(SpectralOrderFactory::build("cubic"));
    ASSERT(spectralOrder);

    long T = 0;
    ASSERT(parametrisation_.fieldParametrisation().get("spectral", T));
    ASSERT(T > 0);

    long N = spectralOrder->getGaussianNumberFromTruncation(T);
    ASSERT(N > 0);

    gridname_ = "O" + std::to_string(N);
    ASSERT(!gridname_.empty());
}


const std::string& Source::gridname() const {
    return gridname_;
}


}  // namespace intgrid
}  // namespace style
}  // namespace mir

