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


#include "mir/output/PNGOutput.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <utility>

#define PNG_DEBUG 3
#include "png.h"

#include "eckit/config/Resource.h"
#include "eckit/filesystem/PathName.h"

#include "mir/action/context/Context.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/repres/latlon/LatLon.h"
#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Mutex.h"
#include "mir/util/Trace.h"


namespace mir {
namespace output {

constexpr int N_BITS_PER_BYTE = 8;

void call_zero(int bad, const std::string& msg) {
    if (bad != 0) {
        Log::error() << "PNGOutput: " << msg << " failed" << std::endl;
        throw exception::SeriousBug(msg);
    }
}

void call_nonzero(void* ok, const std::string& msg) {
    call_zero(ok == nullptr ? 1 : 0, msg);
}

struct PNGOutput::PNGEncoder {
    PNGEncoder()          = default;
    virtual ~PNGEncoder() = default;

    PNGEncoder(const PNGEncoder&) = delete;
    PNGEncoder& operator=(const PNGEncoder&) = delete;

    virtual void encode(png_bytep&, const double&) const = 0;
    virtual int bit_depth() const                        = 0;
    virtual int color_type() const                       = 0;
};

PNGOutput::PNGOutput(std::string path) : path_(std::move(path)) {}

PNGOutput::~PNGOutput() = default;

size_t PNGOutput::save(const param::MIRParametrisation& param, context::Context& ctx) {
    trace::ResourceUsage usage("PNGOutput::save");
    auto timing(ctx.statistics().saveTimer());

    const auto& field = ctx.field();
    field.validate();

    repres::RepresentationHandle rep(field.representation());

    atlas::RegularGrid grid(rep->atlasGrid());
    if (!grid) {
        throw exception::UserError("PNGOutput: field should be on a regular grid");
    }
    auto Ni = size_t(grid.nx());
    auto Nj = size_t(grid.ny());

    ASSERT(Ni && Nj);
    ASSERT(Ni * Nj == rep->numberOfPoints());

    // setup encoder
    std::unique_ptr<const PNGEncoder> encoder(PNGEncoderFactory::build(param, field));
    ASSERT(encoder);

    for (size_t d = 0; d < field.dimensions(); d++) {
        auto& values = field.values(d);

        eckit::PathName path(path_);
        for (size_t counter = 1; path.exists(); ++counter) {
            std::ostringstream name;
            name << path_ << "." << std::setw(4) << std::setfill('0') << counter;
            path = name.str();
        }

        Log::debug() << "PNGOutput: writing to '" + path.asString() + "'" << std::endl;

        // initialize
        FILE* fp;
        png_structp png_ptr;
        png_infop info_ptr;

        call_nonzero(fp = std::fopen(path.asString().c_str(), "wb"), "writing to '" + path.asString() + "'");
        call_nonzero(png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr),
                     "png_create_write_struct");
        call_nonzero(info_ptr = png_create_info_struct(png_ptr), "png_create_info_struct");
        call_zero(setjmp(png_jmpbuf(png_ptr)), "init_io");
        png_init_io(png_ptr, fp);


        // write header
        call_zero(setjmp(png_jmpbuf(png_ptr)), "write header");
        png_set_IHDR(png_ptr, info_ptr, Ni, Nj, encoder->bit_depth(), encoder->color_type(), PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        //        png_color_16 trans;
        //        if (hasMissing_) {
        //            trans.gray =
        //            png_set_tRNS(png_ptr, info_ptr, nullptr, 0, &trans);
        //        }

        png_write_info(png_ptr, info_ptr);
        call_zero(setjmp(png_jmpbuf(png_ptr)), "write header end");


        // write content
        {
            auto size = png_get_rowbytes(png_ptr, info_ptr);
            auto row  = static_cast<png_bytep>(png_malloc(png_ptr, size));

            for (size_t j = 0; j < Nj; ++j) {
                png_bytep p = row;
                for (size_t i = 0; i < Ni; ++i) {
                    encoder->encode(p, values[j * Ni + i]);
                }

                png_write_row(png_ptr, row);
            }
            png_free(png_ptr, row);

            call_zero(setjmp(png_jmpbuf(png_ptr)), "write content end");
            png_write_end(png_ptr, nullptr);
        }

        png_destroy_info_struct(png_ptr, &info_ptr);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        std::fclose(fp);
    }


    return 1;
}

bool PNGOutput::sameAs(const MIROutput& other) const {
    return this == &other;
}

bool PNGOutput::sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const {
    return true;
}

bool PNGOutput::printParametrisation(std::ostream&, const param::MIRParametrisation&) const {
    return false;
}

void PNGOutput::print(std::ostream& out) const {
    out << "PNGOutput[path=" << path_ << "]";
}

static const MIROutputBuilder<PNGOutput> output1("png", {".png"});

static util::once_flag once;
static util::recursive_mutex* local_mutex           = nullptr;
static std::map<std::string, PNGEncoderFactory*>* m = nullptr;

static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, PNGEncoderFactory*>();
}

PNGEncoderFactory::PNGEncoderFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("PNGEncoderFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}

PNGEncoderFactory::~PNGEncoderFactory() = default;

const PNGOutput::PNGEncoder* PNGEncoderFactory::build(const param::MIRParametrisation& param,
                                                      const data::MIRField& field) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    std::string name;
    param.get("png-output-encoder", name = "8-bit/g");

    if (field.hasMissing() && name.back() != 'a') {
        name += 'a';
    }

    Log::debug() << "PNGEncoderFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "PNGEncoderFactory: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("PNGEncoderFactory: unknown '" + name + "'");
    }

    return j->second->make(param, field);
}

void PNGEncoderFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}

template <int N_C_CHANNELS, int N_A_CHANNELS, int N_BYTES_PER_CHANNEL, typename UINT_T>
struct PNGEncoderT : PNGOutput::PNGEncoder {

    PNGEncoderT(const param::MIRParametrisation& param, const data::MIRField& field) :
        missingValue_(field.missingValue()), hasMissing_(field.hasMissing()) {
        ASSERT(N_C_CHANNELS == 1 || N_C_CHANNELS == 3);
        ASSERT(N_A_CHANNELS == 0 || N_A_CHANNELS == 1);

        std::vector<double> minmax;
        if (param.get("png-output-minmax", minmax)) {
            ASSERT(minmax.size() == 2);
            min_ = minmax[0];
            max_ = minmax[1];
        }
        else {
            min_ = std::numeric_limits<double>::signaling_NaN();
            max_ = std::numeric_limits<double>::signaling_NaN();

            bool first = true;
            for (size_t i = 0; i < field.dimensions(); i++) {
                for (auto& v : field.values(i)) {
                    if (hasMissing_ && v == missingValue_) {
                        continue;
                    }
                    if (min_ > v || first) {
                        min_ = v;
                    }
                    if (max_ < v || first) {
                        max_ = v;
                    }
                    first = false;
                }
            }

            if (!std::isfinite(min_) || !std::isfinite(max_)) {
                min_ = max_ = missingValue_;
            }
        }

        maxEncode_ = (UINT_T(1) << N_C_CHANNELS * N_BYTES_PER_CHANNEL * N_BITS_PER_BYTE) - 1;
        maxScale_  = min_ < max_ ? double(maxEncode_) / (max_ - min_) : 1.;

        Log::debug() << "PNGEncoder: min/max = " << min_ << " / " << max_ << std::endl;
    }

    void encode(png_bytep& p, const double& value) const override {
        constexpr int FF = 0xFF;

        if (hasMissing_ && value == missingValue_) {
            // set both colour and alpha channels
            for (int i = 0; i < (N_C_CHANNELS + N_A_CHANNELS) * N_BYTES_PER_CHANNEL; ++i) {
                *(p++) = 0x00;
            }
            return;
        }

        // set colour channels
        UINT_T colour = value <= min_ ? 0 : value >= max_ ? maxEncode_ : UINT_T((value - min_) * maxScale_);
        UINT_T mask(UINT_T(FF) << N_BITS_PER_BYTE * (N_C_CHANNELS * N_BYTES_PER_CHANNEL - 1));

        for (int byte = N_C_CHANNELS * N_BYTES_PER_CHANNEL - 1; byte >= 0; --byte, mask = (mask >> N_BITS_PER_BYTE)) {
            *(p++) = png_byte((colour & mask) >> (byte * N_BITS_PER_BYTE));
        }

        // set alpha channel
        if (N_A_CHANNELS != 0) {
            for (int i = 0; i < N_BYTES_PER_CHANNEL; ++i) {
                *(p++) = FF;
            }
        }
    }

    int bit_depth() const override { return N_BYTES_PER_CHANNEL * N_BITS_PER_BYTE; }

    int color_type() const override {
        return N_C_CHANNELS == 1   ? (N_A_CHANNELS != 0 ? PNG_COLOR_TYPE_GRAY_ALPHA : PNG_COLOR_TYPE_GRAY)
               : N_C_CHANNELS == 3 ? (N_A_CHANNELS != 0 ? PNG_COLOR_TYPE_RGB_ALPHA : PNG_COLOR_TYPE_RGB)
                                   : NOTIMP;
    }

private:
    double missingValue_;
    double min_;
    double max_;
    double maxScale_;
    UINT_T maxEncode_;
    const bool hasMissing_;
};

static const PNGEncoderBuilder<PNGEncoderT<1, 0, 1, uint_fast8_t>> encoder1("8-bit/g");
static const PNGEncoderBuilder<PNGEncoderT<1, 1, 1, uint_fast8_t>> encoder2("8-bit/ga");
static const PNGEncoderBuilder<PNGEncoderT<1, 0, 2, uint_fast16_t>> encoder3("16-bit/g");
static const PNGEncoderBuilder<PNGEncoderT<1, 1, 2, uint_fast16_t>> encoder4("16-bit/ga");
static const PNGEncoderBuilder<PNGEncoderT<3, 0, 1, uint_fast32_t>> encoder5("8-bit/rgb");
static const PNGEncoderBuilder<PNGEncoderT<3, 1, 1, uint_fast32_t>> encoder6("8-bit/rgba");
static const PNGEncoderBuilder<PNGEncoderT<3, 0, 2, uint_fast64_t>> encoder7("16-bit/rgb");
static const PNGEncoderBuilder<PNGEncoderT<3, 1, 2, uint_fast64_t>> encoder8("16-bit/rgba");

}  // namespace output
}  // namespace mir
