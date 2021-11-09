#include "mindvision.h"

#include <iostream>
#include <QProcess>
using namespace std;

int main(int argc, char *argv[])
{
    MindVision mv;
    string cmd;

    try {
        if(string(argv[1]) == "list") {
            mv.list();
            return 0;
        } else if(string(argv[1]) == "open") {
            mv.open(argv[2]);
        }

        do {
            if(cin.eof()) break;
            cin >> cmd;
        } while(cmd != "exit");
    } catch(exception& e) {
        return 1;
    }

    cerr << "exit" << endl;
    return 0;
}
