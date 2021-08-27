#ifndef EASY_JSON_H__
#define EASY_JSON_H__

#include <memory>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::shared_ptr;
using std::make_shared;
using std::shared_ptr;
using std::pair;

enum jsonType {
    JSON_NULL,
    JSON_BOOLEAN, 
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
};

class json {
    jsonType type;
    bool b;
    double num;
    string value;
    vector<shared_ptr<json>> array;
    vector<pair<string, shared_ptr<json>>> object;
public:
    json();
    json(bool _b);
    json(double _num);
    json(string _value);
    json(vector<shared_ptr<json>> _array);
    json(vector<pair<string, shared_ptr<json>>> _object);
    jsonType getType();
    string toString();
};

class parser {
    string jstr;
    string::const_iterator it;
public:
    shared_ptr<json> rt;
    bool match(char c, bool omit=true); // omit white spaces defaultly
    bool match(string s);
    void init(string _jstr);
    shared_ptr<json> parse();
};

#endif