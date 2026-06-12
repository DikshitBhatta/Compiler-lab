#include <iostream>
#include <string>

using namespace std;

// transition[state][symbol]; symbol index: 0 -> 'a', 1 -> 'b'
const int transition[4][2] = {
	{1, 0}, // q0
	{1, 2}, // q1
	{1, 3}, // q2
	{1, 0}  // q3
};

const int START_STATE = 0;
const int ACCEPT_STATE = 3;

int main()
{
	string input;

	cout << "Enter string: ";
	cin >> input;

	int state = START_STATE;
	bool valid = true;

	for (char c : input) {
		if (c == 'a') {
			state = transition[state][0];
		} else if (c == 'b') {
			state = transition[state][1];
		} else {
			// symbol outside the alphabet {a, b}
			valid = false;
			break;
		}
	}

	if (valid && state == ACCEPT_STATE) {
		cout << "VALID" << endl;
	} else {
		cout << "INVALID" << endl;
	}

	return 0;
}