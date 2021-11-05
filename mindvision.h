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
