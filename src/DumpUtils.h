#pragma once

#include <cstdint>
#include <ios>
#include <iomanip>
#include <ostream>

#define NBSP "&nbsp;"
extern const char szThousandSpaces[1001];

class FormatAutoRestore {
    std::streamsize _width, _precision;
    std::ios_base::fmtflags _flags;
    char _fill;
    std::ostream *sptr;

public:
    FormatAutoRestore(std::ostream &s) : _width(s.width()), _precision(s.precision()), _flags(s.flags()),
                                         _fill(s.fill()), sptr(&s) {
    }

    ~FormatAutoRestore() {
        sptr->setf(_flags);
        sptr->fill(_fill);
        sptr->width(_width);
        sptr->precision(_precision);
    }
};

const char *GetIndentStr(uint32_t indent);
const char *GetIndentStrEx(uint32_t indent, bool for_html);
std::ostream &DumpNumber(std::ostream &s, uint64_t n, int w);
std::ostream &DumpTime(std::ostream &s, uint64_t n, int w);
std::ostream &DumpDateTimeElapsed(std::ostream &s, uint64_t n, int w);
