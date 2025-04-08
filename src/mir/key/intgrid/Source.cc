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

#include <algorithm>
#include <cctype>  // for ::isdigit
#include <memory>

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/SpectralOrder.h"


namespace mir::key::intgrid {


static const IntgridBuilder<Source> __intgrid1("source");
static const IntgridBuilder<Source> __intgrid2("SOURCE");


Source::Source(const param::MIRParametrisation& parametrisation, long /*unused*/) : Intgrid(parametrisation) {
    std::unique_ptr<util::SpectralOrder> spectralOrder(util::SpectralOrderFactory::build("cubic"));
    ASSERT(spectralOrder);

    long T = 0;
    if (std::string truncation; parametrisation_.userParametrisation().get("truncation", truncation) &&
                                !truncation.empty() && std::all_of(truncation.begin(), truncation.end(), ::isdigit)) {
        T = std::stol(truncation);
    }
    else {
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


}  // namespace mir::key::intgrid
