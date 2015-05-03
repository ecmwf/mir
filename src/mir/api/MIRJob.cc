/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/log/Timer.h"
#include "eckit/runtime/Context.h"

#include "mir/api/mir_config.h"

#include "mir/action/Action.h"
#include "mir/action/ActionPlan.h"

#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/logic/MIRLogic.h"
#include "mir/output/MIROutput.h"
#include "mir/param/MIRCombinedParametrisation.h"
#include "mir/param/MIRConfiguration.h"
#include "mir/param/MIRDefaults.h"

#include "mir/repres/Representation.h"

#include "mir/api/MIRJob.h"


namespace mir {
namespace api {


MIRJob::MIRJob() {
}


MIRJob::~MIRJob() {
}


void MIRJob::execute(input::MIRInput& input, output::MIROutput& output) const {
    // Optimisation: nothing to do, usefull for MARS
    if (size() == 0) {
        eckit::Log::info() << "Nothing to do (no request)" << std::endl;
        output.copy(*this, input);
        return;
    }

    // Static so it is inited once (mutex?)
    static param::MIRConfiguration configuration;
    eckit::Log::info() << "Configuration: " << configuration << std::endl;

    static param::MIRDefaults defaults;
    eckit::Log::info() << "Defaults: " << defaults << std::endl;

    eckit::Timer timer("MIRJob::execute");

    eckit::Log::info() << "MIRJob::execute: " << *this << std::endl;
    eckit::Log::info() << "          Input: " << input << std::endl;
    eckit::Log::info() << "         Output: " << output << std::endl;

    const param::MIRParametrisation& metadata = input.parametrisation();

    if (matches(metadata)) {
        eckit::Log::info() << "Nothing to do (field matches)" << std::endl;
        output.copy(*this, input);
        return;
    }

    param::MIRCombinedParametrisation combined(*this, metadata, configuration, defaults);
    eckit::Log::info() << "Combined parametrisation: " << combined << std::endl;

    std::auto_ptr< logic::MIRLogic > logic(logic::MIRLogicFactory::build(combined));

    eckit::Log::info() << "Logic: " << *logic << std::endl;

    action::ActionPlan plan(combined);
    logic->prepare(plan);

    eckit::Log::info() << "Action plan is: " << plan << std::endl;

    std::auto_ptr< data::MIRField > field(input.field());
    eckit::Log::info() << "Field is " << *field << std::endl;

    // Add Grid to field
    field->representation(repres::RepresentationFactory::build(metadata));
    eckit::Log::info() << "Representation is " << *(field->representation()) << std::endl;

    plan.execute(*field);

    output.save(*this, input, *field);
}


void MIRJob::print(std::ostream& out) const {
    out << "MIRJob[";
    SimpleParametrisation::print(out);
    out << "]";
}

MIRJob& MIRJob::clear(const std::string& name) {
    eckit::Log::info() << "************* MIRJob::clear [" << name << "]" << std::endl;
    SimpleParametrisation::clear(name);
    return *this;
}
MIRJob& MIRJob::set(const std::string& name, const std::string& value) {
    eckit::Log::info() << "************* MIRJob::set [" << name << "] = [" << value << "] (string)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}

MIRJob& MIRJob::set(const std::string& name, const char* value) {
    eckit::Log::info() << "************* MIRJob::set [" << name << "] = [" << value << "] (char)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}

MIRJob& MIRJob::set(const std::string& name, bool value) {
    eckit::Log::info() << "************* MIRJob::set [" << name << "] = [" << value << "] (bool)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, long value) {
    eckit::Log::info() << "************* MIRJob::set [" << name << "] = [" << value << "] (long)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}


MIRJob& MIRJob::set(const std::string& name, double value) {
    eckit::Log::info() << "************* MIRJob::set [" << name << "] = [" << value << "] (double)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}

MIRJob& MIRJob::set(const std::string& name, param::DelayedParametrisation* value) {
    eckit::Log::info() << "************* MIRJob::set [" << name << "] = [" << value << "] (delayed)" << std::endl;
    SimpleParametrisation::set(name, value);
    return *this;
}

MIRJob& MIRJob::set(const std::string& name, double v1, double v2) {
    eckit::Log::info() << "************* MIRJob::set [" << name << "] = [" << v1 << ", "  << v2 << "] (double)" << std::endl;
    std::vector<double> v(2);
    v[0] = v1;
    v[1] = v2;
    SimpleParametrisation::set(name, v);
    return *this;
}

MIRJob& MIRJob::set(const std::string& name, double v1, double v2, double v3, double v4) {
    eckit::Log::info() << "************* MIRJob::set [" << name << "] =  [" << v1
                       << ", "  << v2 << ", "  << v3 << ", "  << v4 << "] (double)" << std::endl;
    std::vector<double> v(4);
    v[0] = v1;
    v[1] = v2;
    v[2] = v3;
    v[3] = v4;
    SimpleParametrisation::set(name, v);
    return *this;
}


bool MIRJob::matches(const param::MIRParametrisation& metadata) const {
    static const char* force[] = { "vod2uv", "bitmap", "frame", "packing", "accuracy", 0 }; // Move to MIRLogic
    size_t i = 0;
    while (force[i]) {
        if (has(force[i])) {
            eckit::Log::info() << "MIRJob will perform transformation/interpolation ('" << force[i] << "'' specified)" << std::endl;
            return false;
        }
        i++;
    }

    bool ok = SimpleParametrisation::matches(metadata);
    if (!ok) {
        eckit::Log::info() << "MIRJob will perform transformation/interpolation" << std::endl;
    }
    return ok;
}

// This comes grom eckit::Context
static eckit::RegisterConfigHome configs("mir",
                                 MIR_INSTALL_BIN_DIR,
                                 MIR_DEVELOPER_BIN_DIR,
                                 MIR_DATA_DIR,
                                 MIR_DEVELOPER_SRC_DIR);

}  // namespace api
}  // namespace mir

