#include "JsonDumper.h"
#include "gtest/gtest.h"

static void TestEscape(char ch, const char* escaped) {
    char expected[100];
    snprintf(expected, sizeof(expected), "\"%s\"", escaped);
    { //As char
        std::ostringstream ss;
        JsonDumper j(ss);
        j << ch;
        EXPECT_EQ(expected, ss.str());
    }
    { // As string
        std::ostringstream ss;
        JsonDumper j(ss);
        char str[2] = { ch, '\0' };
        j << str;
        EXPECT_EQ(expected, ss.str());
    }
}

TEST(JsonDumper, Escape) {
    TestEscape('A', "A"); //No escape
    TestEscape('"', "\\\"");
    TestEscape('\\', "\\\\");
    TestEscape('\n', "\\n");
    TestEscape('\r', "\\r");
    TestEscape('\b', "\\b");
    TestEscape('\f', "\\f");
    TestEscape('\t', "\\t");
    TestEscape('\x01', "\\u0001");
}

TEST(JsonDumper, EmptyObject)
{
    std::ostringstream ss;
    JsonDumper j(ss);
    j.StartObject();
    j.End();
    EXPECT_EQ("{}", ss.str());
}

TEST(JsonDumper, EmptyArray)
{
    std::ostringstream ss;
    JsonDumper j(ss);
    j.StartArray();
    j.End();
    EXPECT_EQ("[]", ss.str());
}

static void DumpTest(JsonDumper& j)
{
    j.StartObject();
    j.StartMember("Test").StartObject();
    j.StartMember("Int8") << (int8_t)-50;
    j.StartMember("Int16") << (int16_t)-10;
    j.StartMember("Int32") << (int32_t)-10;
    j.StartMember("Int64") << (int64_t)-10;
    j.StartMember("UInt8") << (uint8_t)70;
    j.StartMember("UInt16") << (uint16_t)10;
    j.StartMember("UInt32") << (uint32_t)10;
    j.StartMember("UInt64") << (uint64_t)10;
    j.StartMember("double") << 12.345;
    j.StartMember("float") << 12.345f;
    j.StartMember("NaN") << std::nan("");
    j.StartMember("NaNf") << std::nanf("");
    j.StartMember("Char") << (char)'c';
    j.StartMember("c_str") << "c_string";
    j.StartMember("std_str") << std::string("std::string");
    j.StartMember("string_view") << std::string_view("sv");
    j.StartMember("arr").StartArray();
    for (int i = 0; i < 5; i++) {
        j << i;
    }
    j.End();
    j.End();
    j.End();
}

TEST(JsonDumper, ObjectWrap)
{
    std::ostringstream ss;
    JsonDumper j(ss);
    j.SetWrap(true);
    DumpTest(j);
    EXPECT_EQ(
        "{\n"
        "  \"Test\": {\n"
        "    \"Int8\": -50,\n"
        "    \"Int16\": -10,\n"
        "    \"Int32\": -10,\n"
        "    \"Int64\": -10,\n"
        "    \"UInt8\": 70,\n"
        "    \"UInt16\": 10,\n"
        "    \"UInt32\": 10,\n"
        "    \"UInt64\": 10,\n"
        "    \"double\": 12.345,\n"
        "    \"float\": 12.345,\n"
        "    \"NaN\": \"nan\",\n"
        "    \"NaNf\": \"nan\",\n"
        "    \"Char\": \"c\",\n"
        "    \"c_str\": \"c_string\",\n"
        "    \"std_str\": \"std::string\",\n"
        "    \"string_view\": \"sv\",\n"
        "    \"arr\": [\n"
        "      0,\n"
        "      1,\n"
        "      2,\n"
        "      3,\n"
        "      4\n"
        "    ]\n"
        "  }\n"
        "}",
        ss.str());
}

TEST(JsonDumper, ObjectNoWrap)
{
    std::ostringstream ss;
    JsonDumper j(ss);
    DumpTest(j);
    EXPECT_EQ(
        "{\"Test\": {\"Int8\": -50, \"Int16\": -10, \"Int32\": -10, \"Int64\": -10, \"UInt8\": 70, \"UInt16\": 10, \"UInt32\": 10, \"UInt64\": 10, \"double\": 12.345, \"float\": 12.345, \"NaN\": \"nan\", \"NaNf\": \"nan\", \"Char\": \"c\", \"c_str\": \"c_string\", "
        "\"std_str\": \"std::string\", \"string_view\": \"sv\", \"arr\": [0,1,2,3,4]}}",
        ss.str());
}
