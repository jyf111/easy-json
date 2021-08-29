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

string tab(int cnt) { return string(cnt<<2, ' '); }

string json::toString(int indent) {
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
        default: 
            if(object.empty()) return "{}";
            s = "{";
            indent++;
            for(auto& attribute : object) {
                if(!tag) s += ",\n" + tab(indent);
                else s += "\n" + tab(indent), tag = false;
                s += '"' + attribute.first + "\": " + attribute.second->toString(indent); 
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

void parser::init(string file) {
    std::fstream fin(file);
    if(!fin.is_open()) {
        std::cerr << "Can't open the json file!" << std::endl;
        exit(1);
    }
    jstr.assign((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    it = cbegin(jstr);
}

void parser::genObject(shared_ptr<json> rt, int indent) {
    assert(rt->getType()==JSON_OBJECT);
    auto object = rt->getObject();
    if(object.empty()) { // empty subgraph will not been shown
        fout << tab(indent) + std::to_string(node_id++) + " [style=\"invis\"]\n";
    } else {
        for(auto& [key, value] : object) {
            fout << tab(indent) + "subgraph cluster" + std::to_string(subg_id++) + " {\n";
            fout << tab(indent+1) + "label = <<FONT COLOR=\"red\">" + key + "</FONT>>\n";
            genElement(value, indent+1);
            fout << tab(indent) + "}\n";
        }
    }
}

void parser::genElement(shared_ptr<json> value, int indent) {
    switch(value->getType()) {
        case JSON_NULL: 
            fout << tab(indent) + std::to_string(node_id++) + " [label=\"null\"]\n";
            break;
        case JSON_BOOLEAN: 
            fout << tab(indent) + std::to_string(node_id++) + " [label=\"" + (value->getB() ? "true" : "false") + "\"]\n";
            break;
        case JSON_NUMBER: 
            fout << tab(indent) + std::to_string(node_id++) + " [label=\"" + std::to_string(value->getNum()) + "\"]\n";
            break;
        case JSON_STRING: 
            fout << tab(indent) + std::to_string(node_id++) + " [label=\"" + value->getValue() + "\"]\n";
            break;
        case JSON_ARRAY:
            {
                auto array = value->getArray();
                for(auto& item : array) {
                    genElement(item, indent);
                }
            }
            break;
        default: 
            genObject(value, indent);
    }
}

void parser::show(string file, string json_file) {
    fout.open(file, std::ios::out);
    if(!fout.is_open()) {
        std::cerr << "Can't open the file!" << std::endl;
        exit(1);
    } 
    fout << "graph {\n" + tab(1) + "node [shape=box, color=cadetblue, style=filled]\n" + tab(1) + "label = \"" + json_file + "\"\n";
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
        std::cout << p.rt->toString() << std::endl;
        p.show("tmp.dot", json_file);
    }
    return 0;
}