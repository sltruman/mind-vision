#include "recordthread.h"

RecordThread::RecordThread()
{

}

void RecordThread::run() {
    auto rgbBufferLength = capability.sResolutionRange.iHeightMax * capability.sResolutionRange.iWidthMax * (capability.sIspCapacity.bMonoSensor ? 1 : 3);
    auto rgbBuffer = new unsigned char[rgbBufferLength];

ONCE:
    if(interrupt) {
        delete[] rgbBuffer;
        return;
    }

    tSdkFrameHead	frameHead;
    BYTE			*rawBuffer;

    auto status = CameraGetImageBufferPriority(camera,&frameHead,&rawBuffer,2000,CAMERA_GET_IMAGE_PRIORITY_NEXT);
    cerr << status << " CameraGetImageBuffer " << frameHead.uiMediaType << " " << frameHead.iWidth << " " << frameHead.iHeight << endl;

    if(status != CAMERA_STATUS_SUCCESS) {
        goto ONCE;
    }

    cerr << CameraImageProcess(camera,rawBuffer,rgbBuffer,&frameHead) << " CameraImageProcess" << endl;
    cerr << CameraPushFrame(camera,rgbBuffer,&frameHead) << " CameraPushFrame " << frameHead.uiMediaType << " " << frameHead.iWidth << " " << frameHead.iHeight << endl;
    cerr << CameraReleaseImageBuffer(camera, rawBuffer) << " CameraReleaseImageBuffer" << endl;
    goto ONCE;
}
