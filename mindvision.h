#ifndef MINDVISION_H
#define MINDVISION_H

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
    void once_white_balance();
    void r(int value);
    void g(int value);
    void b(int value);
    void saturation(int value);

    void lookup_tables();
    void gamma(int value);
    void contrast_ratio(int value);

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

private:
    ofstream log;
    typeof(cerr.rdbuf()) rdbuf;

    string pipeName;
    int camera;
    tSdkCameraCapbility capability;

    unsigned char* rgbBuffer;
    int rgbBufferLength;
};

#endif // MINDVISION_H
