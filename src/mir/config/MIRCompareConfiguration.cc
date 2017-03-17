/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Feb 2017


#include "mir/config/MIRCompareConfiguration.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/parser/JSONParser.h"
#include "mir/config/LibMir.h"
#include "mir/param/InheritParametrisation.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace config {


namespace  {


struct Defaults : param::SimpleParametrisation {
    Defaults() {
        // these options are (can be) overridden by the configuration file

        set("tolerance", 1e-10);
    }
};


}  // (anonymous namespace)


MIRCompareConfiguration& MIRCompareConfiguration::instance() {
    static MIRCompareConfiguration instance_;
    return instance_;
}


void MIRCompareConfiguration::configure(const eckit::PathName& path) {

    // Base configuration
    AConfiguration::configure(path);

    // Use defaults (non-overwriting)
    Defaults().copyValuesTo(root_, false);

    //    eckit::Log::debug<LibMir>() << "MIRConfiguration: " << *root_ << std::endl;
}


MIRCompareConfiguration::MIRCompareConfiguration() : AConfiguration() {

    // Always start with internal defaults, not from file
    configure("");
}


void MIRCompareConfiguration::print(std::ostream& out) const {
    out << "MIRCompareConfiguration["
        << static_cast<const AConfiguration&>(*this)
        << "]";
}


const param::MIRParametrisation* MIRCompareConfiguration::lookup(const param::MIRParametrisation& metadata) const {
    long id = 0;
    return metadata.get("paramId", id)? lookup(id, metadata)
                                      : defaults();
}


const param::MIRParametrisation* MIRCompareConfiguration::lookup(const long& paramId, const param::MIRParametrisation& metadata) const {

    // inherit from most-specific paramId/metadata individual and its parents
    param::SimpleParametrisation* param = new param::SimpleParametrisation();
    root_.pick(paramId, metadata).inherit(*param);

    eckit::Log::info() << "MIRCompareConfiguration::lookup: " << *param << std::endl;
    return param;
}


const param::MIRParametrisation* MIRCompareConfiguration::defaults() const {

    // inherit from top-level only (where defaults are held)
    param::SimpleParametrisation* param = new param::SimpleParametrisation();
    root_.inherit(*param);
    return param;
}


}  // namespace config
}  // namespace mir
