// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/Grib.h"

#include <algorithm>
#include <cstring>
#include <ios>
#include <sstream>
#include <utility>

#include "eckit/config/Resource.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


bool grib_call(int e, const char* call, bool NOT_FOUND_IS_OK) {
    if (static_cast<bool>(e)) {
        if (NOT_FOUND_IS_OK && (e == CODES_NOT_FOUND)) {
            return false;
        }

        std::ostringstream os;
        os << call << ": " << codes_get_error_message(e);
        throw mir::exception::SeriousBug(os.str());
    }
    return true;
}


namespace {


enum ScanningMode : long
{
    iScansNegatively       = 1 << 7,
    jScansPositively       = 1 << 6,
    jPointsAreConsecutive  = 1 << 5,
    alternativeRowScanning = 1 << 4
};


// position in canonical order (scanningMode=0) of each value in the given order
std::vector<size_t> grib_canonical_index(const std::string& order, size_t Ni, size_t Nj) {
    const auto scanningMode = grib_order_to_scanning_mode(order);
    if ((scanningMode & alternativeRowScanning) != 0) {
        throw mir::exception::SeriousBug("grib_reorder: order '" + order + "' not supported");
    }

    ASSERT(Ni > 0);
    ASSERT(Nj > 0);

    const auto iNegative = (scanningMode & iScansNegatively) != 0;
    const auto jPositive = (scanningMode & jScansPositively) != 0;

    auto canonical = [=](size_t i, size_t j) {
        return (jPositive ? Nj - 1 - j : j) * Ni + (iNegative ? Ni - 1 - i : i);
    };

    std::vector<size_t> index;
    index.reserve(Ni * Nj);

    if ((scanningMode & jPointsAreConsecutive) != 0) {
        for (size_t i = 0; i < Ni; ++i) {
            for (size_t j = 0; j < Nj; ++j) {
                index.emplace_back(canonical(i, j));
            }
        }
    }
    else {
        for (size_t j = 0; j < Nj; ++j) {
            for (size_t i = 0; i < Ni; ++i) {
                index.emplace_back(canonical(i, j));
            }
        }
    }

    return index;
}


}  // namespace


long grib_order_to_scanning_mode(const std::string& order) {
    if (order.empty()) {
        throw mir::exception::SeriousBug("grib_order_to_scanning_mode: empty order");
    }

    return ((order.find("i+-") != std::string::npos || order.find("i-+") != std::string::npos) ? alternativeRowScanning
                                                                                               : 0) |
           (order.front() == 'j' ? jPointsAreConsecutive : 0) |
           (order.find("j+") != std::string::npos ? jScansPositively : 0) |
           (order.find("i-") != std::string::npos ? iScansNegatively : 0);
}


void grib_reorder_to_canonical(std::vector<double>& values, const std::string& order, size_t Ni, size_t Nj) {
    if (grib_order_to_scanning_mode(order) == 0) {
        return;
    }

    mir::Log::warning() << "grib_reorder: order '" << order << "' to canonical" << std::endl;

    const auto index = grib_canonical_index(order, Ni, Nj);
    ASSERT(values.size() == index.size());

    std::vector<double> out(values.size());
    for (size_t k = 0; k < index.size(); ++k) {
        out[index[k]] = values[k];
    }

    values.swap(out);
}


void grib_reorder_from_canonical(std::vector<double>& values, const std::string& order, size_t Ni, size_t Nj) {
    if (grib_order_to_scanning_mode(order) == 0) {
        return;
    }

    const auto index = grib_canonical_index(order, Ni, Nj);
    ASSERT(values.size() == index.size());

    std::vector<double> out(values.size());
    for (size_t k = 0; k < index.size(); ++k) {
        out[k] = values[index[k]];
    }

    values.swap(out);
}


void grib_get_unique_missing_value(const std::vector<double>& values, double& missingValue) {
    ASSERT(!values.empty());

    // check if it's unique, otherwise a high then a low value
    if (std::find(values.begin(), values.end(), missingValue) == values.end()) {
        return;
    }

    auto mm = std::minmax_element(values.begin(), values.end());

    missingValue = *(mm.second) + 1.;
    if (missingValue == missingValue) {
        return;
    }

    missingValue = *(mm.first) - 1.;
    if (missingValue == missingValue) {
        return;
    }

    throw mir::exception::SeriousBug("grib_get_unique_missing_value: failed to get a unique missing value.");
}


grib_info::grib_info() :
    grid{}, packing{}, extra_settings_size_(sizeof(packing.extra_settings) / sizeof(packing.extra_settings[0])) {
    // NOTE low-level initialisation only necessary for C interface
    std::memset(&grid, 0, sizeof(grid));
    std::memset(&packing, 0, sizeof(packing));

    strings_.reserve(extra_settings_size_);
}


void grib_info::extra_set(const char* key, long value) {
    auto j = static_cast<size_t>(packing.extra_settings_count++);
    ASSERT(j < extra_settings_size_);

    auto& set      = packing.extra_settings[j];
    set.name       = key;
    set.type       = CODES_TYPE_LONG;
    set.long_value = value;
}


void grib_info::extra_set(const char* key, double value) {
    auto j = static_cast<size_t>(packing.extra_settings_count++);
    ASSERT(j < extra_settings_size_);

    auto& set        = packing.extra_settings[j];
    set.name         = key;
    set.type         = CODES_TYPE_DOUBLE;
    set.double_value = value;
}


void grib_info::extra_set(const char* key, const char* value) {
    auto j = static_cast<size_t>(packing.extra_settings_count++);
    ASSERT(j < extra_settings_size_);

    auto& set = packing.extra_settings[j];
    set.name  = key;
    set.type  = CODES_TYPE_STRING;

    strings_.emplace_back(value);
    set.string_value = strings_.back().c_str();
}


HandleDeleter::HandleDeleter(grib_handle* h) : h_(h) {
    ASSERT(h);
}


bool grib_check_is_message_valid() {
    static bool check = eckit::Resource<bool>("$MIR_GRIB_CHECK_IS_MESSAGE_VALID", false);
    return check;
}
