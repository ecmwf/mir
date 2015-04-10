// File Bitmap.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/util/Bitmap.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/io/StdFile.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"

// TODO: Cache bitmaps

static void out(const std::vector<std::vector<bool> > &bitmap) {
    for (int i = 0; i < bitmap.size() ; i++ ) {
        const std::vector<bool> &v = bitmap[i];
        for (int j = 0; j < v.size() ; j++ ) {
            std::cout << (v[j] ? '1' : '0');
        }
        std::cout << std::endl;
    }
}

static void out(std::vector<std::vector<bool> > &bitmap, int row, const std::string &line, bool on, long &prev) {
    
    ASSERT(row >= 0);

    if (prev >= 0) {
        for (size_t i = prev + 1; i < row; i++) {
            bitmap[i] = bitmap[prev];
        }
    }

    prev = row;

    eckit::Translator<std::string, long> s2l;
    eckit::Tokenizer t1("/");
    eckit::Tokenizer t2("-");

    std::vector<std::string> r;


    t1(line, r);

    // std::cout << row << std::endl;

    ASSERT(row >= 0 && row < bitmap.size());
    std::vector<bool> &v = bitmap[row];

    for (int i = 0; i < r.size() ; i++) {
        // std::cout << "  ===> " << r[i] << std::endl;

        std::vector<std::string> s;
        t2(r[i], s);
        ASSERT(s.size() == 1 || s.size() == 2);
        if (s.size() == 1) {
            s.push_back(s[0]);
        }

        long a = s2l(s[0]) - 1;
        long b = s2l(s[1]) - 1;

        if (a >= 0) {

            // std::cout << a << " - " << b <<  std::endl;

            ASSERT(a >= 0 && a < v.size());
            ASSERT(b >= 0 && b < v.size());

            for (int j = a; j <= b ; j++) {
                v[j] = on;
            }
        }
    }


}

Bitmap::Bitmap(const eckit::PathName& path):
    path_(path),
    width_(0),
    height_(0)
{

    eckit::StdFile file(path_);
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

    std::cout << s << std::endl;

    bool on = s.find("values=on") == s.npos;

    int pos = s.find("size="); ASSERT(pos != s.npos);
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

    // std::cout << "w=" << w << ",h=" << h << std::endl;

    pos = s.find("points="); ASSERT(pos != s.npos);
    pos += 7;

    // std::cout << "ON " << on << std::endl;
    std::vector<std::vector<bool> > bitmap(height_, std::vector<bool>(width_, !on));
    // out(bitmap);

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

    out(bitmap);

    std::swap(bitmap, bitmap_);

}

Bitmap::~Bitmap() {
}

void Bitmap::print(std::ostream &out) const {
    out << "Bitmap[path=" << path_ << "]";
}
