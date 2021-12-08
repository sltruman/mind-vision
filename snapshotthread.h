#ifndef SNAPSHOTTHREAD_H
#define SNAPSHOTTHREAD_H

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

struct SnapshotThread : public QThread
{
    explicit SnapshotThread();
    void run() override;
    bool interrupt;
    string dir;
    int camera,resolution,format,period;
    tSdkCameraCapbility capability;
};

#endif // SNAPSHOTTHREAD_H
