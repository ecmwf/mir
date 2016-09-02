/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Pedro Maciel
/// @date Apr 2015


#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/io/StdFile.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"

#include "mir/util/Bitmap.h"

// TODO: Cache bitmaps


namespace mir {
namespace util {
namespace {

static void out(std::vector<std::vector<bool> > &bitmap, long row, const std::string &line, bool on, long &prev) {

    ASSERT(row >= 0);

    if (prev >= 0) {
        for (long i = prev + 1; i < row; i++) {
            bitmap[i] = bitmap[prev];
        }
    }

    prev = row;

    eckit::Translator<std::string, long> s2l;
    eckit::Tokenizer t1("/");
    eckit::Tokenizer t2("-");

    std::vector<std::string> r;


    t1(line, r);


    ASSERT(row >= 0 && row < long(bitmap.size()));
    std::vector<bool> &v = bitmap[row];

    for (size_t i = 0; i < r.size() ; i++) {

        std::vector<std::string> s;
        t2(r[i], s);
        ASSERT(s.size() == 1 || s.size() == 2);
        if (s.size() == 1) {
            s.push_back(s[0]);
        }

        long a = s2l(s[0]) - 1;
        long b = s2l(s[1]) - 1;

        if (a >= 0) {


            ASSERT(a >= 0 && a < long(v.size()));
            ASSERT(b >= 0 && b < long(v.size()));

            for (int j = a; j <= b ; j++) {
                v[j] = on;
            }
        }
    }
}


}  // (unnamed namespace)


Bitmap::Bitmap(const std::string& path):
    path_(path),
    width_(0),
    height_(0) {

    std::vector<std::string> v;
    eckit::Tokenizer parse(":");
    parse(path, v);

    if(v.size() == 3) {
        prodgenBitmap(v[0], v[1], v[2]);
    }
    else {
        disseminationBitmap(path);
    }
}

void Bitmap::disseminationBitmap(const std::string& path) {

    eckit::StdFile file(path);
    int c;
    std::string s;

    while ( (c = fgetc(file)) != EOF) {

        switch (c) {

        case ' ':
        case '\t':
        case '\n':
        case '\r':
            break;

        default:
            s += char(tolower(c));
            break;

        }
    }

    bool on = s.find("values=on") == s.npos;

    size_t pos = s.find("size=");
    ASSERT(pos != s.npos);
    pos += 5;
    int  n = 0;
    while (pos < s.size()) {
        if (s[pos] == ':') {
            height_ = n;
            n = 0;
            pos++;
            continue;
        }

        if (s[pos] == ',') {
            break;
        }

        ASSERT(isdigit(s[pos]));
        n = 10 * n + (s[pos] - '0');
        pos++;
    }
    width_ = n;

    ASSERT(width_);
    ASSERT(height_);

    pos = s.find("points=");
    ASSERT(pos != s.npos);
    pos += 7;

    std::vector<std::vector<bool> > bitmap(height_, std::vector<bool>(width_, !on));

    std::string t;
    long row = -1;
    long prev = -1;
    eckit::Translator<std::string, long> s2l;

    while (pos < s.size()) {

        if (s[pos] == ':') {
            row = s2l(t) - 1;
            t = "";
            pos++;
            continue;
        }

        if (s[pos] == ',') {
            out(bitmap, row, t, on, prev);
            t = "";
            pos++;
            continue;
        }

        if (isalpha(s[pos])) {
            break;
        }

        t += s[pos];
        pos++;
    }

    out(bitmap, row, t, on, prev);
    out(bitmap, height_ - 1, t, on, prev);

    // out(bitmap);

    std::swap(bitmap, bitmap_);
}

void Bitmap::prodgenBitmap(const std::string& path, const std::string& destination, const std::string& number)
{
    char  line[1024];
    std::ifstream in(path.c_str());
    if (!in) {
        throw eckit::CantOpenFile(path);
    }

    eckit::Translator<std::string, size_t> s2l;

    size_t no = s2l(number);

    while (in.getline(line, sizeof(line))) {

        std::string dest(line, line + 5);
        size_t num = s2l(std::string(line + 6, line + 9));
        bool ok = (num == no && destination == dest);

        size_t size = 0;
        for (const char *c = line + 10; c != line + 20; ++c) {
            if (::isdigit(*c)) {
                size *= 10;
                size += *c - '0';
            }
        }

        size_t len = 0;


        std::vector<bool> bitmap(size);
        bitmap.reserve(size);

        size_t k = 0;
        for (;;) {
            in.getline(line, sizeof(line));
            len += strlen(line);

            if(ok) {
                const char *p = line;

                while (*p) {
                    bitmap[k++] = (*p == '1');
                    p++;
                }
             }

            if (len >= size) {
                break;
            }
        }

        if(ok)
        {
            height_ = 1;
            width_ = bitmap.size();
            bitmap_.resize(1);
            std::swap(bitmap_[0], bitmap);
            return;
        }
    }

    std::ostringstream oss;
    oss << "Cannot find bitmap " << no << " for destination " << destination << " in " << path;
    throw eckit::UserError(oss.str());
}

Bitmap::~Bitmap() {
}


void Bitmap::print(std::ostream &out) const {
    out << "Bitmap[path=" << path_ << "]";
}


}  // namespace util
}  // namespace mir

