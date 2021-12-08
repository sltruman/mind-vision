#include "recordthread.h"

RecordThread::RecordThread()
{

}

void RecordThread::run() {
    auto rgbBufferLength = capability.sResolutionRange.iHeightMax * capability.sResolutionRange.iWidthMax * (capability.sIspCapacity.bMonoSensor ? 1 : 3);
    auto rgbBuffer = new unsigned char[rgbBufferLength];

ONCE:
    if(!running) {
        delete[] rgbBuffer;
        return;
    }

    tSdkFrameHead	frameHead;
    BYTE			*rawBuffer;

    auto status = CameraSnapToBuffer(camera,&frameHead,&rawBuffer,2000);
    cerr << status << " CameraSnapToBuffer " << frameHead.uiMediaType << " " << frameHead.iWidth << " " << frameHead.iHeight << endl;

    if(status != CAMERA_STATUS_SUCCESS) {
        goto ONCE;
    }

    cerr << CameraImageProcess(camera,rawBuffer,rgbBuffer,&frameHead) << " CameraImageProcess" << endl;
    cerr << CameraPushFrame(camera,rgbBuffer,&frameHead) << " CameraPushFrame " << frameHead.uiMediaType << " " << frameHead.iWidth << " " << frameHead.iHeight << endl;
    cerr << CameraReleaseImageBuffer(camera, rawBuffer) << " CameraReleaseImageBuffer" << endl;
    goto ONCE;
}

void RecordThread::start() {
    stringstream filename;
    filename << this->dir << "/mind-vision-" << std::time(0);

    cerr << CameraInitRecord(camera,format,const_cast<char*>(filename.str().c_str()),0,quality,frames)
         << " CameraInitRecord " << filename.str() << ' ' << quality << ' ' << frames
         << endl;
    running = true;

    task = thread(&RecordThread::run,this);
}

bool RecordThread::is_running() {
    return running;
}

void RecordThread::stop() {
    running = false;
    CameraStopRecord(camera);
}
