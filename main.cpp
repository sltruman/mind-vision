#include "mindvision.h"

#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, char *argv[])
{
    ofstream log;

    MindVision mv;
    string cmd = argv[1];

    try {
        if(cmd == "list") {
            mv.list();
            return 0;
        } else if(cmd == "open") {
            log.open(string(argv[2]) + ".log");
            cerr.rdbuf(log.rdbuf());
            mv.open(argv[2]);
        } else if(cmd == "test") {
            log.open(string(argv[2]) + ".log");
            cerr.rdbuf(log.rdbuf());
            mv.test(argv[2]);
        }

        do {
            if(cin.eof()) break;
            cin >> cmd;

            if(cmd == "trigger-params") {
            } else if(cmd == "exposure") {
                mv.exposure();
            } else if(cmd == "exposure-mode-set") {
                int value; cin >> value;
                mv.exposure_mode(value);
            } else if(cmd == "brightness-set") {
                int value; cin >> value;
                mv.brightness(value);
            } else if(cmd == "flicker-set") {
                int value; cin >> value;
                mv.flicker(value);
            } else if(cmd == "gain-set") {
                int value; cin >> value;
                mv.gain(value);
            } else if(cmd == "exposure-time-set") {
                int value; cin >> value;
                mv.exposure_time(value);
            } else if(cmd == "frequency-set") {
                int value; cin >> value;
                mv.frequency(value);
            } else if(cmd == "white-balance") {
                mv.white_balance();
            } else if(cmd == "white-balance-mode-set") {
                int value; cin >> value;
                mv.white_balance_mode(value);
            } else if(cmd == "once-white-balance") {
                mv.once_white_balance();
            } else if(cmd == "r-set") {
                int value; cin >> value;
                mv.r(value);
            } else if(cmd == "g-set") {
                int value; cin >> value;
                mv.g(value);
            } else if(cmd == "b-set") {
                int value; cin >> value;
                mv.b(value);
            } else if(cmd == "saturation-set") {
                int value; cin >> value;
                mv.saturation(value);
            } else if(cmd == "lookup-tables") {
                mv.lookup_tables();
            } else if(cmd == "gamma-set") {
                int value; cin >> value;
                mv.gamma(value);
            } else if(cmd == "contrast-ratio-set") {
                int value; cin >> value;
                mv.contrast_ratio(value);
            } else if(cmd == "resolutions") {
                mv.resolutions();
            } else if(cmd == "resolution-get") {
                mv.resolution();
            } else if(cmd == "resolution-set") {
                int value; cin >> value;
                mv.resolution(value);
            } else if(cmd == "isp") {
                mv.isp();
            } else if(cmd == "horizontal-mirror-set") {
                int value; cin >> value;
                mv.horizontal_mirror(value);
            } else if(cmd == "vertical-mirror-set") {
                int value; cin >> value;
                mv.vertical_mirror(value);
            } else if(cmd == "acutance-set") {
                int value; cin >> value;
                mv.acutance(value);
            } else if(cmd == "controls") {
                mv.controls();
            } else if(cmd == "trigger-mode") {
                int value; cin >> value;
                mv.trigger_mode(value);
            } else if(cmd == "once-soft-trigger") {
                mv.once_soft_trigger();
            } else if(cmd == "flash-mode") {
                int value; cin >> value;
                mv.flash_mode(value);
            } else if(cmd == "flash-polarity") {
                int value; cin >> value;
                mv.flash_polarity(value);
            } else {
                cout << "False " << endl;
            }
        } while(cmd != "exit");

    } catch(exception& e) {}

    cerr << "exit" << endl;
    log.close();
    return 0;
}
