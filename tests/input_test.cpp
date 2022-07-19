#include <iostream>
#include <string>

using namespace std;

int main() {
    string message;

    getline(cin, message);

    cout << message << endl;
    cout << message.length() << endl;

    //Removing consecutive whitespace with std::cin >> std::ws
    //Ignoring all leftover characters on the line of input with cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
}