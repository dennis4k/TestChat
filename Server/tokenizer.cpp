#include "tokenizer.hpp"
#include <iostream>

using namespace std;

Tokenizer::Tokenizer(string data, string sep) {
    _pos = 0;
    
    string::size_type beg, end;
    beg = data.find_first_not_of(sep, 0);
    end = data.find_first_of(sep, beg);
        
    while (string::npos != beg || string::npos != end) {
        string s = data.substr(beg, end - beg);
        _tokens.push_back(s);
        
        beg = data.find_first_not_of(sep, end);
        end = data.find_first_of(sep, beg);
    }
}

size_t Tokenizer::countTokens() {
    return _tokens.size();
}

string Tokenizer::nextToken() {
    return _tokens[_pos++];
}

bool Tokenizer::hasMoreTokens() {
    return _pos < _tokens.size();
}
