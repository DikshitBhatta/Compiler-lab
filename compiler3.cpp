#include <iostream>
#include <vector>
#include <string>
using namespace std;

int main()
{
    string A;
    cout << "Enter the non-terminal: ";
    cin >> A;

    int n;
    cout << "Enter the number of productions: ";
    cin >> n;

    cout << "Enter the productions (right-hand side only, e.g. " << A << "a or b):\n";
    vector<string> alpha;   // parts that follow A in left-recursive productions
    vector<string> beta;    // non-left-recursive productions

    for (int i = 0; i < n; i++) {
        string prod;
        cin >> prod;

        // a production is left recursive if it starts with the non-terminal A
        if (prod.size() >= A.size() && prod.compare(0, A.size(), A) == 0) {
            string rest = prod.substr(A.size());    
            if (rest.empty())
                rest = "";                         
            alpha.push_back(rest);
        } else {
            beta.push_back(prod);
        }
    }

    string Aprime = A + "'";

    // Case 1 : no left recursion at all -> grammar unchanged
    if (alpha.empty()) {
        cout << "\nNo left recursion found.\n";
        cout << A << " -> ";
        for (size_t i = 0; i < beta.size(); i++)
            cout << beta[i] << (i + 1 < beta.size() ? " | " : "\n");
        return 0;
    }

    // Case 2 : every production is left recursive -> cannot be removed
    if (beta.empty()) {
        cout << "\nUnable to remove left recursion: ";
        cout << "no non-left-recursive production exists.\n";
        return 0;
    }

    // Case 3 : standard transformation
    cout << "\nAfter removing left recursion:\n";

    cout << A << " -> ";
    for (size_t i = 0; i < beta.size(); i++)
        cout << beta[i] << Aprime << (i + 1 < beta.size() ? " | " : "\n");

    cout << Aprime << " -> ";
    for (size_t i = 0; i < alpha.size(); i++)
        cout << alpha[i] << Aprime << " | ";
    cout << "e\n";   

    return 0;
}
