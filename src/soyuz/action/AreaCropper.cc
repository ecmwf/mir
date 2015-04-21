// File AreaCropper.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/action/AreaCropper.h"
#include "soyuz/param/MIRParametrisation.h"
#include "soyuz/data/MIRField.h"
#include "soyuz/repres/Representation.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"


AreaCropper::AreaCropper(const MIRParametrisation& parametrisation):
    Action(parametrisation),
    north_(0),
    west_(0),
    south_(0),
    east_(0) {
    std::string value;
    ASSERT(parametrisation.get("user.area", value));

    std::vector<std::string> result;
    eckit::Tokenizer parse("/");

    parse(value, result);
    ASSERT(result.size() == 4);

    north_ = eckit::Translator<std::string, double>()(result[0]);
    west_ = eckit::Translator<std::string, double>()(result[1]);
    south_ = eckit::Translator<std::string, double>()(result[2]);
    east_ = eckit::Translator<std::string, double>()(result[3]);
}

AreaCropper::~AreaCropper() {
}

void AreaCropper::print(std::ostream& out) const {
    out << "AreaCropper[";
    out << "north=" << north_;
    out << ",west=" << west_;
    out << ",south=" << south_;
    out << ",east=" << east_;
    out << "]";
}

void AreaCropper::execute(MIRField& field) const {
    const std::vector<double> &values = field.values();
    std::vector<double> result;

    const Representation* representation = field.representation();
    Representation* cropped = representation->crop(north_, west_, south_, east_, values, result);

    if(cropped) { // NULL if nothing happend
        field.representation(cropped);
        field.values(result);
    }
}

static ActionBuilder<AreaCropper> subAreaCropper("crop.area");
