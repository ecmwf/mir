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


#include "mir/input/ConstantInput.h"


namespace mir {
namespace input {


static const ArtificialInputBuilder<ConstantInput> __artificial("constant");


MIRValuesVector ConstantInput::fill(size_t n) const {
    double constant = 0;
    parametrisation().get("constant", constant);

    MIRValuesVector values(n, constant);
    return values;
}


}  // namespace input
}  // namespace mir
