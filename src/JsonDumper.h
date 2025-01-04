#pragma once
#include <cmath>
#include <cstdint>
#include <functional>
#include <ostream>
#include <stack>

const char* GetIndentStr(uint32_t indent);

class JsonDumper {
    std::ostream& s;
    int indent { 0 };
    bool member_started { false };
    struct state_s {
        bool in_array { false };
        bool first_element { true };
        bool wrap { false };
    } state_s;
    std::stack<struct state_s> state_storage;
    struct state_s state;

public:
    JsonDumper(std::ostream& _s)
        : s { _s }
    {
    }
    void SetWrap(bool _wrap = true) { state.wrap = _wrap; }

    void NewLine()
    {
        if (!state.first_element && state.in_array) {
            s << ",";
        }
        if (!state.wrap) {
            s << std::endl;
        }
        state.first_element = true;
    }

    JsonDumper& StartObject(bool is_array = false)
    {
        NextItem();
        Push(is_array);
        s << (is_array ? '[' : '{');
        if (state.wrap) {
            indent += 2;
        }
        member_started = false;
        return *this;
    }
    JsonDumper& StartArray()
    {
        return StartObject(true);
    }

    JsonDumper& StartMember(const char* name)
    {
        if (state.in_array) {
            throw new std::bad_function_call();
        }
        if (!state.first_element) {
            s << ',';
            if (!state.wrap) {
                s << ' ';
            }
        } else {
            state.first_element = false;
        }
        if (state.wrap) {
            s << std::endl;
            s << GetIndentStr(indent);
        }
        s << "\"" << name << "\": ";
        member_started = true;
        return *this;
    }
    JsonDumper& End()
    {
        if (state.wrap) {
            if (!state.first_element) {
                s << std::endl;
                s << GetIndentStr(indent - 2);
            }
        }
        s << (state.in_array ? "]" : "}");
        Pop();
        if (state.wrap) {
            indent -= 2;
        }
        member_started = false;
        return *this;
    }

    JsonDumper& Write(const char* str, size_t max_len)
    {
        for (size_t i = 0; i < max_len; i++) {
            if (str[i] == '\0') {
                return *this << std::string_view(str, i);
            }
        }
        return *this << std::string_view(str, max_len);
    }

    JsonDumper& operator<<(const char* str)
    {
        return *this << std::string_view(str);
    }

    JsonDumper& operator<<(const std::string& str)
    {
        return *this << std::string_view(str);
    }

    JsonDumper& operator<<(double v)
    {
        if (std::isnan(v)) {
            return *this << "nan";
        }
        if (state.in_array) {
            NextItem();
        }
        s << v;
        member_started = false;
        return *this;
    }

    JsonDumper& operator<<(float v)
    {
        if (std::isnan(v)) {
            return *this << "nan";
        }
        return *this << (double)v;
    }

    JsonDumper& operator<<(int64_t v)
    {
        if (state.in_array) {
            NextItem();
        }
        s << v;
        member_started = false;
        return *this;
    }

    JsonDumper& operator<<(uint64_t v)
    {
        if (state.in_array) {
            NextItem();
        }
        s << v;
        member_started = false;
        return *this;
    }
    JsonDumper& operator<<(int32_t v) { return *this << (int64_t)v; }
    JsonDumper& operator<<(uint32_t v) { return *this << (uint64_t)v; }
    JsonDumper& operator<<(int16_t v) { return *this << (int64_t)v; }
    JsonDumper& operator<<(uint16_t v) { return *this << (uint64_t)v; }
    JsonDumper& operator<<(int8_t v) { return *this << (int64_t)v; }
    JsonDumper& operator<<(uint8_t v) { return *this << (uint64_t)v; }

    template <class T>
    JsonDumper& operator<<(T v)
    {
        if (state.in_array) {
            NextItem();
        }
        s << '"' << v << '"';
        member_started = false;
        return *this;
    }

    JsonDumper& operator<<(std::string_view sv)
    {
        if (state.in_array) {
            NextItem();
        }
        s << '"';
        for (auto c : sv) {
            if ((c == '"') || (c == '\\') || (('\0' < c) && (c < ' '))) {
                switch (c) {
                case '"':
                    s << "\\\"";
                    break;
                case '\\':
                    s << "\\\\";
                    break;
                case '\b':
                    s << "\\b";
                    break;
                case '\f':
                    s << "\\f";
                    break;
                case '\n':
                    s << "\\n";
                    break;
                case '\r':
                    s << "\\r";
                    break;
                case '\t':
                    s << "\\t";
                    break;
                default: {
                    char tmp[sizeof("\\u0000") + 1];
                    snprintf(tmp, sizeof(tmp), "\\u%04x", c);
                    s << tmp;
                } break;
                }
            } else {
                s << c;
            }
        }
        s << '"';
        member_started = false;
        return *this;
    }

protected:
    void Push(bool _in_array)
    {
        state_storage.push(state);
        state.first_element = true;
        state.in_array = _in_array;
    }
    void Pop()
    {
        state = state_storage.top();
        state_storage.pop();
    }
    void NextItem()
    {
        if (!state.first_element && state.in_array) {
            if (state_storage.size() > 1) {
                s << ',';
            } else {
                s << ", ";
            }
        }
        if (state.wrap) {
            if (!member_started) {
                if (state_storage.size() > 0 || !state.first_element) {
                    s << std::endl;
                }
                s << GetIndentStr(indent);
            }
        }
        state.first_element = false;
    }
};
