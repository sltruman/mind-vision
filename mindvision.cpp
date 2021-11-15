#include "mindvision.h"


MindVision::MindVision():camera(0)
{
    cerr << "CameraSdkInit " << CameraSdkInit(1) << endl;    //sdk初始化  0 English 1中文
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
        cout << "False " << endl;
        throw runtime_error("相机初始化失败！");
    }

    pipeName = cameraName;
    start();
}

void MindVision::run(){
    QLocalServer::removeServer(pipeName.c_str());
    QLocalServer server;
    server.listen(pipeName.c_str());

    cerr << "CameraGetCapability " << CameraGetCapability(camera,&capability) << endl;

    rgbBufferLength = capability.sResolutionRange.iHeightMax * capability.sResolutionRange.iWidthMax * (capability.sIspCapacity.bMonoSensor ? 1 : 3);
    rgbBuffer = new unsigned char[rgbBufferLength];

    cerr << "CameraPlay " << CameraPlay(camera) << endl;

    if(capability.sIspCapacity.bMonoSensor) cerr << CameraSetIspOutFormat(camera,CAMERA_MEDIA_TYPE_MONO8) << endl;
    else cerr << CameraSetIspOutFormat(camera,CAMERA_MEDIA_TYPE_RGB8) << endl;

    cout << "True " << pipeName << ' ' << endl;

    while(server.isListening() && !pipeName.empty()) {
        if(!server.waitForNewConnection(1000)) {
            continue;
        }

        auto sock = server.nextPendingConnection();

        while(QLocalSocket::ConnectedState == sock->state() && !pipeName.empty()) {
            if (!sock->waitForReadyRead(1000)) {
                continue;
            }

            auto c = sock->readLine();
            tSdkFrameHead        frameHead;
            unsigned char* rawBuffer;

            cerr << CameraGetImageBuffer(camera,&frameHead,&rawBuffer,10000) << ' ';
            cerr << CameraImageProcess(camera,rawBuffer,rgbBuffer,&frameHead) << ' ';
            cerr << CameraReleaseImageBuffer(camera,rawBuffer) << endl;

            stringstream ss;
            ss << frameHead.iWidth << ' '
               << frameHead.iHeight << ' '
               << (frameHead.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? 1 : 3) << ' ' << endl;

            cerr << ss.str() << endl;

            sock->write(ss.str().data(),ss.str().size());

            rgbBufferLength = frameHead.iHeight * frameHead.iWidth * (frameHead.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? 1 : 3);
            sock->write((const char*)rgbBuffer,rgbBufferLength);
        }

        sock->disconnectFromServer();
    }

    server.close();
}

void MindVision::exposure() {
    BOOL            mode;
    int             brightness;
    double          exposureTime;
    int             analogGain;
    BOOL            flicker;
    int             frequencySel;
    double	        expLineTime; //当前的行曝光时间，单位为us

    CameraGetAeState(camera,&mode);//获得相机当前的曝光模式。
    CameraGetAeTarget(camera,&brightness);//获得自动曝光的亮度目标值。
    CameraGetAntiFlick(camera,&flicker);//获得自动曝光时抗频闪功能的使能状态。
    CameraGetLightFrequency(camera,&frequencySel);//获得自动曝光时，消频闪的频率选择。
    CameraGetAnalogGain(camera,&analogGain);//获得图像信号的模拟增益值。
    CameraGetExposureTime(camera,&exposureTime);//获得相机的曝光时间。

/*
    获得一行的曝光时间。对于CMOS传感器，其曝光
    的单位是按照行来计算的，因此，曝光时间并不能在微秒
    级别连续可调。而是会按照整行来取舍。这个函数的
    作用就是返回CMOS相机曝光一行对应的时间。
*/
    CameraGetExposureLineTime(camera, &expLineTime);

    cout << "True "
         << mode << ' '
         << capability.sExposeDesc.uiTargetMin << ' ' << capability.sExposeDesc.uiTargetMax << ' ' << brightness << ' '
         << flicker << ' '
         << frequencySel << ' '
         << capability.sExposeDesc.uiAnalogGainMin << ' ' << capability.sExposeDesc.uiAnalogGainMax << ' ' << analogGain << ' '
         << capability.sExposeDesc.uiExposeTimeMin << ' ' << capability.sExposeDesc.uiExposeTimeMax << ' ' << exposureTime / expLineTime << ' ' << endl;
}

void MindVision::exposure_mode(int value) {
    CameraSetAeState(camera,value);
    cout << "True " << endl;
}

void MindVision::brightness(int value) {
    CameraSetAeTarget(camera,value);
    cout << "True " << endl;
}

void MindVision::flicker(int value) {
    CameraSetAntiFlick(camera,value);
    cout << "True " << endl;
}

void MindVision::gain(int value) {
    CameraSetAnalogGain(camera,value);
    cout << "True " << endl;
}

void MindVision::exposure_time(int value) {
    double          expLineTime;
    CameraGetExposureLineTime(camera, &expLineTime);
    CameraSetExposureTime(camera,value * expLineTime);
    cout << "True " << endl;
}

void MindVision::frequency(int value) {
    CameraSetLightFrequency(camera,value);
    cout << "True " << endl;
}

void MindVision::white_balance() {
    int saturation;
    BOOL mode;
    int r,g,b;

    CameraGetWbMode(camera,&mode);
    CameraGetGain(camera,&r,&g,&b);
    CameraGetSaturation(camera,&saturation);

    cout << "True "
         << mode << ' '
         << capability.sRgbGainRange.iRGainMin << ' ' << capability.sRgbGainRange.iRGainMax << ' ' << r << ' '
         << capability.sRgbGainRange.iGGainMin << ' ' << capability.sRgbGainRange.iGGainMax << ' ' << g << ' '
         << capability.sRgbGainRange.iBGainMin << ' ' << capability.sRgbGainRange.iBGainMax << ' ' << b << ' '
         << capability.sSaturationRange.iMin << ' ' << capability.sSaturationRange.iMax << ' ' << saturation << ' ' << endl;
}

void MindVision::white_balance_mode(int index){
    CameraSetWbMode(camera,index);
    cout << "True " << endl;
}

void MindVision::once_white_balance(){
    CameraSetOnceWB(camera);
    cout << "True " << endl;
}

void MindVision::r(int value){
    int r,g,b;
    CameraGetGain(camera,&r,&g,&b);
    CameraSetGain(camera,value,g,b);
    cout << "True " << endl;
}

void MindVision::g(int value){
    int r,g,b;
    CameraGetGain(camera,&r,&g,&b);
    CameraSetGain(camera,r,value,b);
    cout << "True " << endl;
}

void MindVision::b(int value){
    int r,g,b;
    CameraGetGain(camera,&r,&g,&b);
    CameraSetGain(camera,r,g,value);
    cout << "True " << endl;
}

void MindVision::saturation(int value) {
    CameraSetSaturation(camera,value);
    cout << "True " << endl;
}

void MindVision::lookup_tables() {
    int                 gamma=0;
    int                 contrast=0;

    CameraGetGamma(camera,&gamma);
    CameraGetContrast(camera,&contrast);

    cout << "True "
         << capability.sGammaRange.iMin << ' ' << capability.sGammaRange.iMax << ' ' << gamma << ' '
         << capability.sContrastRange.iMin << ' ' << capability.sContrastRange.iMax << ' ' << contrast << ' ' << endl;
}

void MindVision::gamma(int value) {
    CameraSetGamma(camera,value);
    cout << "True " << endl;
}

void MindVision::contrast_ratio(int value) {
    CameraSetContrast(camera,value);
    cout << "True " << endl;
}

void MindVision::resolutions() {
    stringstream ss;
    for(auto i=0;i < capability.iImageSizeDesc;i++) {
        ss << capability.pImageSizeDesc[i].acDescription << ' ' << endl;
    }
    cout << ss.str();
}

void MindVision::resolution() {
    tSdkImageResolution resolution = { 0 };
    CameraGetImageResolution(camera,&resolution);
    cout << "True " << resolution.iIndex << endl;
}

void MindVision::resolution(int index) {
    CameraSetImageResolution(camera,&capability.pImageSizeDesc[index]);
    cout << "True " << endl;
}

void MindVision::isp() {
    BOOL        m_bHflip=FALSE;
    BOOL        m_bVflip=FALSE;
    int         m_Sharpness=0;

    //获得图像的镜像状态。
    CameraGetMirror(camera, MIRROR_DIRECTION_HORIZONTAL, &m_bHflip);
    CameraGetMirror(camera, MIRROR_DIRECTION_VERTICAL,   &m_bVflip);

    //获取当前锐化设定值。
    CameraGetSharpness(camera, &m_Sharpness);

    cout << "True "
         << m_bHflip << ' ' << m_bVflip << ' '
         << capability.sSharpnessRange.iMin << ' ' <<  capability.sSharpnessRange.iMax << ' ' << m_Sharpness << ' ' << endl;
}

void MindVision::horizontal_mirror(int value) {
    CameraSetMirror(camera, MIRROR_DIRECTION_HORIZONTAL, value);
    cout << "True " << endl;
}

void MindVision::vertical_mirror(int value) {
    CameraSetMirror(camera, MIRROR_DIRECTION_VERTICAL, value);
    cout << "True " << endl;
}

void MindVision::acutance(int value) {
    CameraSetSharpness(camera,value);
    cout << "True " << endl;
}

void MindVision::controls() {
    int  pbySnapMode;
    int StrobeMode=0;
    int  uPolarity=0;

    CameraGetTriggerMode(camera,&pbySnapMode);
    CameraGetStrobeMode(camera,&StrobeMode);
    CameraGetStrobePolarity(camera,&uPolarity);

    cout << "True "
         << pbySnapMode << ' '
         << StrobeMode << ' '
         << uPolarity << ' ' << endl;
}

void MindVision::trigger_mode(int value) {
    CameraSetTriggerMode(camera,value);
    cout << "True " << endl;
}

void MindVision::once_soft_trigger() {
    CameraSoftTrigger(camera);
}

void MindVision::flash_mode(int value) {
    CameraSetStrobeMode(camera,value);
}

void MindVision::flash_polarity(int value) {
    CameraSetStrobePolarity(camera,value);
}

