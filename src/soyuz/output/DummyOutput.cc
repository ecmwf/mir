// File DummyOutput.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/output/DummyOutput.h"

#include <istream>


DummyOutput::DummyOutput() {
}

DummyOutput::~DummyOutput() {
}


void DummyOutput::copy(const MIRParametrisation &param, MIRInput &input) {

}

void DummyOutput::save(const MIRParametrisation &param, MIRInput &input, MIRField &field) {
}

void DummyOutput::print(std::ostream &out) const {
    out << "DummyOutput[...]";
}
