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

#define TAB_SIZE 2
string tab(int cnt) { return string(cnt*TAB_SIZE, ' '); }

string toString(double num) {
    static char buf[32];
    snprintf(buf, sizeof(buf), "%.16g", num);
    return string(buf);
}

string json::dump(int indent) {
    string s;
    bool tag = true;
    switch(type) {
        case JSON_NULL: return "null";
        case JSON_BOOLEAN: return (b ? "true" : "false");
        case JSON_NUMBER: return toString(num);
        case JSON_STRING: return value;
        case JSON_ARRAY: 
            s = "[";
            for(auto& item : array) {
                if(!tag) s += ", ";
                else tag = false;
                s += item->dump();
            }
            return s + "]";
        default: 
            if(object.empty()) return "{}";
            s = "{";
            indent++;
            for(auto& attribute : object) {
                if(!tag) s += ",\n" + tab(indent);
                else s += "\n" + tab(indent), tag = false;
                s += '"' + attribute.first + "\": " + attribute.second->dump(indent); 
            }
            return s + "\n" + tab(indent-1) + "}";
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

bool json::isAtom() {
    return type!=JSON_ARRAY && type!=JSON_OBJECT;
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
    if(isDigit(it) || *it=='.' || *it=='-') { // number
        string number;
        do {
            number += *it;
            ++it;
        } while(it!=cend(jstr) && (isDigit(it) || *it=='.'));
        rt = make_shared<json>(std::strtod(number.c_str(), nullptr)); //TODO accuracy
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

void parser::init(string file) {
    std::fstream fin(file);
    if(!fin.is_open()) {
        std::cerr << "Can't open the json file!" << std::endl;
        exit(1);
    }
    jstr.assign((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    it = cbegin(jstr);
}

void parser::genObject(shared_ptr<json> rt, int indent, string label) {
    assert(rt->getType()==JSON_OBJECT);
    auto object = rt->getObject();
    indent++;
    fout << tab(indent-1) + "subgraph cluster" + std::to_string(subg_id++) + " {\n" 
            + tab(indent) + "label = \"" + label + "\"\n"
            + tab(indent) + "color = black\n";
    if(object.empty()) { // empty subgraph will not been shown
        fout << tab(indent) + std::to_string(node_id++) + " [style=\"invis\"]\n";
    } else {
        for(auto& [key, value] : object) {
            fout << tab(indent) + "subgraph cluster" + std::to_string(subg_id++) + " {\n"
                + tab(indent+1) + "label = <<B><FONT COLOR=\"red\">" + key + "</FONT></B>>\n"
                + tab(indent+1) + "color = white\n";
            if(value->isAtom()) genAtom(value, indent+1);
            else if(value->getType()==JSON_ARRAY) genArray(value, indent+1);
            else genObject(value, indent+1);
            fout << tab(indent) + "}\n";
        }
    }
    fout << tab(indent-1) << "}\n";
}

void parser::genArray(shared_ptr<json> value, int indent, string label) {
    assert(value->getType()==JSON_ARRAY);
    fout << tab(indent) + "subgraph cluster" + std::to_string(subg_id++) + " {\n" 
        + tab(indent+1) + "label = \"" + label + "\"\n"
        + tab(indent+1) + "color = black\n";
    auto array = value->getArray();
    int idx = 0;
    for(auto& item : array) {
        if(item->isAtom()) {
            fout << tab(indent+1) + "subgraph cluster" + std::to_string(subg_id++) + " {\n"
                + tab(indent+2) + "label = \"[" + std::to_string(idx++) + "]\"\n"
                + tab(indent+2) + "color = black\n";
            genAtom(item, indent+2);
            fout << tab(indent+1) << "}\n";
        } else if(item->getType()==JSON_ARRAY) {
            genArray(item, indent+1, "["+std::to_string(idx++)+"]");
        } else {
            genObject(item, indent+1, "["+std::to_string(idx++)+"]");
        }
    }
    fout << tab(indent) + "}\n";
}

void parser::genAtom(shared_ptr<json> value, int indent) {
    assert(value->isAtom());
    switch(value->getType()) {
        case JSON_NULL: 
            fout << tab(indent) + std::to_string(node_id++) + " [label=\"null\"]\n";
            return;
        case JSON_BOOLEAN: 
            fout << tab(indent) + std::to_string(node_id++) + " [label=\"" + (value->getB() ? "true" : "false") + "\"]\n";
            return;
        case JSON_NUMBER: 
            fout << tab(indent) + std::to_string(node_id++) + " [label=\"" + toString(value->getNum()) + "\"]\n";
            return;
        default: //string 
            fout << tab(indent) + std::to_string(node_id++) + " [label=\"" + value->getValue() + "\"]\n";
            return;
    }
}

void parser::show(string file, string json_file) {
    fout.open(file, std::ios::out);
    if(!fout.is_open()) {
        std::cerr << "Can't open the file!" << std::endl;
        exit(1);
    } 
    fout << "digraph {\n" + tab(1) + "node [shape=ellipse, color=cadetblue, style=filled]\n" + 
        tab(1) + "label = \"" + json_file + "\"\n" + 
        tab(1) + "edge [minlen=0.1]\n" + 
        tab(1) + "compound = true\n";
    genObject(rt, 1);
    fout << "}";
    fout.close();
}

int main(int argc, char *argv[]) {
    parser p;
    if(argc!=2) {
        std::cerr << "Please input the json file correctly!" << std::endl;
        return 1;
    }
    string json_file = argv[1];
    p.init(json_file);
    p.rt = p.parse();
    if(p.rt!=nullptr) {
        std::cout << p.rt->dump() << std::endl;
        p.show("tmp.dot", json_file);
    }
    return 0;
}