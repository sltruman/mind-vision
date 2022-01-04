#include "snapshotthread.h"


SnapshotThread::SnapshotThread()
{

}

void SnapshotThread::run() {

//    auto status = CameraSetResolutionForSnap(camera,&capability.pImageSizeDesc[resolution]);
//    cerr << status << " CameraSetResolutionForSnap" << endl;
ONCE:
    tSdkFrameHead	frameHead;
    BYTE			*pbyBuffer;

    auto status = CameraSnapToBuffer(camera,&frameHead,&pbyBuffer,2000);
    cerr << status << " CameraSnapToBuffer " << frameHead.uiMediaType << " " << frameHead.iWidth << " " << frameHead.iHeight << endl;

    if(status != CAMERA_STATUS_SUCCESS) {
        frameHead.iHeight = capability.sResolutionRange.iHeightMax;
        frameHead.iWidth = capability.sResolutionRange.iWidthMax;
        frameHead.uiMediaType = (capability.sIspCapacity.bMonoSensor ? 1 : 3);
    }

    stringstream filename;
    filename << this->dir << "/mind-vision-" << std::time(0);

    cerr << CameraSaveImage(camera, const_cast<char*>(filename.str().c_str()), pbyBuffer, &frameHead, format, 100) << " CameraSaveImage " << filename.str() << " " << format << endl;
    cerr << CameraReleaseImageBuffer(camera, pbyBuffer) << " CameraReleaseImageBuffer" << endl;

    if(period == -1 || interrupt) return;

    msleep(period);
    goto ONCE;
}
