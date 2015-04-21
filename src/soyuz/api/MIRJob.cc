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

#include "soyuz/action/Action.h"
#include "soyuz/data/MIRField.h"
#include "soyuz/input/MIRInput.h"
#include "soyuz/logic/MIRLogic.h"
#include "soyuz/output/MIROutput.h"
#include "soyuz/param/MIRCombinedParametrisation.h"
#include "soyuz/param/MIRConfiguration.h"
#include "soyuz/param/MIRDefaults.h"
#include "soyuz/repres/Representation.h"

#include "soyuz/api/MIRJob.h"


MIRJob::MIRJob() {
}


MIRJob::~MIRJob() {
}


void MIRJob::execute(mir::input::MIRInput& input, mir::output::MIROutput& output) const {
    // Optimisation: nothing to do, usefull for MARS
    if(settings_.size() == 0) {
        eckit::Log::info() << "Nothing to do (no request)" << std::endl;
        output.copy(*this, input);
        return;
    }

    // Static so it is inited once (mutex?)
    static MIRConfiguration configuration;
    static MIRDefaults defaults;

    eckit::Timer timer("MIRJob::execute");

    eckit::Log::info() << "MIRJob::execute: " << *this << std::endl;
    eckit::Log::info() << "          Input: " << input << std::endl;
    eckit::Log::info() << "         Output: " << output << std::endl;

    const MIRParametrisation& metadata = input.parametrisation();

    if (matches(metadata)) {
        eckit::Log::info() << "Nothing to do (field matches)" << std::endl;
        output.copy(*this, input);
        return;
    }

    MIRCombinedParametrisation combined(*this, metadata, configuration, defaults);
    eckit::Log::info() << "        Combined: " << combined << std::endl;

    std::auto_ptr< mir::logic::MIRLogic > logic(mir::logic::MIRLogicFactory::build(combined));

    eckit::Log::info() << "Logic: " << *logic << std::endl;

    std::vector<std::auto_ptr< mir::action::Action > > actions;
    logic->prepare(actions);

    eckit::Log::info() << "Actions are: " << std::endl;
    std::string arrow = "   ";
    for(std::vector<std::auto_ptr< mir::action::Action > >::const_iterator j = actions.begin(); j != actions.end(); ++j) {
        eckit::Log::info() << arrow << *(*j);
        arrow = " => ";
    }
    eckit::Log::info() << std::endl;

    std::auto_ptr<MIRField> field(input.field());
    eckit::Log::info() << "Field is " << *field << std::endl;

    // Add Grid to field
    field->representation(RepresentationFactory::build(metadata));
    eckit::Log::info() << "Representation is " << *(field->representation()) << std::endl;

    for(std::vector<std::auto_ptr< mir::action::Action > >::const_iterator j = actions.begin(); j != actions.end(); ++j) {
        eckit::Log::info() << "Execute: " << *(*j) << std::endl;
        (*j)->execute(*field);
    }

    output.save(*this, input, *field);

}


void MIRJob::print(std::ostream& out) const {
    out << "MIRJob[";
    const char* sep = "";
    for(std::map<std::string, std::string>::const_iterator j = settings_.begin(); j != settings_.end(); ++j) {
        out << sep;
        out << (*j).first << "=" << (*j).second;
        sep = ",";
    }
    out << "]";
}


void MIRJob::set(const std::string& name, const std::string& value) {
    eckit::Log::info() << "************* MIRJob::set [" << name << "] =  [" << value << "]" << std::endl;
    settings_[name] = value;
}


bool MIRJob::get(const std::string& name, std::string& value) const {
    std::map<std::string, std::string>::const_iterator j = settings_.find(name);
    if(j != settings_.end()) {
        value = (*j).second;
        return true;
    }
    return false;
}


bool MIRJob::matches(const MIRParametrisation& metadata) const {

    static const char* force[] = { "bitmap", "frame", "packing", "accuracy", 0 }; // More to add

    for(std::map<std::string, std::string>::const_iterator j = settings_.begin(); j != settings_.end(); ++j) {
        eckit::Log::info() << "Check if " << (*j).first << "=" << (*j).second << " triggers interpolation" << std::endl;
        // Check for keywords that triggers
        size_t i = 0;
        while(force[i]) {
            if((*j).first == force[i]) {
                eckit::Log::info() << "    Yes. (Forced)" << std::endl;
                return false;
            }
            i++;
        }

        // Check if same a field
        std::string value;
        if(metadata.get((*j).first, value)) {
            if(value != (*j).second) {
                eckit::Log::info() << "    Yes. Field is " << value << std::endl;
                return false;
            }
        }

        eckit::Log::info() << "-"<< std::endl;

    }

    return true;
}

