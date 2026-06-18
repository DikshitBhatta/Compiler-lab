#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <cctype>

using namespace std;

// token = (lexeme, type) pair stored for later phases of the compiler
struct Token {
    string lexeme;
    string type;
};

const string KEYWORDS[] = {
    "int", "float", "double", "char", "if", "else",
    "while", "for", "return", "void", "printf", "scanf"
};

bool isKeyword(const string &word)
{
    for (const string &kw : KEYWORDS) {
        if (word == kw) {
            return true;
        }
    }
    return false;
}

bool isOperatorChar(char c)
{
    return c == '+' || c == '-' || c == '*' || c == '/' ||
           c == '=' || c == '<' || c == '>' || c == '!' ||
           c == '&' || c == '|' || c == '%';
}

bool isDelimiter(char c)
{
    return c == ';' || c == ',' || c == '(' || c == ')' ||
           c == '{' || c == '}' || c == '[' || c == ']';
}

int main()
{
    ifstream file("File.txt");
    if (!file) {
        cout << "Error: could not open File.txt" << endl;
        return 1;
    }

    vector<Token> tokens; // storage of the generated tokens
    char c;

    while (file.get(c)) {
        if (isspace(c)) {
            // skip whitespace
            continue;
        }

        if (isalpha(c) || c == '_') {
            // keyword / identifier
            string word(1, c);
            while (file.get(c) && (isalnum(c) || c == '_')) {
                word += c;
            }
            file.unget();
            tokens.push_back({word, isKeyword(word) ? "KEYWORD" : "IDENTIFIER"});
        } else if (isdigit(c)) {
            // number (integer or real)
            string num(1, c);
            while (file.get(c) && (isdigit(c) || c == '.')) {
                num += c;
            }
            file.unget();
            tokens.push_back({num, "NUMBER"});
        } else if (isOperatorChar(c)) {
            // single / double operator
            string op(1, c);
            if (file.get(c)) {
                if (isOperatorChar(c)) {
                    // ==, >=, <=, !=, &&, ||, ...
                    op += c;
                } else {
                    file.unget();
                }
            }
            tokens.push_back({op, "OPERATOR"});
        } else if (isDelimiter(c)) {
            tokens.push_back({string(1, c), "DELIMITER"});
        } else {
            tokens.push_back({string(1, c), "UNKNOWN"});
        }
    }

    file.close();

    cout << "+-----+-----------------+--------------+" << endl;
    cout << "| S.N | LEXEME          | TOKEN        |" << endl;
    cout << "+-----+-----------------+--------------+" << endl;
    for (size_t i = 0; i < tokens.size(); i++) {
        cout << "| " << setw(3) << left << i + 1
             << " | " << setw(15) << left << tokens[i].lexeme
             << " | " << setw(12) << left << tokens[i].type << " |" << endl;
    }
    cout << "+-----+-----------------+--------------+" << endl;
    cout << "Total tokens generated and stored: " << tokens.size() << endl;

    return 0;
}
