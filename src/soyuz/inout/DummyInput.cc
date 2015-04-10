// File DummyInput.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "DummyInput.h"
#include "soyuz/util/MIRField.h"

#include <cmath>
#include <iostream>


DummyInput::DummyInput() {
    settings_["gridType"] = "regular_ll";
    settings_["north"] = "50";
    settings_[ "west"] = "0";
    settings_["south"] = "0";
    settings_["east"] = "50";
    settings_["north_south_increment"] = "1";
    settings_["west_east_increment"] = "1";
}

DummyInput::~DummyInput() {}


const MIRParametrisation &DummyInput::parametrisation() const {
    return *this;
}

MIRField *DummyInput::field() const {
    MIRField *field = new MIRField(false, 999.);

    std::vector< double > values(51 * 51, 42);
    size_t k = 0;
    for (size_t i = 0; i < 51; ++i)
        for (size_t j = 0; j < 51; ++j) {
            values[k++] = sin(double(i) / 10.) + cos(double(j) / 10.);
        }
    field->values(values);

    return field;
}

void DummyInput::print(std::ostream &out) const {
    out << "DummyInput[...]";
}

bool DummyInput::lowLevelGet(const std::string &name, std::string &value) const {
    std::map<std::string, std::string>::const_iterator j = settings_.find(name);
    if (j != settings_.end()) {
        value = (*j).second;
        return true;
    }
    return false;
}
