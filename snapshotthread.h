#ifndef SNAPSHOTTHREAD_H
#define SNAPSHOTTHREAD_H

#include <CameraApi.h>
#include <QThread>
#include <QDateTime>
#include <string>
#include <iostream>
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
