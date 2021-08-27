#include "json.h"

#include <iostream>

json::json() {
    type = JSON_NULL;
}

json::json(bool _b) : b(_b) {
    type = JSON_BOOLEAN;
}

json::json(double _num) : num(_num) {
    type = JSON_NUMBER;
}

json::json(string _value) : value(_value) {
    type = JSON_STRING;
}

json::json(vector<shared_ptr<json>> _array) : array(_array) {
    type = JSON_ARRAY;
}

json::json(vector<pair<string, shared_ptr<json>>> _object) : object(_object) {
    type = JSON_OBJECT;
}

string json::toString() {
    string s;
    bool tag = true;
    switch(type) {
        case JSON_NULL: return "null";
        case JSON_BOOLEAN: return (b ? "true" : "false");
        case JSON_NUMBER: return std::to_string(num);
        case JSON_STRING: return value;
        case JSON_ARRAY: 
            s = "[";
            for(auto& item : array) {
                if(!tag) s += ", ";
                else tag = false;
                s += item->toString();
            }
            return s + "]";
        default: // object TODO: fix indent 
            if(object.empty()) return "{}";
            s = "{";
            for(auto& attribute : object) {
                if(!tag) s += ",\n  ";
                else s += "\n  ", tag = false;
                s += '"' + attribute.first + "\": " + attribute.second->toString(); 
            }
            return s + "\n}";
    }
}

jsonType json::getType() {
    return type;
}

bool isWhiteSpace(string::const_iterator it) {
    return *it==' ' || *it=='\n' || *it=='\t' || *it=='\r';
}

bool isDigit(string::const_iterator it) {
    return *it>='0' && *it<='9';
}

bool parser::match(char c, bool omit) { 
    if(omit) while(it!=cend(jstr) && isWhiteSpace(it)) ++it;
    if(it!=cend(jstr) && *it==c) {
        ++it;
        return true;
    }
    return false;
}

bool parser::match(string s) {
    for(char c : s) {
        if(it==cend(jstr)) return false;
        if(!match(c, false)) return false;
    }
    return true;
}

shared_ptr<json> parser::parse() {
    shared_ptr<json> rt;
    while(it!=cend(jstr) && isWhiteSpace(it)) ++it;
    if(it==cend(jstr)) return nullptr;
    if(isDigit(it) || *it=='.') { // number
        string number;
        do {
            number += *it;
            ++it;
        } while(it!=cend(jstr) && (isDigit(it) || *it=='.'));
        rt = make_shared<json>(stod(number)); //TODO accuracy
    } else if(*it=='[') { // array
        vector<shared_ptr<json>> array;
        ++it;
        if(it==end(jstr)) return nullptr;
        else if(*it!=']') {
            while(true) {
                auto item = parse();
                if(item==nullptr) return nullptr;
                array.emplace_back(item);
                if(match(',')) {
                    continue;
                } else if(match(']')) {
                    break;
                } else return nullptr;
            }
        } else ++it;
        rt = make_shared<json>(array);
    } else if(*it=='{') { // object
        vector<pair<string, shared_ptr<json>>> object;
        ++it;
        if(it==end(jstr)) return nullptr;
        else if(*it!='}') {
            while(true) {
                if(!match('"')) return nullptr;
                string key;
                while(true) {
                    if(it==cend(jstr)) return nullptr;
                    if(*it=='"') {
                        ++it;
                        break;
                    }
                    key += *it;
                    ++it;
                }
                if(!match(':')) return nullptr;
                auto value = parse();
                if(value==nullptr) return nullptr;
                object.emplace_back(key, value);
                if(match(',')) {
                    continue;
                } else if(match('}')) {
                    break;
                } else return nullptr;
            }
        } else ++it;
        rt = make_shared<json>(object);
    } else {
        switch(*it) {
            case 't': 
                if(!match("true")) return nullptr;
                rt = make_shared<json>(true);
                break;
            case 'f':
                if(!match("false")) return nullptr;
                rt = make_shared<json>(false);
                break;
            case 'n':
                if(!match("null")) return nullptr;
                rt = make_shared<json>();
                break;
            case '"':
                string value;
                while(true) {
                    ++it;
                    if(it==cend(jstr)) return nullptr;
                    if(*it=='"') {
                        ++it; 
                        break;
                    }
                    value += *it;
                }
                rt = make_shared<json>(value);
                break;
        }
    }
    return rt;
}

void parser::init(string _jstr) {
    jstr = _jstr;
    it = cbegin(jstr);
}

int main() {
    parser p;
    p.init(R"( {
        "apple": "red",
        "weight": 12.0,   
        "nothing": null,
        "quality": "high",
        "price": [12.0, 21.2, 31.3]   
    } )");
    p.rt = p.parse();
    if(p.rt!=nullptr) {
        std::cout << p.rt->toString() << std::endl;
    }
    return 0;
}