#include "mindvision.h"


MindVision::MindVision():camera(0)
{
    cerr << CameraSdkInit(1) << endl;    //sdk初始化  0 English 1中文
}

MindVision::~MindVision()
{
    cerr.rdbuf(rdbuf);
    log.close();

    if(!camera) return;

    pipeName.clear();
    this->wait();
    CameraUnInit(camera);
    delete[] rgbBuffer;
}

void MindVision::list()
{
    int                     cameraCounts = 100;
    tSdkCameraDevInfo       cameraEnumList[100];

    CameraEnumerateDevice(cameraEnumList,&cameraCounts);

    for(auto i=0;i < cameraCounts;i++) {
        cout << cameraEnumList[i].acProductSeries << ' '
           << cameraEnumList[i].acProductName << ' '
           << cameraEnumList[i].acFriendlyName << ' '
           << cameraEnumList[i].acLinkName << ' '
           << cameraEnumList[i].acDriverVersion << ' '
           << cameraEnumList[i].acSensorType << ' '
           << cameraEnumList[i].acPortType << ' '
           << cameraEnumList[i].acSn << ' '
           << cameraEnumList[i].uInstance;

        if(cameraEnumList[i].acProductSeries == string("GIGE")) {
            char camIp[16],camMask[16],camGateWay[16],etIp[16],etMask[16],etGateWay[16];
            CameraGigeGetIp(cameraEnumList + i,camIp,camMask,camGateWay,etIp,etMask,etGateWay);
            cout  << ' ' << camIp << ' ' << camMask << ' ' << camGateWay << ' ' << etIp << ' ' << etMask << ' ' << etGateWay;
        }

        cout << endl;
    }

}

void MindVision::open(string cameraName) {
    log.open(cameraName + ".log");
    rdbuf = cerr.rdbuf(log.rdbuf());

    auto status = CameraInitEx2(&cameraName[0],&camera);
    cerr << "CameraInitEx2 " << status << endl;
    if(status != CAMERA_STATUS_SUCCESS) {
        cout << "failed " << endl;
        throw runtime_error("相机初始化失败！");
    }


    cerr << "CameraGetCapability " << CameraGetCapability(camera,&capability) << endl;

    rgbBufferLength = capability.sResolutionRange.iHeightMax * capability.sResolutionRange.iWidthMax * (capability.sIspCapacity.bMonoSensor ? 1 : 3);
    rgbBuffer = new unsigned char[rgbBufferLength];

    cerr << "CameraPlay " << CameraPlay(camera) << endl;

    if(capability.sIspCapacity.bMonoSensor) cerr << CameraSetIspOutFormat(camera,CAMERA_MEDIA_TYPE_MONO8) << endl;
    else cerr << CameraSetIspOutFormat(camera,CAMERA_MEDIA_TYPE_RGB8) << endl;

    pipeName = cameraName;
    start();
}

void MindVision::run(){
    QLocalServer::removeServer(pipeName.c_str());
    QLocalServer server;
    server.listen(pipeName.c_str());
    cout << "succeeded " << pipeName << ' ' << endl;
    static int i = 0;
    while(server.isListening() && !pipeName.empty()) {
        if(!server.waitForNewConnection(1000)) {
            cerr << 'l';
            continue;
        }

        cerr << 'n';

        auto sock = server.nextPendingConnection();

        while(QLocalSocket::ConnectedState == sock->state() && !pipeName.empty()) {
            if (!sock->waitForReadyRead(1000)) {
                cerr << 'x' << endl;
                continue;
            }

            auto c = sock->readLine();
            tSdkFrameHead        frameHead;
            unsigned char* rawBuffer;

            cerr << CameraGetImageBuffer(camera,&frameHead,&rawBuffer,10000) << ' ';
            cerr << CameraImageProcess(camera,rawBuffer,rgbBuffer,&frameHead) << ' ';
            cerr << CameraReleaseImageBuffer(camera,rawBuffer) << endl;

            stringstream ss;
            ss << capability.sResolutionRange.iWidthMax << ' '
               << capability.sResolutionRange.iHeightMax << ' '
               << (capability.sIspCapacity.bMonoSensor ? 1 : 3) << ' ' << endl;

            cerr << ss.str() << endl;

            sock->write(ss.str().data(),ss.str().size());
            sock->write((const char*)rgbBuffer,rgbBufferLength);
        }

        sock->disconnectFromServer();
    }

     cerr << 'c';
    server.close();
}
