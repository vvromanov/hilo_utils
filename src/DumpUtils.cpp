#include "DumpUtils.h"

#define TEN_SPACES "          "
#define HUNDRED_SPACES TEN_SPACES TEN_SPACES TEN_SPACES TEN_SPACES TEN_SPACES TEN_SPACES TEN_SPACES TEN_SPACES TEN_SPACES TEN_SPACES
#define THOUSAND_SPACES HUNDRED_SPACES HUNDRED_SPACES HUNDRED_SPACES HUNDRED_SPACES HUNDRED_SPACES HUNDRED_SPACES HUNDRED_SPACES HUNDRED_SPACES HUNDRED_SPACES HUNDRED_SPACES

const char szThousandSpaces[1001] = THOUSAND_SPACES;


const char szHtmlIndent[] =
        NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP
                NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP
                NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP
                NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP
                NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP
                NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP
                NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP
                NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP
                NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP
                NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP NBSP;

#define MAX_HTML_INDENT ((sizeof(szHtmlIndent)-1)/(sizeof(NBSP)-1))

const char *GetIndentStr(uint32_t indent) {
    const char *res = szThousandSpaces + sizeof(szThousandSpaces) - 1;
    res -= indent;
    if (res < szThousandSpaces) {
        return szThousandSpaces;
    }
    return res;
}

const char *GetIndentStrEx(uint32_t indent, bool for_html) {
    if (for_html) {
        const char *res = szHtmlIndent + sizeof(szHtmlIndent) - 1;
        res -= (indent * (sizeof(NBSP) - 1));
        if (res < szHtmlIndent) {
            return szHtmlIndent;
        }
        return res;
    } else {
        return GetIndentStr(indent);
    }
}

static int NumberW(int64_t n) {
    int w = 0;
    if (n == 0) {
        return 1;
    }
    if (n < 0) {
        ++w;
        n = -n;
    }
    while (n) {
        ++w;
        n /= 10;
    }
    return w;
}

static char getSymbol(uint64_t mul) {
    switch (mul) {
        case 1000ULL:
            return 'k';
        case 1000'000ULL:
            return 'M';
        case 1000'000'000ULL:
            return 'G';
        case 1000'000'000'000ULL:
            return 'T';
        case 1000'000'000'000'000ULL:
            return 'P';
        case 1000'000'000'000'000'000ULL:
            return 'E';
        default:
            return '?';
    }
}

std::ostream &DumpNumber(std::ostream &s, uint64_t n, int w) {
    int nw = NumberW(n);
    if (w >= nw || w < 4) {
        s << std::setw(w) << n;
        return s;
    }
    uint64_t mult = 1;
    --w;
    while (nw > w) {
        mult *= 1000;
        nw = NumberW(n / mult);
    }
    if (w - nw == 1) {
        s << ' ';
    }
    s << n / mult;
    if (w - nw == 2) {
        s << '.' << n / (mult / 10) % 10;
    }
    s << getSymbol(mult);
    return s;
}

std::ostream &DumpTime(std::ostream &s, uint64_t n, int w) {
    uint64_t connected_time_h = n / 60 / 60;
    int hw = NumberW(connected_time_h);
    if (hw + 2 >= w) {
        return DumpNumber(s, connected_time_h, w);
    }
    if (hw + 6 <= w) {
        uint64_t connected_time_m = n % (60 * 60) / 60;
        uint64_t connected_time_s = n % 60;
        s << std::setw(w - 6) << connected_time_h << ':' << std::setw(2) << std::setfill('0') << connected_time_m
          << ':' << std::setw(2) << std::setfill('0') << connected_time_s;
    } else {
        uint64_t connected_time_m = n % (60 * 60) / 60;
        s << std::setw(w - 3) << connected_time_h << ':' << std::setw(2) << std::setfill('0') << connected_time_m;
    }
    s << std::setfill(' ');
    return s;
}
