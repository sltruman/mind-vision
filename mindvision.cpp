#include "mindvision.h"


MindVision::MindVision() : camera(0)
{
    cerr << CameraSdkInit(1) << " CameraSdkInit" << endl;    //sdk初始化  0 English 1中文
}

MindVision::~MindVision()
{
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
        cout << cameraEnumList[i].acProductSeries << ','
           << cameraEnumList[i].acProductName << ','
           << cameraEnumList[i].acFriendlyName << ','
           << cameraEnumList[i].acLinkName << ','
           << cameraEnumList[i].acDriverVersion << ','
           << cameraEnumList[i].acSensorType << ','
           << cameraEnumList[i].acPortType << ','
           << cameraEnumList[i].acSn << ','
           << cameraEnumList[i].uInstance;

        string series(cameraEnumList[i].acProductSeries);
        std::transform(series.begin(),series.end(),series.begin(),toupper);

        if(series.find("GIGE") != -1) {
            char camIp[16],camMask[16],camGateWay[16],etIp[16],etMask[16],etGateWay[16];
            CameraGigeGetIp(cameraEnumList + i,camIp,camMask,camGateWay,etIp,etMask,etGateWay);
            cout  << ',' << camIp << ',' << camMask << ',' << camGateWay << ',' << etIp << ',' << etMask << ',' << etGateWay;
        }

        cout << endl;
    }

}

void MindVision::open(string cameraName) {
    int                     cameraCounts = 100;
    tSdkCameraDevInfo       cameraEnumList[100];
    CameraEnumerateDevice(cameraEnumList,&cameraCounts);

    tSdkCameraDevInfo* pCameraInfo = nullptr;
    for(auto i=0;i < cameraCounts;i++) {
        if(cameraName == cameraEnumList[i].acSn) {
            pCameraInfo = cameraEnumList + i;
            break;
        }
    }

    if(pCameraInfo == nullptr) {
        cout << "False " << endl;
        throw runtime_error("相机初始化失败！");
    }

    auto coutbuf = cout.rdbuf(cerr.rdbuf());
    auto status = CameraInit(pCameraInfo,-1,-1,&camera);
    cerr << "CameraInitEx2 " << status << endl;
    cout.rdbuf(coutbuf);

    if(status != CAMERA_STATUS_SUCCESS) {
        cout << "False " << endl;
        throw runtime_error("相机初始化失败！");
    }

    status = CameraGetCapability(camera,&capability);
    cerr << status << " CameraGetCapability" << endl;
    if(status != CAMERA_STATUS_SUCCESS) {
        cout << "False " << endl;
        throw runtime_error("相机初始化失败！");
    }

    if(capability.sIspCapacity.bMonoSensor) status = CameraSetIspOutFormat(camera,CAMERA_MEDIA_TYPE_MONO8);
    else status = CameraSetIspOutFormat(camera,CAMERA_MEDIA_TYPE_RGB8);

    cerr << status << " CameraSetIspOutFormat" << endl;

    if(status != CAMERA_STATUS_SUCCESS) {
        cout << "False " << endl;
        throw runtime_error("相机初始化失败！");
    }

    pipeName = cameraName;

    start();
}


void MindVision::test(string cameraName) {
    int                     cameraCounts = 100;
    tSdkCameraDevInfo       cameraEnumList[100];
    CameraEnumerateDevice(cameraEnumList,&cameraCounts);

    tSdkCameraDevInfo* pCameraInfo = nullptr;
    for(auto i=0;i < cameraCounts;i++) {
        if(cameraName == cameraEnumList[i].acSn) {
            pCameraInfo = cameraEnumList + i;
            break;
        }
    }

    if(pCameraInfo == nullptr) {
        cout << "False " << endl;
        throw runtime_error("相机初始化失败！");
    }

    auto status = CameraInit(pCameraInfo,-1,-1,&camera);
    cout << "CameraInitEx2 " << status << endl;

    if(status != CAMERA_STATUS_SUCCESS) {
        cout << "False " << endl;
        throw runtime_error("相机初始化失败！");
    }

    pipeName = cameraName;

    cout << "CameraGetCapability " << CameraGetCapability(camera,&capability) << endl;

    rgbBufferLength = capability.sResolutionRange.iHeightMax * capability.sResolutionRange.iWidthMax * (capability.sIspCapacity.bMonoSensor ? 1 : 3);
    rgbBuffer = new unsigned char[rgbBufferLength];

    cout << "CameraPlay " << CameraPlay(camera) << endl;

    if(capability.sIspCapacity.bMonoSensor) cerr << CameraSetIspOutFormat(camera,CAMERA_MEDIA_TYPE_MONO8) << " CameraSetIspOutFormat" << endl;
    else cout << CameraSetIspOutFormat(camera,CAMERA_MEDIA_TYPE_RGB8) << " CameraSetIspOutFormat" << endl;

    cout << "True " << pipeName << ' ' << endl;

    while(!pipeName.empty()) {
        tSdkFrameHead        frameHead;
        unsigned char* rawBuffer;

        auto status = CameraGetImageBuffer(camera,&frameHead,&rawBuffer,2000);
        cout << status << " CameraGetImageBuffer" << endl;

        if(status == CAMERA_STATUS_SUCCESS) {
            cout << CameraImageProcess(camera,rawBuffer,rgbBuffer,&frameHead) << " CameraImageProcess" << endl;
            cout << CameraReleaseImageBuffer(camera,rawBuffer) << " CameraReleaseImageBuffer" << endl;
        }


        stringstream ss;
        ss << frameHead.iWidth << ' '
           << frameHead.iHeight << ' '
           << (frameHead.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? 1 : 3) << ' ' << endl;

        cout << ss.str() << endl;
        cout << CameraSaveImage(camera,const_cast<char*>(cameraName.c_str()),rgbBuffer,&frameHead,emSdkFileType::FILE_JPG,100) << " CameraSaveImage" << endl;
    }
}

void MindVision::run(){
    QLocalServer::removeServer(pipeName.c_str());
    QLocalServer server;
    server.listen(pipeName.c_str());

    rgbBufferLength = capability.sResolutionRange.iHeightMax * capability.sResolutionRange.iWidthMax * (capability.sIspCapacity.bMonoSensor ? 1 : 3);
    rgbBuffer = new unsigned char[rgbBufferLength];

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

            tSdkFrameHead  frameHead;
            unsigned char* rawBuffer;

            auto status = CameraGetImageBufferPriority(camera,&frameHead,&rawBuffer,2000,CAMERA_GET_IMAGE_PRIORITY_NEWEST);
            cerr << status << " CameraGetImageBuffer" << endl;

            if(status == CAMERA_STATUS_SUCCESS) {
                CameraImageProcess(camera,rawBuffer,rgbBuffer,&frameHead);
                CameraReleaseImageBuffer(camera,rawBuffer);
            } else {
                frameHead.iHeight = capability.sResolutionRange.iHeightMax;
                frameHead.iWidth = capability.sResolutionRange.iWidthMax;
                frameHead.uiMediaType = (capability.sIspCapacity.bMonoSensor ? 1 : 3);
            }

            stringstream ss;
            ss << frameHead.iWidth << ' '
               << frameHead.iHeight << ' '
               << (frameHead.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? 1 : 3) << ' ' << endl;

            rgbBufferLength = frameHead.iHeight * frameHead.iWidth * (frameHead.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? 1 : 3);

            sock->readLine().toStdString();
            sock->write(ss.str().data(),ss.str().size());
            sock->readLine().toStdString();
            sock->write((const char*)rgbBuffer,rgbBufferLength);
        }

        sock->disconnectFromServer();
    }

    server.close();
}

void MindVision::exposure() {
    BOOL            mode = 0;
    int             brightness = 0;
    double          exposureTime = 0;
    int             analogGain = 0;
    BOOL            flicker = 0;
    int             frequencySel = 0;
    double	        expLineTime = 0; //当前的行曝光时间，单位为us

    auto ret = CameraGetAeState(camera,&mode);
    auto status = ret;
    cerr << ret << " CameraGetAeState" << endl;//获得相机当前的曝光模式。
    status += ret = CameraGetAeTarget(camera,&brightness);
    cerr << ret << " CameraGetAeTarget" << endl;//获得自动曝光的亮度目标值。
    status += ret = CameraGetAntiFlick(camera,&flicker);
    cerr << ret << " CameraGetAntiFlick" << endl;//获得自动曝光时抗频闪功能的使能状态。
    status += ret = CameraGetLightFrequency(camera,&frequencySel);
    cerr <<  ret << " CameraGetLightFrequency" << endl;//获得自动曝光时，消频闪的频率选择。
    status += ret = CameraGetAnalogGain(camera,&analogGain);
    cerr << ret  << " CameraGetAnalogGain" << endl;//获得图像信号的模拟增益值。
    status += ret = CameraGetExposureTime(camera,&exposureTime);
    cerr << ret << " CameraGetExposureTime" << endl;//获得相机的曝光时间。

/*
    获得一行的曝光时间。对于CMOS传感器，其曝光
    的单位是按照行来计算的，因此，曝光时间并不能在微秒
    级别连续可调。而是会按照整行来取舍。这个函数的
    作用就是返回CMOS相机曝光一行对应的时间。
*/
    status += ret = CameraGetExposureLineTime(camera, &expLineTime);
    cerr << ret << " CameraGetExposureLineTime" << endl;

    cout << (status ? "False " : "True ")
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
    cerr << CameraSetAeTarget(camera,value) << " CameraSetAeTarget" << endl;
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
    int saturation=0;
    BOOL mode= -1;
    int r=0,g=0,b=0;
    int algorithm = -1;
    BOOL monochrome = 0,inverse = 0;
    int color_temrature = -1;
    int color_temrature_mode;

    cerr << CameraGetWbMode(camera,&mode) << " CameraGetWbMode" << endl;
    cerr << CameraGetGain(camera,&r,&g,&b) << " CameraGetGain" << endl;
    cerr << CameraGetSaturation(camera,&saturation) << " CameraGetSaturation" << endl;
    cerr << CameraGetMonochrome(camera,&monochrome) << " CameraGetMonochrome" << endl;
    cerr << CameraGetInverse(camera,&inverse) << " CameraGetInverse" << endl;
    cerr << CameraGetBayerDecAlgorithm(camera,ISP_PROCESSSOR_PC,&algorithm) << " CameraGetBayerDecAlgorithm" << endl;
    cerr << CameraGetClrTempMode(camera,&color_temrature_mode) << " CameraGetClrTempMode " << color_temrature_mode << endl;
    switch(color_temrature_mode) {
    case CT_MODE_AUTO:
        color_temrature = 0;
        break;
    case CT_MODE_PRESET:
        cerr << CameraGetPresetClrTemp(camera,&color_temrature) << " CameraGetPresetClrTemp " << color_temrature << endl;
        color_temrature += 1;
        break;
    case CT_MODE_USER_DEF:
        color_temrature = 4;
        break;
    }

    cout << "True "
         << mode << ' '
         << capability.sRgbGainRange.iRGainMin << ' ' << capability.sRgbGainRange.iRGainMax << ' ' << r << ' '
         << capability.sRgbGainRange.iGGainMin << ' ' << capability.sRgbGainRange.iGGainMax << ' ' << g << ' '
         << capability.sRgbGainRange.iBGainMin << ' ' << capability.sRgbGainRange.iBGainMax << ' ' << b << ' '
         << capability.sSaturationRange.iMin << ' ' << capability.sSaturationRange.iMax << ' ' << saturation << ' '
         << monochrome << ' ' << inverse << ' ' << algorithm << ' ' << color_temrature << ' '
         << endl;
}

void MindVision::white_balance_mode(int index) {
    cerr << CameraSetWbMode(camera,index) << " CameraSetWbMode" << endl;
    cout << "True " << endl;
}

void MindVision::color_temrature(int index) {
    switch(index) {
    case 0:
        cerr << CameraSetClrTempMode(camera,CT_MODE_AUTO) << " CameraSetClrTempMode " << CT_MODE_AUTO << endl;
        cerr << CameraSetOnceWB(camera) << " CameraSetOnceWB" << endl;
        break;
    case 1:
    case 2:
    case 3:
        cerr << CameraSetClrTempMode(camera,CT_MODE_PRESET) << " CameraSetClrTempMode " << CT_MODE_PRESET << endl;
        cerr << CameraSetPresetClrTemp(camera,index - 1) << " CameraSetPresetClrTemp " << index - 1 << endl;
        break;
    case 4:
        cerr << CameraSetClrTempMode(camera,CT_MODE_USER_DEF) << " CameraSetClrTempMode " << CT_MODE_USER_DEF << endl;
        break;
    }

    cout << "True " << endl;
}

void MindVision::once_white_balance() {
    cerr << CameraSetOnceWB(camera) << " CameraSetOnceWB" << endl;
    cout << "True " << endl;
}

void MindVision::rgb(int r,int g,int b) {
    cerr << CameraSetGain(camera,r,g,b) << " CameraSetGain " << r << ' ' << g << ' ' << b << endl;
    cout << "True " << endl;
}

void MindVision::saturation(int value) {
    cerr << CameraSetSaturation(camera,value) << " CameraSetSaturation " << value << endl;
    cout << "True " << endl;
}

void MindVision::monochrome(int enable) {
    cerr << CameraSetMonochrome(camera,enable) << " CameraSetMonochrome" << endl;
    cout << "True " << endl;
}

void MindVision::inverse(int enable) {
    cerr << CameraSetInverse(camera,enable) << " CameraSetInverse" << endl;
    cout << "True " << endl;
}

void MindVision::algorithm(int index) {
    cerr << CameraSetBayerDecAlgorithm(camera,ISP_PROCESSSOR_PC,index) << " CameraSetBayerDecAlgorithm" << endl;
    cout << "True " << endl;
}

void MindVision::lookup_table_mode() {
    int mode = -1;
    cerr << CameraGetLutMode(camera,&mode) << " CameraGetLutMode" << endl;
    cout << "True "
         << mode << ' ' << endl;
}

void MindVision::lookup_table_mode(int index) {
    cerr << CameraSetLutMode(camera,index) << " CameraSetLutMode" << endl;
    cout << "True " << endl;
}

void MindVision::lookup_tables_for_dynamic() {
    int                 gamma=0;
    int                 contrast=0;
    unsigned short r[4096];

    cerr << CameraGetGamma(camera,&gamma) << " CameraGetGamma" << endl;
    cerr << CameraGetContrast(camera,&contrast) << " CameraGetContrast" << endl;
    cerr << CameraGetCurrentLut(camera,LUT_CHANNEL_ALL,r) << endl;

    stringstream ss;
    for(auto i=0;i < 4096;i++)
        ss << r[i] << ',';
    ss.seekp(-1,ios::end);
    ss << " ";

    cout << "True "
         << capability.sGammaRange.iMin << ' ' << capability.sGammaRange.iMax << ' ' << gamma << ' '
         << capability.sContrastRange.iMin << ' ' << capability.sContrastRange.iMax << ' ' << contrast << ' '
         << ss.str()
         << endl;
}

void MindVision::lookup_tables_for_preset() {
    int preset = -1;
    unsigned short r[4096];
    cerr << CameraGetLutPresetSel(camera,&preset) << " CameraGetLutPresetSel" << endl;
    cerr << CameraGetCurrentLut(camera,LUT_CHANNEL_ALL,r) << " CameraGetCurrentLut" << endl;

    stringstream ss;
    for(auto i=0;i < 4096;i++)
        ss << r[i] << ',';
    ss.seekp(-1,ios::end);
    ss << " ";

    cout << "True "
         << preset << ' '
         << ss.str()
         << endl;
}

void MindVision::lookup_table_preset(int index) {
    cerr << CameraSelectLutPreset(camera,index) << " CameraSelectLutPreset" << endl;
    cout << "True "
         << endl;
}

void MindVision::lookup_tables_for_custom(int index) {
    USHORT r[4096];
    cerr << CameraGetCustomLut(camera,index,r) << endl;

    stringstream ss;
    for(auto i=0;i < 4096;i++)
        ss << r[i] << ',';
    ss.seekp(-1,ios::end); ss << " ";

    cout << "True "
         << ss.str()
         << endl;
}

void MindVision::gamma(int value) {
    CameraSetGamma(camera,value);
    cout << "True " << endl;
}

void MindVision::contrast_ratio(int value) {
    CameraSetContrast(camera,value);
    cout << "True " << endl;
}

void MindVision::transform() {
    BOOL        m_bHflip=FALSE;
    BOOL        m_bVflip=FALSE;
    int         m_Sharpness=0,noise = 0,noise3D = 0,count = 0,weight =0,rotate  = 0;
    float weights ;

    //获得图像的镜像状态。
    CameraGetMirror(camera, MIRROR_DIRECTION_HORIZONTAL, &m_bHflip);
    CameraGetMirror(camera, MIRROR_DIRECTION_VERTICAL,   &m_bVflip);

    //获取当前锐化设定值。
    CameraGetSharpness(camera, &m_Sharpness);
    CameraGetNoiseFilterState(camera,&noise );
    CameraGetDenoise3DParams(camera,&noise3D,&count,&weight,&weights);
    CameraGetRotate(camera,&rotate);

    cout << "True "
         << m_bHflip << ' ' << m_bVflip << ' '
         << capability.sSharpnessRange.iMin << ' ' <<  capability.sSharpnessRange.iMax << ' ' << m_Sharpness << ' '
         << noise << ' ' << noise3D << ' ' << count << ' '
         << rotate << ' '
         << endl;
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

void MindVision::noise(int enable) {
    CameraSetNoiseFilter(camera,enable);
    cout << "True " << endl;
}

void MindVision::noise3d(int enable,int value) {
    CameraSetDenoise3DParams(camera,enable,value,nullptr);
    cout << "True " << endl;
}

void MindVision::rotate(int value) {
    CameraSetRotate(camera,value);
    cout << "True " << endl;
}

void MindVision::video() {
    int speed=0,hz=0;
    CameraGetFrameSpeed(camera,&speed);
    CameraGetFrameRate(camera,&hz);
    cout << "True "
         << speed << ' '
         << hz << ' '
         << endl;
}

void MindVision::frame_rate_speed(int index) {
    CameraSetFrameSpeed(camera,index);
    cout << "True " << endl;
}

void MindVision::frame_rate_limit(int value) {
    CameraSetFrameRate(camera,value);
    cout << "True " << endl;
}

void MindVision::resolutions() {
    tSdkImageResolution resolution;
    CameraGetImageResolution(camera,&resolution);

    stringstream ss;
    for(auto i=0;i < capability.iImageSizeDesc;i++)
        ss << capability.pImageSizeDesc[i].acDescription << ',';
    ss.seekp(-1,ios::end); ss << " ";

    cout << "True" << ' '
         << (resolution.iIndex != 0xFF ? 0 : 1) << ' '
         << ss.str()
         << resolution.iIndex << ' '
         << endl;
}

void MindVision::resolution(int index) {
    CameraSetImageResolution(camera,&capability.pImageSizeDesc[index]);
    cout << "True " << endl;
}

void MindVision::io() {
    int state;
    CameraGetIOState(camera,0,);
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
    cerr << CameraSetTriggerMode(camera,value) << " CameraSetTriggerMode " << value << endl;
    cout << "True " << endl;
}

void MindVision::once_soft_trigger() {
    CameraSoftTrigger(camera);
    cout << "True " << endl;
}

void MindVision::flash_mode(int value) {
    CameraSetStrobeMode(camera,value);
    cout << "True " << endl;
}

void MindVision::flash_polarity(int value) {
    CameraSetStrobePolarity(camera,value);
    cout << "True " << endl;
}

void MindVision::params_reset() {
    cerr << CameraLoadParameter(camera,PARAMETER_TEAM_DEFAULT) << " CameraLoadParameter" << endl;
    cout << "True " << endl;
}

void MindVision::params_save(int value) {
    cerr << CameraSaveParameter(camera,value) << " CameraSaveParameter " << value << endl;
    cout << "True " << endl;
}

void MindVision::params_load(int value) {
    cerr << CameraLoadParameter(camera,value) << " CameraLoadParameter "<< value << endl;
    cout << "True " << endl;
}

void MindVision::params_save_to_file(string filename) {
    cerr << CameraSaveParameterToFile(camera,const_cast<char*>(filename.c_str())) << " CameraSaveParameterToFile " << filename << endl;
    cout << "True " << endl;
}

void MindVision::params_load_from_file(string filename) {
    cerr << CameraReadParameterFromFile(camera,const_cast<char*>(filename.c_str())) << " CameraReadParameterFromFile " << filename<< endl;
    cout << "True " << endl;
}

void MindVision::snapshot_resolution() {
    tSdkImageResolution resolution;
    CameraGetResolutionForSnap(camera,&resolution);
    cout << "True " << resolution.iIndex << endl;
}

void MindVision::snapshot_resolution(int index) {
    CameraSetResolutionForSnap(camera,&capability.pImageSizeDesc[index]);
    cout << "True " << endl;
}

void MindVision::snapshot_start(string dir,int resolution,int format,int period) {
    st.dir = dir;
    st.format = format;
    st.camera = camera;
    st.capability = capability;
    st.resolution = resolution;
    st.period = period;

    st.start();
    cout << "True " << endl;
}

void MindVision::snapshot_state() {
    cout << "True " << st.isRunning() << " " << endl;
}

void MindVision::snapshot_stop() {
    st.interrupt = true;
    st.terminate();
    st.wait();
    cout << "True " << endl;
}

void MindVision::rename(string name) {
    cerr << CameraSetFriendlyName(camera,const_cast<char*>(name.c_str())) << " CameraSetFriendlyName" << endl;
    cout << "True " << endl;
}

void MindVision::play() {
    cerr << CameraPlay(camera) << " CameraPlay"  << endl;
    cout << "True " << endl;
}

void MindVision::pause() {
    cerr << CameraPause(camera) << " CameraPause"  << endl;
    cout << "True " << endl;
}

void MindVision::stop() {
    cerr << CameraStop(camera) << " CameraStop"  << endl;
    cout << "True " << endl;
}


