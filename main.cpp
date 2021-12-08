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

            if(cmd == "exposure") {
                mv.exposure();
            } else if(cmd == "exposure-mode-set") {
                int value; cin >> value; mv.exposure_mode(value);
            } else if(cmd == "brightness-set") {
                int value; cin >> value; mv.brightness(value);
            } else if(cmd == "flicker-set") {
                int value; cin >> value; mv.flicker(value);
            } else if(cmd == "gain-set") {
                int value; cin >> value; mv.gain(value);
            } else if(cmd == "exposure-time-set") {
                int value; cin >> value; mv.exposure_time(value);
            } else if(cmd == "frequency-set") {
                int value; cin >> value; mv.frequency(value);
            } else if(cmd == "white-balance") {
                mv.white_balance();
            } else if(cmd == "white-balance-mode-set") {
                int value; cin >> value; mv.white_balance_mode(value);
            } else if(cmd == "once-white-balance") {
                mv.once_white_balance();
            } else if(cmd == "rgb-set") {
                int r,g,b; cin >> r >> g >> b; mv.rgb(r,g,b);
            } else if(cmd == "saturation-set") {
                int value; cin >> value; mv.saturation(value);
            } else if(cmd == "monochrome-set") {
                int enable; cin >> enable; mv.monochrome(enable);
            } else if(cmd == "inverse-set") {
                int enable; cin >> enable; mv.inverse(enable);
            } else if(cmd == "algorithm-set") {
                int enable; cin >> enable; mv.algorithm(enable);
            } else if(cmd == "color-temrature-set") {
                int index; cin >> index; mv.color_temrature(index);
            } else if(cmd == "lookup-table-mode") {
                mv.lookup_table_mode();
            } else if(cmd == "lookup-table-mode-set") {
                int index; cin >> index; mv.lookup_table_mode(index);
            } else if(cmd == "lookup-tables-for-dynamic") {
                mv.lookup_tables_for_dynamic();
            } else if(cmd == "gamma-set") {
                int value; cin >> value; mv.gamma(value);
            } else if(cmd == "contrast-ratio-set") {
                int value; cin >> value; mv.contrast_ratio(value);
            } else if(cmd == "lookup-tables-for-preset") {
                mv.lookup_tables_for_preset();
            } else if(cmd == "lookup-table-preset-set") {
                int index; cin >> index; mv.lookup_table_preset(index);
            } else if(cmd == "lookup-tables-for-custom") {
                int index; cin >> index; mv.lookup_tables_for_custom(index);
            } else if(cmd == "resolutions") {
                mv.resolutions();
            } else if(cmd == "resolution-set") {
                int value; cin >> value; mv.resolution(value);
            } else if(cmd == "transform") {
                mv.transform();
            } else if(cmd == "horizontal-mirror-set") {
                int value; cin >> value; mv.horizontal_mirror(value);
            } else if(cmd == "vertical-mirror-set") {
                int value; cin >> value; mv.vertical_mirror(value);
            } else if(cmd == "acutance-set") {
                int value; cin >> value; mv.acutance(value);
            } else if(cmd == "noise-set") {
                int enable; cin >> enable; mv.noise(enable);
            } else if(cmd == "noise3d-set") {
                int enable,count; cin >> enable >> count; mv.noise3d(enable,count);
            } else if(cmd == "rotate-set") {
                int value; cin >> value; mv.rotate(value);
            } else if(cmd == "video") {
                mv.video();
            } else if(cmd == "frame-rate-speed-set") {
                int index; cin >> index; mv.frame_rate_speed(index);
            } else if(cmd == "frame-rate-limit-set") {
                int value; cin >> value; mv.frame_rate_limit(value);
            } else if(cmd == "io") {
                mv.io();
            } else if(cmd == "io-mode-set") {
                string type; int index,value; cin >> type >> index >> value; mv.io_mode(type,index,value);
            } else if(cmd == "io-state-set") {
                string type; int index,value; cin >> type >> index >> value; mv.io_state(type,index,value);
            } else if(cmd == "controls") {
                mv.controls();
            } else if(cmd == "trigger-mode-set") {
                int value; cin >> value; mv.trigger_mode(value);
            } else if(cmd == "once-soft-trigger") {
                mv.once_soft_trigger();
            } else if(cmd == "trigger-frames-set") {
                int value; cin >> value; mv.trigger_frames(value);
            } else if(cmd == "trigger-delay-set") {
                int value; cin >> value; mv.trigger_delay(value);
            } else if(cmd == "trigger-interval-set") {
                int value; cin >> value; mv.trigger_interval(value);
            }else if(cmd == "outside-trigger-mode-set") {
                int value; cin >> value; mv.outside_trigger_mode(value);
            }  else if(cmd == "outside-trigger-debounce-set") {
                int value; cin >> value; mv.outside_trigger_debounce(value);
            } else if(cmd == "flash-mode-set") {
                int value; cin >> value; mv.flash_mode(value);
            } else if(cmd == "flash-polarity-set") {
                int value; cin >> value; mv.flash_polarity(value);
            } else if(cmd == "flash-delay-set") {
                int value; cin >> value; mv.flash_delay(value);
            } else if(cmd == "flash-pulse-set") {
                int value; cin >> value; mv.flash_pulse(value);
            } else if(cmd == "firmware") {
                mv.firmware();
            } else if(cmd == "rename") {
                string name; cin >> name; mv.rename(name);
            } else if(cmd == "params-reset") {
                mv.params_reset();
            } else if(cmd == "params-save") {
                int value; cin >> value; mv.params_save(value);
            } else if(cmd == "params-load") {
                int value; cin >> value; mv.params_load(value);
            } else if(cmd == "params-save-to-file") {
                string value; cin >> value; mv.params_save_to_file(value);
            } else if(cmd == "params-load-from-file") {
                string value; cin >> value; mv.params_load_from_file(value);
            } else if(cmd == "snapshot-start") {
                string dir; int resolution,format,period; cin >> dir >> resolution >> format >> period; mv.snapshot_start(dir,resolution,format,period);
            } else if(cmd == "snapshot-state") {
                mv.snapshot_state();
            } else if(cmd == "snapshot-stop") {
                mv.snapshot_stop();
            } else if(cmd == "record-start") {
                string dir; int format,quality,frames; cin >> dir >> format >> quality>> frames; mv.record_start(dir,format,quality,frames);
            } else if(cmd == "record-state") {
                mv.record_state();
            } else if(cmd == "record-stop") {
                mv.record_stop();
            } else if(cmd == "play") {
                mv.play();
            } else if(cmd == "pause") {
                mv.pause();
            } else if(cmd == "stop") {
                mv.stop();
            }
        } while(cmd != "exit");
    } catch(exception& e) {}

    cerr << "exit" << endl;
    log.close();
    return 0;
}
