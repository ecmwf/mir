// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>
#include <vector>


namespace mir::util {


class Bitmap {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit Bitmap(const std::string&);

    // -- Destructor

    ~Bitmap();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    size_t width() const { return width_; }

    size_t height() const { return height_; }

    bool on(size_t j, size_t i) const { return bitmap_[j][i]; }

    size_t footprint() const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    Bitmap(const Bitmap&);
    Bitmap& operator=(const Bitmap&);

    // -- Members

    std::string path_;
    std::vector<std::vector<bool> > bitmap_;
    size_t width_;
    size_t height_;

    // -- Methods

    void disseminationBitmap(const std::string& path);
    void prodgenBitmap(const std::string& path, const std::string& destination, const std::string& number);

    // -- Overridden methods

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Bitmap& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::util
