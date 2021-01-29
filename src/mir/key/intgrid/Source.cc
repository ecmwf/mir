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


#include "mir/key/intgrid/Source.h"

#include <memory>

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/SpectralOrder.h"


namespace mir {
namespace key {
namespace intgrid {


static IntgridBuilder<Source> __intgrid1("source");
static IntgridBuilder<Source> __intgrid2("SOURCE");


Source::Source(const param::MIRParametrisation& parametrisation, long) : Intgrid(parametrisation) {
    std::unique_ptr<util::SpectralOrder> spectralOrder(util::SpectralOrderFactory::build("cubic"));
    ASSERT(spectralOrder);

    long T = 0;
    if (!(parametrisation_.userParametrisation().get("truncation", T) && T > 0)) {
        ASSERT(parametrisation_.fieldParametrisation().get("truncation", T));
    }
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
}  // namespace key
}  // namespace mir
