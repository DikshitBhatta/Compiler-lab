// ============================================================================
//  Canonical LR(1) Parser
//  Compiler Design Mini-Project  (COMP 409)
//
//  Augmented Grammar
//      0.  S' -> S
//      1.  S  -> L = R
//      2.  S  -> R
//      3.  L  -> * R
//      4.  L  -> id
//      5.  R  -> L
//
//  The program:
//      1. builds FIRST sets,
//      2. constructs the canonical collection of sets of LR(1) items,
//      3. builds the ACTION and GOTO parsing tables,
//      4. reports whether any conflict exists in the table, and
//      5. drives the LR parser over one or more input strings, printing the
//         complete stack / input / action trace.
//
//  Build :  g++ -std=c++17 -O2 canonical_lr_parser.cpp -o clr
//  Run   :  ./clr
// ============================================================================

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include <iomanip>

using namespace std;

// ---------------------------------------------------------------------------
//  Grammar definition
// ---------------------------------------------------------------------------
struct Production {
    string lhs;
    vector<string> rhs;
};

vector<Production> G;            // numbered grammar productions
set<string> terminals;
set<string> nonTerminals;
const string ENDM = "$";         // end-of-input marker
const string AUG  = "S'";        // augmented start symbol

bool isTerminal(const string& s)    { return terminals.count(s)    > 0; }
bool isNonTerminal(const string& s) { return nonTerminals.count(s) > 0; }

void defineGrammar() {
    G.push_back({"S'", {"S"}});          // 0
    G.push_back({"S",  {"L", "=", "R"}}); // 1
    G.push_back({"S",  {"R"}});          // 2
    G.push_back({"L",  {"*", "R"}});     // 3
    G.push_back({"L",  {"id"}});         // 4
    G.push_back({"R",  {"L"}});          // 5

    nonTerminals = {"S'", "S", "L", "R"};
    terminals    = {"=", "*", "id", ENDM};
}

// pretty-print a production "A -> X Y Z"
string showProduction(int idx) {
    const Production& p = G[idx];
    string s = p.lhs + " -> ";
    for (size_t i = 0; i < p.rhs.size(); ++i) {
        s += p.rhs[i];
        if (i + 1 < p.rhs.size()) s += " ";
    }
    return s;
}

// ---------------------------------------------------------------------------
//  FIRST sets  (this grammar has no epsilon productions)
// ---------------------------------------------------------------------------
map<string, set<string>> FIRST;

void computeFirst() {
    for (const string& t : terminals) FIRST[t] = {t};
    for (const string& nt : nonTerminals) FIRST[nt]; // create empty entries

    bool changed = true;
    while (changed) {
        changed = false;
        for (const Production& p : G) {
            const string& Y = p.rhs[0];           // first symbol of the body
            for (const string& s : FIRST[Y]) {
                if (!FIRST[p.lhs].count(s)) {
                    FIRST[p.lhs].insert(s);
                    changed = true;
                }
            }
        }
    }
}

// FIRST of a symbol sequence. The sequence always ends with a terminal (the
// carried look-ahead), and the grammar has no epsilon, so FIRST is determined
// by the first symbol of the sequence.
set<string> firstOfSeq(const vector<string>& seq) {
    set<string> res;
    if (!seq.empty())
        for (const string& s : FIRST[seq[0]]) res.insert(s);
    return res;
}

// ---------------------------------------------------------------------------
//  LR(1) items and sets
// ---------------------------------------------------------------------------
struct Item {
    int    prod;   // index into G
    int    dot;    // position of the dot inside the body
    string look;   // look-ahead terminal

    bool operator<(const Item& o) const {
        if (prod != o.prod) return prod < o.prod;
        if (dot  != o.dot ) return dot  < o.dot;
        return look < o.look;
    }
    bool operator==(const Item& o) const {
        return prod == o.prod && dot == o.dot && look == o.look;
    }
};
using ItemSet = set<Item>;

// closure(I) : repeatedly expand non-terminals appearing right after the dot
ItemSet closure(ItemSet I) {
    bool changed = true;
    while (changed) {
        changed = false;
        vector<Item> toAdd;
        for (const Item& it : I) {
            const Production& p = G[it.prod];
            if (it.dot < (int)p.rhs.size()) {
                string B = p.rhs[it.dot];          // symbol after the dot
                if (isNonTerminal(B)) {
                    // beta = symbols after B, followed by the carried look-ahead
                    vector<string> beta(p.rhs.begin() + it.dot + 1, p.rhs.end());
                    beta.push_back(it.look);
                    set<string> fb = firstOfSeq(beta);
                    for (int j = 0; j < (int)G.size(); ++j) {
                        if (G[j].lhs == B) {
                            for (const string& b : fb) {
                                Item ni{j, 0, b};
                                if (!I.count(ni)) toAdd.push_back(ni);
                            }
                        }
                    }
                }
            }
        }
        for (const Item& ni : toAdd)
            if (!I.count(ni)) { I.insert(ni); changed = true; }
    }
    return I;
}

// goto(I, X) : advance the dot over symbol X, then take the closure
ItemSet goTo(const ItemSet& I, const string& X) {
    ItemSet J;
    for (const Item& it : I) {
        const Production& p = G[it.prod];
        if (it.dot < (int)p.rhs.size() && p.rhs[it.dot] == X)
            J.insert({it.prod, it.dot + 1, it.look});
    }
    return closure(J);
}

// ---------------------------------------------------------------------------
//  Canonical collection of sets of LR(1) items
// ---------------------------------------------------------------------------
vector<ItemSet>            C;        // the states
map<pair<int,string>, int> TRANS;    // raw goto transitions  (state, symbol) -> state

// symbol order chosen so that state numbering follows the standard textbook layout
const vector<string> SYMBOL_ORDER = {"S", "L", "R", "*", "id", "="};

int findState(const ItemSet& s) {
    for (int i = 0; i < (int)C.size(); ++i)
        if (C[i] == s) return i;
    return -1;
}

void buildCanonicalCollection() {
    ItemSet I0 = closure({ Item{0, 0, ENDM} });   // closure of { [S' -> .S , $] }
    C.push_back(I0);

    for (int i = 0; i < (int)C.size(); ++i) {
        for (const string& X : SYMBOL_ORDER) {
            ItemSet g = goTo(C[i], X);
            if (g.empty()) continue;
            int j = findState(g);
            if (j == -1) { C.push_back(g); j = (int)C.size() - 1; }
            TRANS[{i, X}] = j;
        }
    }
}

// ---------------------------------------------------------------------------
//  ACTION and GOTO tables
// ---------------------------------------------------------------------------
map<int, map<string,string>> ACTION;   // ACTION[state][terminal]    = "sN" | "rN" | "acc"
map<int, map<string,int>>    GOTOT;     // GOTOT [state][nonterminal] = N
vector<string>               conflicts;

void recordAction(int state, const string& sym, const string& act) {
    if (ACTION[state].count(sym) && ACTION[state][sym] != act) {
        conflicts.push_back("State I" + to_string(state) + " on '" + sym +
                            "': " + ACTION[state][sym] + " vs " + act);
    }
    ACTION[state][sym] = act;
}

void buildTables() {
    for (int i = 0; i < (int)C.size(); ++i) {
        for (const Item& it : C[i]) {
            const Production& p = G[it.prod];
            if (it.dot < (int)p.rhs.size()) {
                string a = p.rhs[it.dot];
                if (isTerminal(a)) {                       // shift
                    recordAction(i, a, "s" + to_string(TRANS[{i, a}]));
                }
            } else {                                        // dot at the end
                if (it.prod == 0) {                         // S' -> S .
                    if (it.look == ENDM) recordAction(i, ENDM, "acc");
                } else {                                    // reduce
                    recordAction(i, it.look, "r" + to_string(it.prod));
                }
            }
        }
        for (const string& nt : nonTerminals) {
            if (nt == AUG) continue;
            if (TRANS.count({i, nt})) GOTOT[i][nt] = TRANS[{i, nt}];
        }
    }
}

// ---------------------------------------------------------------------------
//  Printing helpers
// ---------------------------------------------------------------------------
string showItem(const Item& it) {
    const Production& p = G[it.prod];
    string s = p.lhs + " -> ";
    for (int i = 0; i <= (int)p.rhs.size(); ++i) {
        if (i == it.dot) s += ". ";
        if (i < (int)p.rhs.size()) s += p.rhs[i] + " ";
    }
    s += ", " + it.look;
    return s;
}

void printGrammar() {
    cout << "Augmented Grammar:\n";
    for (int i = 0; i < (int)G.size(); ++i)
        cout << "   " << i << ".  " << showProduction(i) << "\n";
    cout << "\n";
}

void printFirst() {
    cout << "FIRST sets:\n";
    for (const string& nt : {"S", "L", "R"}) {
        cout << "   FIRST(" << nt << ") = { ";
        bool first = true;
        for (const string& s : FIRST[nt]) { cout << (first ? "" : ", ") << s; first = false; }
        cout << " }\n";
    }
    cout << "\n";
}

void printStates() {
    cout << "Canonical collection of sets of LR(1) items  ("
         << C.size() << " states):\n\n";
    for (int i = 0; i < (int)C.size(); ++i) {
        cout << "I" << i << ":\n";
        for (const Item& it : C[i])
            cout << "    [ " << showItem(it) << " ]\n";
        // outgoing transitions
        for (const string& X : SYMBOL_ORDER)
            if (TRANS.count({i, X}))
                cout << "      goto(I" << i << ", " << X << ") = I" << TRANS[{i, X}] << "\n";
        cout << "\n";
    }
}

void printTable() {
    vector<string> terms = {"id", "*", "=", ENDM};
    vector<string> nts   = {"S", "L", "R"};

    cout << "Parsing Table  (ACTION | GOTO):\n\n";
    cout << setw(7) << left << "STATE";
    for (const string& t : terms) cout << setw(7) << left << t;
    cout << "|";
    for (const string& n : nts)   cout << setw(6) << right << n;
    cout << "\n";
    cout << string(7 + 7 * terms.size() + 1 + 6 * nts.size(), '-') << "\n";

    for (int i = 0; i < (int)C.size(); ++i) {
        cout << setw(7) << left << ("I" + to_string(i));
        for (const string& t : terms) {
            string cell = ACTION[i].count(t) ? ACTION[i][t] : "";
            cout << setw(7) << left << cell;
        }
        cout << "|";
        for (const string& n : nts) {
            string cell = GOTOT[i].count(n) ? to_string(GOTOT[i][n]) : "";
            cout << setw(6) << right << cell;
        }
        cout << "\n";
    }
    cout << "\n";
}

// ---------------------------------------------------------------------------
//  The LR parsing driver
// ---------------------------------------------------------------------------
string join(const vector<string>& v, int from = 0) {
    string s;
    for (int i = from; i < (int)v.size(); ++i) {
        s += v[i];
        if (i + 1 < (int)v.size()) s += " ";
    }
    return s;
}
string joinStates(const vector<int>& v) {
    string s;
    for (size_t i = 0; i < v.size(); ++i) {
        s += to_string(v[i]);
        if (i + 1 < v.size()) s += " ";
    }
    return s;
}

bool parse(vector<string> input, const string& label) {
    cout << "----------------------------------------------------------------\n";
    cout << "Parsing string : " << join(input) << "\n";
    cout << "----------------------------------------------------------------\n";
    input.push_back(ENDM);

    vector<int>    stStack  = {0};   // state stack
    vector<string> symStack;         // symbol stack (display only)
    int ip = 0;

    cout << left
         << setw(22) << "STACK"
         << setw(16) << "SYMBOLS"
         << setw(18) << "INPUT"
         << "ACTION\n";

    while (true) {
        int    s = stStack.back();
        string a = input[ip];

        string stackStr = joinStates(stStack);
        string symStr   = join(symStack);
        string inStr    = join(input, ip);

        if (!ACTION[s].count(a)) {
            cout << left << setw(22) << stackStr << setw(16) << symStr
                 << setw(18) << inStr << "ERROR (no action)\n";
            cout << "\nResult: string \"" << label << "\" is REJECTED.\n\n";
            return false;
        }

        string act = ACTION[s][a];

        if (act[0] == 's') {                       // shift
            int j = stoi(act.substr(1));
            cout << left << setw(22) << stackStr << setw(16) << symStr
                 << setw(18) << inStr << "shift -> I" << j << "\n";
            stStack.push_back(j);
            symStack.push_back(a);
            ++ip;
        } else if (act[0] == 'r') {                // reduce
            int pr = stoi(act.substr(1));
            const Production& p = G[pr];
            cout << left << setw(22) << stackStr << setw(16) << symStr
                 << setw(18) << inStr << "reduce by " << showProduction(pr) << "\n";
            for (size_t k = 0; k < p.rhs.size(); ++k) {
                stStack.pop_back();
                symStack.pop_back();
            }
            int t = stStack.back();
            int g = GOTOT[t][p.lhs];
            stStack.push_back(g);
            symStack.push_back(p.lhs);
        } else {                                   // accept
            cout << left << setw(22) << stackStr << setw(16) << symStr
                 << setw(18) << inStr << "ACCEPT\n";
            cout << "\nResult: string \"" << label << "\" is ACCEPTED.\n\n";
            return true;
        }
    }
}

// ---------------------------------------------------------------------------
//  main
// ---------------------------------------------------------------------------
int main() {
    defineGrammar();
    computeFirst();
    buildCanonicalCollection();
    buildTables();

    cout << "================================================================\n";
    cout << "              CANONICAL LR(1) PARSER  -  COMP 409\n";
    cout << "================================================================\n\n";

    printGrammar();
    printFirst();
    printStates();
    printTable();

    cout << "Conflict check:\n";
    if (conflicts.empty()) {
        cout << "   No shift/reduce or reduce/reduce conflicts were found.\n";
        cout << "   The grammar is Canonical LR(1).\n\n";
    } else {
        cout << "   " << conflicts.size() << " conflict(s) detected:\n";
        for (const string& c : conflicts) cout << "     - " << c << "\n";
        cout << "\n";
    }

    // ---- Test strings -----------------------------------------------------
    // In this grammar '*' is a prefix (dereference) operator, so the well-formed
    // rendering of the assignment string "id = id * id" is "id = * id" (id = *id).
    parse({"id", "=", "*", "id"}, "id = * id");

    // The literal three-id string is correctly rejected: '*' cannot sit between
    // two ids in this grammar.
    parse({"id", "=", "id", "*", "id"}, "id = id * id");

    return 0;
}
