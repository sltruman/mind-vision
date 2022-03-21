#ifndef MINDVISION_H
#define MINDVISION_H

#ifdef WIN32
#include <Windows.h>
#undef min
#endif

#include "snapshotthread.h"
#include "recordthread.h"

#include <CameraApi.h>
#include <CameraStatus.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <mutex>
using namespace std;

#include <QSharedMemory>

class MindVision
{
public:
    MindVision();
    ~MindVision();
    void list();
    void open(string cameraName);
    void stop();
    void frame();

    //曝光控制
    void exposure(bool full);
    void exposure_mode(int value);
    void brightness(int value);
    void threshold(int value);
    void flicker(int value);
    void gain(int value);
    void gain_range(int minimum,int maximum);
    void exposure_time(int value);
    void exposure_time_range(double minimum,double maximum);
    void frequency(int value);
    void exposure_window(int x,int y,int w,int h);

    //颜色调整
    void white_balance();
    void white_balance_mode(int index);
    void color_temrature(int index);
    void once_white_balance();
    void white_balance_window(int x,int y,int w,int h);
    void rgb(int r,int g,int b);
    void saturation(int value);
    void monochrome(int enable);
    void inverse(int enable);
    void algorithm(int index);


    //查表变换
    void lookup_table_mode();
    void lookup_table_mode(int index);
    void lookup_tables_for_dynamic();
    void gamma(int value);
    void contrast_ratio(int value);
    void lookup_tables_for_preset();
    void lookup_table_preset(int index);
    void lookup_tables_for_custom(int index);

    //图形变换
    void transform();
    void horizontal_mirror(int hard,int value);
    void vertical_mirror(int hard,int value);
    void acutance(int value);
    void noise(int enable);
    void noise3d(int enable,int value);
    void rotate(int value);
    void flat_field_corrent(int enable);
    void flat_field_init(int light);
    void flat_field_params_save(string filepath);
    void flat_field_params_load(string filepath);
    void dead_pixels_correct(int enable);
    void dead_pixels(string x_list,string y_list);
    void dead_pixels_analyze_for_bright(int threshold);
    void dead_pixels_analyze_for_dead(int threshold);
    void undistort(int enable);
    void undistory_params(int w,int h,string camera_matrix,string distort_coeffs);

    //视频参数
    void video();
    void frame_rate_speed(int index);
    void frame_rate_limit(int value);
    void video_output_format(int index);
    void raw_output_range(int value);

    //分辨率
    void resolutions();
    void resolution(int index);
    void resolution(int x,int y,int w,int h);

    //IO
    void io();
    void io_mode(string type,int index,int value);
    void io_state(string type,int index,int value);

    //触发控制
    void controls();
    void trigger_mode(int value);
    void once_soft_trigger();
    void trigger_frames(int value);
    void trigger_delay(unsigned int);
    void trigger_interval(unsigned int);
    void outside_trigger_mode(int);
    void outside_trigger_debounce(unsigned int);
    void outside_shutter(int);
    void flash_mode(int value);
    void flash_polarity(int value);
    void flash_delay(unsigned int value);
    void flash_pulse(unsigned int value);

    //配置
    void firmware();
    void name();
    void rename(string name);
    void params_reset();
    void params_save(int value);
    void params_load(int value);
    void params_save_to_file(string filepath);
    void params_load_from_file(string filepath);

    void snapshot_resolution();
    void snapshot_resolution(int index);

    void snapshot_start(string dir,int resolution,int format,int interval);
    void snapshot_state();
    void snapshot_stop();

    void record_start(string dir,int format,int quality,int frames);
    void record_state();
    void record_stop();

    void play();
    void pause();

    void status_sync(string type);
    void status(string type);
    void brightness();

    //fpn
    void fpn_save(string filepath);
    void fpn_load(string filepath);
    void fpn_clear();
    void fpn(int);

    void infrared_thermometry(int index);
    void infrared_color(int index);
    void infrared_display(int index);
    void infrared_shutter(int value);
    void infrared_cool(int value);
    void infrared_emissivity(int value);
    void infrared_sharpen(int value);
    void infrared_dde(int value);
    void infrared_exposure(int value);
    void infrared_factory_check_exposure(int value);
    void infrared_status(bool output);
    void infrared_manual(bool checked,short value);
    void infrared_temperature_check();
    void infrared_temperature_check_stop(bool checked);
    void infrared_factory_check_temperature_check_stop();
    void infrared_shutter_temperature_raise_sample(bool checked);
    void infrared_factory_check_detect(bool checked);
    void infrared_response_rate_sample(bool checked);
    void infrared_temperature_curve_sample(bool checked);

    void infrared_factory_check();

    void infrared_frame_temp_cnt(int value);
    void infrared_params_status();
    void infrared_sample_path(string path);

    void infrared_response_rate_start(int value,string path);
    void infrared_response_rate_status();
    void infrared_response_rate_stop();
    void infrared_load_response_rate_file(string path,string path2);
    void infrared_cover_start(int value,string path);
    void infrared_cover_status();
    void infrared_cover_stop();
    void infrared_load_cover_file(string path,string path2);
    void infrared_save_config(vector<string>);
    void infrared_delete_config();
    void infrared_cmd(string cmd);

    void infrared_osd(bool checked);
    void infrared_temperature_display(bool checked);
    void infrared_temperature_roi_status(bool output);
    void infrared_roi(bool checked,int index,int user_width_start,int user_width_number,int user_high_start,int user_high_number,int user_roi_emissivity);
    void infrared_blackbody_calibrate(bool checked,int blackbody_temprature,int user_width_start,int user_width_end,int user_high_start,int user_high_end);
    void infrared_color_map(bool checked,int low,int high);
    void infrared_temperature_compensation(int value);
    void infrared_distance_compensation(int value);
    void infrared_humidity_compensation(int value);
    void infrared_high_warm(bool checked,int temperature);
    void infrared_low_warm(bool checked,int temperature);

private:
    struct {
        int num = 0;
        int width = 0;
        int height = 0;
        int bits = 0;
        char camera_status[256] = "\0";
        char exposure_status[256] = "\0";
        int record_status = 0;
        int snapshot_status = 0;
        char _[8];
    } frame_head;

    ofstream log;
    streambuf* rdbuf;

    string pipeName;
    int camera;
    tSdkCameraCapbility capability;
    tSdkCameraDevInfo camera_info;

    QSharedMemory sm,infrared_sm,infrared_params_sm,infrared_temperature_roi_sm;

    SnapshotThread st;
    RecordThread rt;

    tSdkFrameHead light_frame_head,dark_frame_head;
    unsigned char *light_buffer,*dark_buffer;

    bool playing;

    stringstream status_string;
    mutex m_status_string;
};

#endif // MINDVISION_H
