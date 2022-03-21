#include "mindvision.h"

#include <iostream>
#include <fstream>
#include <mutex>
using namespace std;

#include <QStandardPaths>

int main(int argc, char *argv[])
{
    ofstream log;
    string cmd = argv[1];

    MindVision mv;

    if(cmd == "list") {
        mv.list();
        return 0;
    } else if(cmd == "open") {
        try {
            auto dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation).toLocal8Bit().data();
            log.open(string(dir) + "/" + string(argv[2]) + ".log");
            cerr.rdbuf(log.rdbuf());
            mv.open(argv[2]);
            cmd.clear();
        } catch(...) {
            return 1;
        }
    } else {
        return 0;
    }

    thread async_input([&]() { while(!cin.eof() && cmd != "exit") if(cmd.empty()) cin >> cmd; });

    while(!cin.eof() && cmd != "exit") {
        if(cmd.empty()) {
            mv.frame();
            continue;
        }

        if(cmd == "exposure") {
            int full; cin >> full; mv.exposure(full);
        } else if(cmd == "exposure-mode-set") {
            int value; cin >> value; mv.exposure_mode(value);
        } else if(cmd == "brightness-set") {
            int value; cin >> value; mv.brightness(value);
        } else if(cmd == "threshold-set") {
            int value; cin >> value; mv.threshold(value);
        } else if(cmd == "flicker-set") {
            int value; cin >> value; mv.flicker(value);
        } else if(cmd == "gain-set") {
            int value; cin >> value; mv.gain(value);
        } else if(cmd == "gain-range-set") {
            int minimum,maximum; cin >> minimum >> maximum; mv.gain_range(minimum,maximum);
        } else if(cmd == "exposure-time-set") {
            int value; cin >> value; mv.exposure_time(value);
        } else if(cmd == "exposure-time-range-set") {
            double minimum,maximum; cin >> minimum >> maximum; mv.exposure_time_range(minimum,maximum);
        } else if(cmd == "frequency-set") {
            int value; cin >> value; mv.frequency(value);
        } else if(cmd == "exposure-window-set") {
            int x,y,w,h; cin >> x >> y >> w >> h; mv.exposure_window(x,y,w,h);
        } else if(cmd == "white-balance") {
            mv.white_balance();
        } else if(cmd == "white-balance-mode-set") {
            int value; cin >> value; mv.white_balance_mode(value);
        } else if(cmd == "once-white-balance") {
            mv.once_white_balance();
        } else if(cmd == "white-balance-window-set") {
            int x,y,w,h; cin >> x >> y >> w >> h; mv.white_balance_window(x,y,w,h);
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
        } else if(cmd == "resolution-custom-set") {
            int x,y,w,h; cin >> x >> y >> w >> h; mv.resolution(x,y,w,h);
        } else if(cmd == "transform") {
            mv.transform();
        } else if(cmd == "horizontal-mirror-set") {
            int hard,value; cin >> hard >> value; mv.horizontal_mirror(hard,value);
        } else if(cmd == "vertical-mirror-set") {
            int hard,value; cin >> hard >> value; mv.vertical_mirror(hard,value);
        } else if(cmd == "acutance-set") {
            int value; cin >> value; mv.acutance(value);
        } else if(cmd == "noise-set") {
            int enable; cin >> enable; mv.noise(enable);
        } else if(cmd == "noise3d-set") {
            int enable,count; cin >> enable >> count; mv.noise3d(enable,count);
        } else if(cmd == "rotate-set") {
            int value; cin >> value; mv.rotate(value);
        } else if(cmd == "flat-field-corrent-set") {
            int enable; cin >> enable; mv.flat_field_corrent(enable);
        } else if(cmd == "flat-field-init") {
            int light; cin >> light; mv.flat_field_init(light);
        } else if(cmd == "flat-field-params-save") {
            string filepath; cin >> filepath; mv.flat_field_params_save(filepath);
        } else if(cmd == "flat-field-params-load") {
            string filepath; cin >> filepath; mv.flat_field_params_load(filepath);
        } else if(cmd == "dead-pixels-correct-set") {
            int enable; cin >> enable; mv.dead_pixels_correct(enable);
        } else if(cmd == "dead-pixels-set") {
            string x,y; cin >> x >> y;mv.dead_pixels(x,y);
        } else if(cmd == "dead-pixels-analyze-for-bright") {
            int threshold; cin >> threshold; mv.dead_pixels_analyze_for_bright(threshold);
        } else if(cmd == "dead-pixels-analyze-for-dead") {
            int threshold; cin >> threshold; mv.dead_pixels_analyze_for_dead(threshold);
        } else if(cmd == "undistort-set") {
            int enable; cin >> enable;
            mv.undistort(enable);
        } else if(cmd == "undistort-params-set") {
            int w,h; string camera_matrix,distort_coeffs; cin >> w >> h >> camera_matrix >> distort_coeffs;
            mv.undistory_params(w,h,camera_matrix,distort_coeffs);
        } else if(cmd == "video") {
            mv.video();
        } else if(cmd == "frame-rate-speed-set") {
            int index; cin >> index; mv.frame_rate_speed(index);
        } else if(cmd == "frame-rate-limit-set") {
            int value; cin >> value; mv.frame_rate_limit(value);
        } else if(cmd == "video-output-format-set") {
            int index; cin >> index; mv.video_output_format(index);
        } else if(cmd == "raw-output-range-set") {
            int value; cin >> value; mv.raw_output_range(value);
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
        } else if(cmd == "outside-trigger-mode-set") {
            int value; cin >> value; mv.outside_trigger_mode(value);
        } else if(cmd == "outside-trigger-debounce-set") {
            int value; cin >> value; mv.outside_trigger_debounce(value);
        } else if(cmd == "outside-shutter-set") {
            int index; cin >> index; mv.outside_shutter(index);
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
        } else if(cmd == "name") {
            mv.name();
        } else if(cmd == "rename") {
            string name; getline(cin,name); mv.rename(name.substr(1));
        } else if(cmd == "rename-empty") {
            mv.rename("");
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
        } else if(cmd == "snapshot-stop") {
            mv.snapshot_stop();
        } else if(cmd == "record-start") {
            string dir; int format,quality,frames; cin >> dir >> format >> quality>> frames; mv.record_start(dir,format,quality,frames);
        } else if(cmd == "record-stop") {
            mv.record_stop();
        } else if(cmd == "play") {
            mv.play();
        } else if(cmd == "pause") {
            mv.pause();
        } else if(cmd == "status") {
            string type; cin >> type; mv.status(type);
        } else if(cmd == "brightness") {
            mv.brightness();
        } else if(cmd == "fpn-save") {
            string filepath; cin >> filepath; mv.fpn_save(filepath);
        } else if(cmd == "fpn-load") {
            string filepath; cin >> filepath; mv.fpn_load(filepath);
        } else if(cmd == "fpn-clear") {
            mv.fpn_clear();
        } else if(cmd == "fpn") {
            int enable; cin >> enable; mv.fpn(enable);
        }

        if(cmd == "infrared-thermometry") {
            int index; cin >> index; mv.infrared_thermometry(index);
        } else if(cmd == "infrared-color") {
            int index; cin >> index; mv.infrared_color(index);
        } else if(cmd == "infrared-display") {
            int index; cin >> index; mv.infrared_display(index);
        } else if(cmd == "infrared-shutter") {
            int value; cin >> value; mv.infrared_shutter(value);
        } else if(cmd == "infrared-cool") {
            int value; cin >> value; mv.infrared_cool(value);
        } else if(cmd == "infrared-emissivity") {
            int value; cin >> value; mv.infrared_emissivity(value);
        } else if(cmd == "infrared-sharpen") {
            int value; cin >> value; mv.infrared_sharpen(value);
        } else if(cmd == "infrared-dde") {
            int value; cin >> value; mv.infrared_dde(value);
        } else if(cmd == "infrared-exposure") {
            int value; cin >> value; mv.infrared_exposure(value);
        } else if(cmd == "infrared-status") {
            int value; cin >> value; mv.infrared_status(value);
        } else if(cmd == "infrared-manual") {
            bool checked;short value; cin >> checked >> value; mv.infrared_manual(checked,value);
        } else if(cmd == "infrared-temperature-check") {
            mv.infrared_temperature_check();
        } else if(cmd == "infrared-stop-temperature-check") {
            bool checked; cin >> checked; mv.infrared_temperature_check_stop(checked);
        } else if(cmd == "infrared_factory_check") {
            mv.infrared_factory_check();
        } else if(cmd == "infrared_factory_check_temperature_check_stop") {
            mv.infrared_factory_check_temperature_check_stop();
        } else if(cmd == "infrared-shutter-temperature-raise-sample") {
            bool checked; cin >> checked; mv.infrared_shutter_temperature_raise_sample(checked);
        } else if(cmd == "infrared-factory-check-detect") {
            bool checked; cin >> checked; mv.infrared_factory_check_detect(checked);
        } else if(cmd == "infrared-response-rate-sample") {
            bool checked; cin >> checked; mv.infrared_response_rate_sample(checked);
        } else if(cmd == "infrared_temperature_curve_sample") {
            bool checked; cin >> checked; mv.infrared_temperature_curve_sample(checked);
        } else if(cmd == "infrared_frame_temp_cnt") {
            int value; cin >> value; mv.infrared_frame_temp_cnt(value);
        } else if(cmd == "infrared_factory_check_exposure") {
            int value; cin >> value; mv.infrared_factory_check_exposure(value);
        } else if(cmd == "infrared_factory_check") {
            mv.infrared_factory_check();
        } else if(cmd == "infrared_sample_path") {
            string path; cin >> path; mv.infrared_sample_path(path);
        } else if(cmd == "infrared_params_status") {
            mv.infrared_params_status();
        } else if(cmd == "infrared_response_rate_start") {
            int value; string path; cin >> value >> path; mv.infrared_response_rate_start(value,path);
        } else if(cmd == "infrared_response_rate_status") {
            mv.infrared_response_rate_status();
        } else if(cmd == "infrared_response_rate_stop") {
            mv.infrared_response_rate_stop();
        } else if(cmd == "infrared_load_response_rate_file") {
            string path, path2; cin >> path >> path2; mv.infrared_load_response_rate_file(path,path2);
        } else if(cmd == "infrared_cover_start") {
            int value; string path; cin >> value >> path; mv.infrared_cover_start(value,path);
        } else if(cmd == "infrared_cover_status") {
            mv.infrared_cover_status();
        } else if(cmd == "infrared_cover_stop") {
            mv.infrared_cover_stop();
        } else if(cmd == "infrared_load_cover_file") {
            string path, path2; cin >> path >> path2; mv.infrared_load_cover_file(path,path2);
        } else if(cmd == "infrared_save_config") {
            int count;
            cin >> count;
            vector<string> filenames;
            for(int i=0;i<count;i++) {
                string filename; cin >> filename;
                filenames.emplace_back(filename);
            }

            mv.infrared_save_config(filenames);
        } else if(cmd == "infrared_delete_config") {
            mv.infrared_delete_config();
        } else if(cmd == "infrared_cmd") {
            string cmd;
            getline(cin,cmd);
            mv.infrared_cmd(cmd);
        } else if(cmd == "infrared_osd") {
            bool checked; cin >> checked; mv.infrared_osd(checked);
        } else if(cmd == "infrared_temperature_display") {
            bool checked; cin >> checked; mv.infrared_temperature_display(checked);
        } else if(cmd == "infrared_temperature_roi_status") {
            int value; cin >> value; mv.infrared_temperature_roi_status(value);
        } else if(cmd == "infrared_roi") {
            bool checked;int index,user_width_start,user_width_number,user_high_start,user_high_number,user_roi_emissivity;
            cin >> checked >> index >> user_width_start >> user_width_number >> user_high_start >> user_high_number >> user_roi_emissivity;
            mv.infrared_roi(checked,index,user_width_start,user_width_number,user_high_start,user_high_number,user_roi_emissivity);
        } else if(cmd == "infrared_color_map") {
            bool checked;int low,high; cin >> checked >> low >> high; mv.infrared_color_map(checked,low,high);
        } else if(cmd == "infrared_blackbody_calibrate") {
            bool checked;int blackbody_temprature,user_width_start,user_width_number,user_high_start,user_high_number;
            cin >> checked >> blackbody_temprature >> user_width_start >> user_width_number >> user_high_start >> user_high_number;
            mv.infrared_blackbody_calibrate(checked,blackbody_temprature,user_width_start,user_width_number,user_high_start,user_high_number);
        } else if(cmd == "infrared_temperature_compensation") {
            int value; cin >> value; mv.infrared_temperature_compensation(value);
        } else if(cmd == "infrared_distance_compensation") {
            int value; cin >> value; mv.infrared_distance_compensation(value);
        } else if(cmd == "infrared_humidity_compensation") {
            int value; cin >> value; mv.infrared_humidity_compensation(value);
        } else if(cmd == "infrared_high_warm") {
            bool checked;int blackbody_temprature;
            cin >> checked >> blackbody_temprature;
            mv.infrared_high_warm(checked,blackbody_temprature);
        } else if(cmd == "infrared_low_warm") {
            bool checked;int blackbody_temprature;
            cin >> checked >> blackbody_temprature;
            mv.infrared_low_warm(checked,blackbody_temprature);
        }

        cmd.clear();
    }

    async_input.join();

    mv.stop();
    log.close();
    return 0;
}
