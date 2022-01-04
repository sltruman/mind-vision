#ifndef RECORDTHREAD_H
#define RECORDTHREAD_H

#ifdef WIN32
#include <Windows.h>
#undef min
#endif

#include <CameraStatus.h>
#include <CameraApi.h>

#include <QThread>
#include <string>
#include <iostream>
#include <ctime>
#include <sstream>

using namespace std;
struct RecordThread : public QThread
{
    RecordThread();
    void run() override;

    bool interrupt;
    string dir;
    int camera,format,quality,frames;
    tSdkCameraCapbility capability;
};

#endif // RECORDTHREAD_H
