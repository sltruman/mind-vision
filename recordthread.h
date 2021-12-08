#ifndef RECORDTHREAD_H
#define RECORDTHREAD_H

#ifdef WIN32
#include <Windows.h>
#undef min
#endif

#include <CameraStatus.h>
#include <CameraApi.h>

#include <thread>
#include <string>
#include <iostream>
#include <ctime>
#include <sstream>

using namespace std;
struct RecordThread
{
    RecordThread();
    void run();
    void start();
    void stop();
    bool is_running();

    thread task;
    bool running;
    string dir;
    int camera,format,quality,frames;
    tSdkCameraCapbility capability;
};

#endif // RECORDTHREAD_H
