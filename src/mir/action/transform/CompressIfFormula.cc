/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "mir/action/transform/CompressIfFormula.h"

#include <iostream>
#include <sstream>
#include "eckit/log/Log.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/FormulaParser.h"


namespace mir {
namespace action {
namespace transform {


static CompressIfBuilder< CompressIfFormula > __compressIf("formula");


CompressIfFormula::CompressIfFormula(const param::MIRParametrisation& parametrisation) :
    CompressIf(parametrisation) {

    std::string formula;
    ASSERT(parametrisation.userParametrisation().get("transform-compress-if", formula));

    std::istringstream in(formula);
    util::FormulaParser p(in);

    param::SimpleParametrisation param;
    formula_.reset(p.parse(param));

    ASSERT(formula_);
eckit::Log::debug<LibMir>() << (*formula_) << std::endl;




//    context::Context ctx;
//    f->perform(ctx);
}


bool CompressIfFormula::operator()(const util::BoundingBox& cropping) const {
    return true;
}


void CompressIfFormula::print(std::ostream& out) const {
    out << "CompressIfFormula[formula=" << formula_ << "]";
}


}  // namespace transform
}  // namespace action
}  // namespace mir

