#include "mindvision.h"

#include <iostream>
using namespace std;

int main(int argc, char *argv[])
{
    MindVision mv;

    if(string(argv[1]) == "list") {
        mv.list();
        return 0;
    }


    if(string(argv[1]) == "open") {
        mv.open(argv[2]);
    } else {
        return 1;
    }

    string cmd;

    do {
        cin >> cmd;
    } while(cmd != "exit");

    return 0;
}
