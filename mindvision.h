#ifndef MINDVISION_H
#define MINDVISION_H

#ifdef WIN32
#include <Windows.h>
#undef min
#endif

#include "snapshotthread.h"

#include <CameraApi.h>
#include <CameraStatus.h>
#include <QLocalServer>
#include <QLocalSocket>
#include <QThread>

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

class MindVision : public QThread
{
public:
    MindVision();
    ~MindVision();
    void list();
    void open(string cameraName);
    void test(string cameraName);
    void stop();
    void run() override;

    void exposure();
    void exposure_mode(int value);
    void brightness(int value);
    void flicker(int value);
    void gain(int value);
    void exposure_time(int value);
    void frequency(int value);

    void white_balance();
    void white_balance_mode(int index);
    void color_temrature(int index);
    void once_white_balance();
    void rgb(int r,int g,int b);
    void saturation(int value);
    void monochrome(int enable);
    void inverse(int enable);
    void algorithm(int index);

    void lookup_table_mode();
    void lookup_table_mode(int index);
    void lookup_tables_for_dynamic();
    void gamma(int value);
    void contrast_ratio(int value);
    void lookup_tables_for_preset();
    void lookup_table_preset(int index);
    void lookup_tables_for_custom(int index);


    void resolutions();
    void resolution();
    void resolution(int index);

    void isp();
    void horizontal_mirror(int value);
    void vertical_mirror(int value);
    void acutance(int value);

    void controls();
    void trigger_mode(int value);
    void once_soft_trigger();
    void flash_mode(int value);
    void flash_polarity(int value);

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

    void rename(string name);
private:
    ofstream log;
    streambuf* rdbuf;

    string pipeName;
    int camera;
    tSdkCameraCapbility capability;

    unsigned char* rgbBuffer;
    int rgbBufferLength;

    SnapshotThread st;
};

#endif // MINDVISION_H
