#ifndef EASY_JSON_H__
#define EASY_JSON_H__

#include <memory>
#include <string>
#include <vector>
#include <fstream>

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
    std::fstream fout;
    json();
    json(bool _b);
    json(double _num);
    json(string _value);
    json(vector<shared_ptr<json>> _array);
    json(vector<pair<string, shared_ptr<json>>> _object);
    jsonType getType();
    bool getB();
    double getNum();
    string getValue();
    vector<shared_ptr<json>>& getArray();
    vector<pair<string, shared_ptr<json>>>& getObject();
    string toString();
};

class parser {
    string jstr;
    string::const_iterator it;
    int node_id = 0;
    int subg_id = 0;
public:
    shared_ptr<json> rt;
    std::fstream fout;
    bool match(char c, bool omit=true); // omit white spaces defaultly
    bool match(string s);
    void init(string _jstr);
    shared_ptr<json> parse();
    void genElement(shared_ptr<json> value);
    void genObject(shared_ptr<json> rt);
    void show(string file);
};

#endif