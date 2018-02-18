/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/compare/CompareMetadata.h"

#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"


namespace mir_cmp {


Compare::CompareResults CompareMetadata::getFieldMetadata(
        const data::MIRField& field,
        const param::MIRParametrisation& parametrisation) const {
    Compare::CompareResults results;

    results.set("shortName",    "<empty>");
    results.set("packingError", "<empty>");
    results.set("paramId",      "<empty>");
    if (parametrisation.has("shortName"))    { results.setFrom< std::string >(parametrisation, "shortName");    }
    if (parametrisation.has("packingError")) { results.setFrom< double      >(parametrisation, "packingError"); }
    if (parametrisation.has("paramId"))      { results.setFrom< size_t      >(parametrisation, "paramId");      }

    results.set(".hasMissing()", field.hasMissing());
    if (field.hasMissing()) {
        results.set(".missingValue()", field.missingValue());
    }

    results.set(".dimensions()", field.dimensions());
    for (size_t w = 0; w < field.dimensions(); ++w) {
        std::ostringstream oss;
        oss << ".values(" << w << ").size()";
        results.set(oss.str(), field.values(w).size());
    }

    return results;
}


bool CompareMetadata::compare(
        const data::MIRField& field1, const param::MIRParametrisation& param1,
        const data::MIRField& field2, const param::MIRParametrisation& param2 ) const {

    return compareResults(
                getFieldMetadata(field1, param1),
                getFieldMetadata(field2, param2),
                options_ );

}


namespace {
static ComparisonBuilder<CompareMetadata> compare("metadata");
}


}  // namespace mir_cmp

