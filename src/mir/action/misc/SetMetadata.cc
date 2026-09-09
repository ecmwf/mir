// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/misc/SetMetadata.h"

#include <ostream>

#include "eckit/types/Types.h"
#include "eckit/utils/Tokenizer.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Translator.h"


namespace mir::action {


SetMetadata::SetMetadata(const param::MIRParametrisation& param) : Action(param) {
    std::string metadata;
    ASSERT(parametrisation().get("metadata", metadata));

    // TODO: create a parser
    eckit::Tokenizer parse1(",");
    eckit::Tokenizer parse2("=");

    std::vector<std::string> v;
    parse1(metadata, v);

    for (auto& j : v) {
        std::vector<std::string> w;
        parse2(j, w);
        ASSERT(w.size() == 2);

        metadata_[w[0]] = util::from_string<long>(w[1]);
    }
}


bool SetMetadata::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const SetMetadata*>(&other);
    return (o != nullptr) && (metadata_ == o->metadata_);
}


void SetMetadata::print(std::ostream& out) const {
    out << "SetMetadata[" << metadata_ << "]";
}


void SetMetadata::execute(context::Context& ctx) const {
    data::MIRField& field = ctx.field();
    for (size_t i = 0; i < field.dimensions(); i++) {
        field.metadata(i, metadata_);
    }
}


const char* SetMetadata::name() const {
    return "SetMetadata";
}


static const ActionBuilder<SetMetadata> __action("set.metadata");


}  // namespace mir::action
