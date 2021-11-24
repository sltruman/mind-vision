#include "snapshotthread.h"

SnapshotThread::SnapshotThread()
{

}

void SnapshotThread::run() {

    auto status = CameraSetResolutionForSnap(camera,&capability.pImageSizeDesc[resolution]);
    cerr << status << "CameraSetResolutionForSnap" << endl;
ONCE:
    tSdkFrameHead	frameHead;
    BYTE			*pbyBuffer;

    //CameraSnapToBuffer抓拍一张图像保存到buffer中
    status = CameraSnapToBuffer(camera,&frameHead,&pbyBuffer,2000);
    cerr << status << " CameraSnapToBuffer" << endl;

    if(status != CAMERA_STATUS_SUCCESS) {
        frameHead.iHeight = capability.sResolutionRange.iHeightMax;
        frameHead.iWidth = capability.sResolutionRange.iWidthMax;
        frameHead.uiMediaType = (capability.sIspCapacity.bMonoSensor ? 1 : 3);
    }

    auto time = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
    auto filename = this->dir + "/mindvision" + time.toStdString();

    cerr << CameraSaveImage(camera, const_cast<char*>(filename.c_str()), pbyBuffer, &frameHead, format, 100) << " CameraSaveImage " << filename << endl;
    cerr << CameraReleaseImageBuffer(camera, pbyBuffer) << " CameraReleaseImageBuffer" << endl;

    if(period == -1) return;

    msleep(period);
    goto ONCE;
}
