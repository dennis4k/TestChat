#include <string>
#include <set>
#include <iostream>
#include <vector>
using namespace std;


class Tokenizer {
private:
    unsigned int _pos;
    vector<string> _tokens;
public:
    Tokenizer(string data, string separators);
    size_t countTokens();
    string nextToken();
    bool hasMoreTokens();
};