#include "json.h"

#include <iostream>
#include <assert.h>

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

bool json::getB() {
    return b;
}

double json::getNum() {
    return num;
}

string json::getValue() {
    return value;
}

vector<shared_ptr<json>>& json::getArray() {
    return array;
}

vector<pair<string, shared_ptr<json>>>& json::getObject() {
    return object;
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

void parser::genObject(shared_ptr<json> rt) {
    assert(rt->getType()==JSON_OBJECT);
    auto object = rt->getObject();
    for(auto& [key, value] : object) {
        fout << "subgraph cluster" + std::to_string(subg_id++) + "{\n";
        fout << "label = <<FONT COLOR=\"red\">" + key + "</FONT>>\n";
        genElement(value);
        fout << "}\n";
    }
}

void parser::genElement(shared_ptr<json> value) {
    switch(value->getType()) {
        case JSON_NULL: 
            fout << std::to_string(node_id++) + " [label=\"null\"]\n";
            break;
        case JSON_BOOLEAN: 
            fout << std::to_string(node_id++) + " [label=\"" + (value->getB() ? "true" : "false") + "\"]\n";
            break;
        case JSON_NUMBER: 
            fout << std::to_string(node_id++) + " [label=\"" + std::to_string(value->getNum()) + "\"]\n";
            break;
        case JSON_STRING: 
            fout << std::to_string(node_id++) + " [label=\"" + value->getValue() + "\"]\n";
            break;
        case JSON_ARRAY:
            {
                auto array = value->getArray();
                for(auto& item : array) {
                    genElement(item);
                }
            }
            break;
        default: 
            genObject(value);
    }
}

void parser::show(string file) {
    fout.open(file, std::ios::out);
    if(!fout.is_open()) {
        std::cerr << "Can't open the file" << '\n';
        exit(1);
    } 
    fout << "graph {node [shape=\"box\"]\ncompound = true\n";
    genObject(rt);
    fout << "}";
    fout.close();
}

int main() {
    parser p;
    p.init(R"( {
        "color": "red",
        "weight": 12.0,   
        "nothing": null,
        "quality": "high",
        "list": [12.0, 21.2, 31.3],
        "object": {
            "boolean": true,
            "name": "apple" 
        }
    } )");
    p.rt = p.parse();
    if(p.rt!=nullptr) {
        std::cout << p.rt->toString() << std::endl;
        p.show("test.dot");
    }
    return 0;
}