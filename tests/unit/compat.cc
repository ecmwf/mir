// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <cstring>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/grib/Compatibility.h"
#include "mir/grib/OutputKeys.h"
#include "mir/grib/Packing.h"
#include "mir/input/MIRInput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::tests::unit {


using grib::Compatibility;
using grib::OutputKeys;


struct compatibility : param::SimpleParametrisation {
    explicit compatibility(const std::string& name = "") {
        if (!name.empty()) {
            set("compatibility", name);
        }
    }
    const MIRParametrisation& userParametrisation() const override { return *this; }
    const MIRParametrisation& fieldParametrisation() const override { return field_; }

private:
    SimpleParametrisation field_;
};


const OutputKeys::value_type* find(const OutputKeys::keys_type& keys, const std::string& key) {
    for (const auto& [k, v] : keys) {
        if (k == key) {
            return &v;
        }
    }
    return nullptr;
}


const grib_values* find(const grib_info& info, const std::string& key) {
    for (long j = 0; j < info.packing.extra_settings_count; ++j) {
        if (key == info.packing.extra_settings[j].name) {
            return &info.packing.extra_settings[j];
        }
    }
    return nullptr;
}


struct Handle : std::unique_ptr<grib_handle, decltype(&codes_handle_delete)> {
    explicit Handle(const char* sample) :
        unique_ptr(codes_grib_handle_new_from_samples(nullptr, sample), &codes_handle_delete) {
        ASSERT(*this);
    }

    long get_long(const char* key) const {
        long value = 0;
        GRIB_CALL(codes_get_long(get(), key, &value));
        return value;
    }

    std::string get_string(const char* key) const {
        char buffer[64];
        size_t length = sizeof(buffer);
        GRIB_CALL(codes_get_string(get(), key, buffer, &length));
        return buffer;
    }
};


CASE("Compatibility: names") {
    SECTION("none") {
        const Compatibility c{compatibility{}};
        EXPECT(c.empty());

        OutputKeys keys;
        c.keys(1, keys);
        EXPECT(keys.after().empty());

        EXPECT(c.packing_type("spectral_simple") == "spectral_simple");
        EXPECT(c.packing_type(CODES_UTIL_PACKING_TYPE_SPECTRAL_SIMPLE) == CODES_UTIL_PACKING_TYPE_SPECTRAL_SIMPLE);
    }

    SECTION("production's (pgen), alone and combined") {
        for (const auto* name : {"no-local-extension", "adjust-spectral-packing",
                                 "no-local-extension/adjust-spectral-packing"}) {
            const Compatibility c{compatibility{name}};
            EXPECT_NOT(c.empty());
            EXPECT_EQUAL(c.name(), name);
        }
    }

    SECTION("unknown, alone or combined") {
        for (const auto* name : {"something", "no-local-extension/something", "grib1-delete-local-definition",
                                 "spectral-complex-packing", "millidegree-area", "produce-large-constant-fields"}) {
            EXPECT_THROWS_AS(Compatibility{compatibility{name}}, exception::UserError);
        }
    }
}


CASE("Compatibility: no-local-extension") {
    const Compatibility c{compatibility{"no-local-extension"}};

    for (long edition : {1, 2}) {
        OutputKeys keys;
        c.keys(edition, keys);

        const auto* value = find(keys.after(), "deleteLocalDefinition");
        EXPECT((value != nullptr) == (edition == 1));
        if (value != nullptr) {
            EXPECT(std::get<long>(*value) == 1);
        }

        EXPECT(keys.before().empty());
    }

    // the packing type as requested
    EXPECT(c.packing_type("spectral_simple") == "spectral_simple");
}


CASE("Compatibility: adjust-spectral-packing") {
    const Compatibility c{compatibility{"adjust-spectral-packing"}};

    EXPECT(c.packing_type("spectral_simple") == "spectral_complex");
    EXPECT(c.packing_type("spectral_complex") == "spectral_complex");
    EXPECT(c.packing_type("grid_simple") == "grid_simple");

    EXPECT(c.packing_type(CODES_UTIL_PACKING_TYPE_SPECTRAL_SIMPLE) == CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX);
    EXPECT(c.packing_type(CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX) == CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX);
    EXPECT(c.packing_type(CODES_UTIL_PACKING_TYPE_GRID_SIMPLE) == CODES_UTIL_PACKING_TYPE_GRID_SIMPLE);

    // no keys
    OutputKeys keys;
    c.keys(1, keys);
    EXPECT(keys.after().empty());
}


CASE("Compatibility: adjust-spectral-packing, as the packing sets it") {
    param::SimpleParametrisation field;
    field.set("input", "{artificialInput:constant,constant:0.,accuracy:16,truncation:20,edition:2,packing:complex,"
                       "spectral:true}");
    std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build("constant", field));

    for (bool adjust : {false, true}) {
        api::MIRJob job;
        job.set("packing", "simple");
        if (adjust) {
            job.set("compatibility", "adjust-spectral-packing");
        }

        const param::CombinedParametrisation param(job, input->parametrisation());
        std::unique_ptr<grib::Packing> pack(grib::Packing::build(param));

        // as codes_grib_util_set_spec
        grib_info info;
        pack->fill(nullptr, info);
        EXPECT(info.packing.packing == CODES_UTIL_PACKING_USE_PROVIDED);
        EXPECT(info.packing.packing_type ==
               (adjust ? CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX : CODES_UTIL_PACKING_TYPE_SPECTRAL_SIMPLE));

        // as set on the handle
        if (adjust) {
            Handle h("sh_ml_grib2");
            MIRValuesVector values(static_cast<size_t>(h.get_long("numberOfValues")), 0.);
            values[0] = 1.;

            pack->set(nullptr, h.get(), values);
            EXPECT_EQUAL(h.get_string("packingType"), "spectral_complex");
        }
    }
}


CASE("OutputKeys") {
    OutputKeys keys;
    keys.add_before("bitsPerValue", 8L);
    keys.add_after("paramId", 167L);
    keys.add_after("deleteLocalDefinition", 0L);
    keys.add_after("deleteLocalDefinition", 1L);  // replaces
    keys.add_after("missingValue", 9999.);
    keys.add_after("packingType", std::string("grid_simple"));

    EXPECT(keys.before().size() == 1);
    EXPECT(keys.after().size() == 4);
    EXPECT(std::get<long>(*find(keys.after(), "deleteLocalDefinition")) == 1);

    SECTION("set on a handle") {
        for (const auto* sample : {"regular_ll_pl_grib1", "regular_ll_pl_grib2"}) {
            Handle h(sample);
            keys.set_before(h.get());
            keys.set_after(h.get());

            EXPECT(h.get_long("bitsPerValue") == 8);
            EXPECT(h.get_long("paramId") == 167);
            EXPECT(h.get_long("localUsePresent") == 0);
            EXPECT_EQUAL(h.get_string("packingType"), "grid_simple");
        }
    }

    SECTION("as codes_grib_util_set_spec takes them") {
        grib_info info;
        keys.fill(info);

        EXPECT(info.packing.deleteLocalDefinition == 1);
        EXPECT(find(info, "deleteLocalDefinition") == nullptr);

        const auto* bits = find(info, "bitsPerValue");
        EXPECT(bits != nullptr && bits->type == CODES_TYPE_LONG && bits->long_value == 8);

        const auto* id = find(info, "paramId");
        EXPECT(id != nullptr && id->type == CODES_TYPE_LONG && id->long_value == 167);

        const auto* missing = find(info, "missingValue");
        EXPECT(missing != nullptr && missing->type == CODES_TYPE_DOUBLE && missing->double_value == 9999.);

        const auto* packing = find(info, "packingType");
        EXPECT(packing != nullptr && packing->type == CODES_TYPE_STRING &&
               std::strcmp(packing->string_value, "grid_simple") == 0);
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
