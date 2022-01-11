#include "snapshotthread.h"


SnapshotThread::SnapshotThread()
{

}

void SnapshotThread::run() {

//    auto status = CameraSetResolutionForSnap(camera,&capability.pImageSizeDesc[resolution]);
//    cerr << status << " CameraSetResolutionForSnap" << endl;
    auto rgbBufferLength = capability.sResolutionRange.iHeightMax * capability.sResolutionRange.iWidthMax * (capability.sIspCapacity.bMonoSensor ? 1 : 3);
    auto rgbBuffer = new unsigned char[rgbBufferLength];

    tSdkFrameHead	frameHead;
    BYTE			*rawBuffer;
ONCE:
    auto status = CameraGetImageBufferPriority(camera,&frameHead,&rawBuffer,2000,CAMERA_GET_IMAGE_PRIORITY_NEXT);
    cerr << status << " CameraGetImageBuffer " << frameHead.uiMediaType << " " << frameHead.iWidth << " " << frameHead.iHeight << endl;

    if(status == CAMERA_STATUS_SUCCESS) {
        stringstream filename;
        filename << this->dir << "/mind-vision-" << std::time(0);

        cerr << CameraImageProcess(camera,rawBuffer,rgbBuffer,&frameHead) << " CameraImageProcess" << endl;
//        cerr << CameraFlipFrameBuffer(rgbBuffer,&frameHead,1) << " CameraFlipFrameBuffer" << endl;
        cerr << CameraSaveImage(camera, const_cast<char*>(filename.str().c_str()), rgbBuffer, &frameHead, format, 100) << " CameraSaveImage " << filename.str() << " " << format << endl;
        cerr << CameraReleaseImageBuffer(camera, rawBuffer) << " CameraReleaseImageBuffer" << endl;
    } else {

    }

    if(period == -1 || interrupt) {
        delete[] rgbBuffer;
        return;
    }

    msleep(period);

    goto ONCE;
}
