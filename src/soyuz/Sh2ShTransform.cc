// File Sh2ShTransform.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "Sh2ShTransform.h"
#include "MIRParametrisation.h"
#include "MIRField.h"
#include "Representation.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"


Sh2ShTransform::Sh2ShTransform(const MIRParametrisation &parametrisation):
    Action(parametrisation),
    truncation_(0) {
    std::string value;
    ASSERT(parametrisation.get("user.resol", value));

    truncation_ = eckit::Translator<std::string, size_t>()(value);
}

Sh2ShTransform::~Sh2ShTransform() {
}

void Sh2ShTransform::print(std::ostream &out) const {
    out << "Sh2ShTransform[";
    out << "truncation=" << truncation_;
    out << "]";
}

void Sh2ShTransform::execute(MIRField &field) const {
    const std::vector<double> &values = field.values();
    std::vector<double> result;

    const Representation *representation = field.representation();
    Representation *repres = representation->truncate(truncation_, values, result);

    if (repres) { // NULL if nothing happend
        field.representation(repres);
        field.values(result);
    }
}

static ActionBuilder<Sh2ShTransform> subSh2ShTransform("transform.sh2sh");
