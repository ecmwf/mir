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


#include "array.h"

#include <algorithm>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "eckit/geo/Exceptions.h"
#include "eckit/geo/Grid.h"
#include "eckit/log/JSON.h"

#include "mir/action/context/Context.h"
#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Grib.h"


ArrayInput::ArrayInput(PyObject* values, PyObject* gridspec) : values_(values), gridspec_(gridspec) {
    Py_INCREF(values_);
    Py_INCREF(gridspec_);

    if (!PyUnicode_Check(gridspec_)) {
        throw std::runtime_error("ArrayInput: gridspec expects a string.");
    }

    param_ = std::make_unique<mir::param::GridSpecParametrisation>(std::string{PyUnicode_AsUTF8(gridspec_)});
    ASSERT(param_);

    if (PyObject_GetBuffer(values_, &buffer_, PyBUF_CONTIG_RO | PyBUF_FORMAT) == -1) {
        throw std::runtime_error("ArrayInput: Failed to get buffer.");
    }

    if (strcmp(buffer_.format, "d") == 0) {
        auto size = static_cast<size_t>(buffer_.len / sizeof(double));
        input_    = std::make_unique<mir::input::RawInput>(static_cast<double*>(buffer_.buf), size, *param_);
    }
    else if (strcmp(buffer_.format, "f") == 0) {
        converted_.resize(buffer_.len / sizeof(float));
        auto* src = static_cast<float*>(buffer_.buf);
        std::copy(src, src + converted_.size(), converted_.begin());

        input_ = std::make_unique<mir::input::RawInput>(converted_.data(), converted_.size(), *param_);
    }
    else {
        PyBuffer_Release(&buffer_);
        throw std::runtime_error("ArrayInput: Unsupported format. Expected 'd' or 'f'.");
    }
    ASSERT(input_);
}


ArrayInput::~ArrayInput() {
    PyBuffer_Release(&buffer_);
    Py_DECREF(values_);
    Py_DECREF(gridspec_);
}


mir::data::MIRField ArrayInput::field() const {
    return input().field();
}


void ArrayInput::print(std::ostream& out) const {
    out << "ArrayInput[" << *input_ << "]";
}


size_t ArrayOutput::save(const mir::param::MIRParametrisation&, mir::context::Context& ctx) {
    const auto& field = ctx.field();

    // save values
    ASSERT(field.dimensions() == 1);
    field.validate();
    values_ = field.values(0);

    // save gridspec (a hack)
    mir::api::MIRJob job;
    mir::repres::RepresentationHandle(field.representation())->fillJob(job);

    std::ostringstream spec;
    eckit::JSON j(spec);
    j << job;

    std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(spec.str()));
    ASSERT(grid);

    shape_    = grid->shape();
    gridspec_ = grid->spec_str();

    return 1;
}


void ArrayOutput::print(std::ostream& out) const {
    out << "ArrayOutput[#values=" << values_.size() << ",shape=[";
    const auto* sep = "";
    for (auto s : shape_) {
        out << sep << s;
        sep = ",";
    }
    out << "],gridspec=[" << gridspec_ << "]]";
}
