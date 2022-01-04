#include "mindvision.h"


MindVision::MindVision() : camera(0)
  , dark_buffer(nullptr)
  , light_buffer(nullptr)
{
    cerr << CameraSdkInit(1) << " CameraSdkInit" << endl;    //sdk初始化  0 English 1中文
}

MindVision::~MindVision()
{
    if(!camera) return;

    pipeName.clear();
    this->wait();
    CameraUnInit(camera);

    delete dark_buffer;
    delete light_buffer;
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

    camera_info = *pCameraInfo;

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

void MindVision::run() {
    QLocalServer::removeServer(pipeName.c_str());
    QLocalServer server;
    server.listen(pipeName.c_str());

    auto rgbBufferLength = capability.sResolutionRange.iHeightMax * capability.sResolutionRange.iWidthMax * (capability.sIspCapacity.bMonoSensor ? 1 : 3);
    auto rgbBuffer = new unsigned char[rgbBufferLength];

    cout << "True " << pipeName << ' ' << endl;

    while(server.isListening() && !pipeName.empty()) {
        if(!server.waitForNewConnection(1000)) {
            continue;
        }

        auto sock = server.nextPendingConnection();

        while(QLocalSocket::ConnectedState == sock->state() && !pipeName.empty()) {

            if (!sock->waitForReadyRead(1000))
                continue;

            sock->readLine().toStdString();

            tSdkFrameHead  frameHead,frameHead2;
            unsigned char* rawBuffer = nullptr;

            auto status = CameraGetImageBufferPriority(camera,&frameHead,&rawBuffer,2000,CAMERA_GET_IMAGE_PRIORITY_NEWEST);

            cerr << status << " CameraGetImageBuffer " << ios::hex << reinterpret_cast<void*>(rawBuffer) << endl;

            if(status == CAMERA_STATUS_SUCCESS) {
                frameHead2 = frameHead;
                CameraImageProcess(camera,rawBuffer,rgbBuffer,&frameHead);
                CameraFlipFrameBuffer(rgbBuffer,&frameHead,1);
                CameraReleaseImageBuffer(camera,rawBuffer);
            } else {
                frameHead = frameHead2;
            }

            stringstream ss;
            ss << capability.sResolutionRange.iWidthMax << ' '
               << capability.sResolutionRange.iHeightMax << ' '
               << frameHead.iWidth << ' '
               << frameHead.iHeight << ' '
               << (frameHead.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? 1 : 3) << ' ' << endl;

            rgbBufferLength = frameHead.iHeight * frameHead.iWidth * (frameHead.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? 1 : 3);

            sock->write(ss.str().data(),ss.str().size());
            sock->readLine().toStdString();
            sock->write((const char*)rgbBuffer,rgbBufferLength);
            sock->waitForBytesWritten();
        }

        sock->disconnectFromServer();
    }

    delete[] rgbBuffer;

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

    cout << (status ? "False" : "True") << endl
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

void MindVision::gain_range(int minimum,int maximum) {
    CameraSetAeAnalogGainRange(camera,minimum,maximum);
    cout << "True " << endl;
}

void MindVision::exposure_time(int value) {
    double          expLineTime;
    CameraGetExposureLineTime(camera, &expLineTime);
    CameraSetExposureTime(camera,value * expLineTime);
    cout << "True " << endl;
}

void MindVision::exposure_time_range(int minimum,int maximum) {
    CameraSetAeExposureRange(camera,minimum,maximum);
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

    int x,y,w,h;
    cerr << CameraGetWbWindow(camera,&x,&y,&w,&h) << " CameraGetWbWindow" << endl;

    cout << "True" << endl
         << mode << ' '
         << capability.sRgbGainRange.iRGainMin << ' ' << capability.sRgbGainRange.iRGainMax << ' ' << r << ' '
         << capability.sRgbGainRange.iGGainMin << ' ' << capability.sRgbGainRange.iGGainMax << ' ' << g << ' '
         << capability.sRgbGainRange.iBGainMin << ' ' << capability.sRgbGainRange.iBGainMax << ' ' << b << ' '
         << capability.sSaturationRange.iMin << ' ' << capability.sSaturationRange.iMax << ' ' << saturation << ' '
         << monochrome << ' ' << inverse << ' ' << algorithm << ' ' << color_temrature << ' '
         << x << ' ' << y << ' ' << w << ' ' << h << ' '
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

void MindVision::white_balance_window(int x,int y,int w,int h) {
    cerr << CameraSetWbWindow(camera,x,y,w,h) << " CameraSetWbWindow" << endl;
    cout << "True" << endl;
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
    int         m_Sharpness=0,noise = 0,noise3D = 0,count = 0,weight =0,rotate=0;
    float weights ;

    //获得图像的镜像状态。
    CameraGetMirror(camera, MIRROR_DIRECTION_HORIZONTAL, &m_bHflip);
    CameraGetMirror(camera, MIRROR_DIRECTION_VERTICAL,   &m_bVflip);

    //获取当前锐化设定值。
    CameraGetSharpness(camera, &m_Sharpness);
    CameraGetNoiseFilterState(camera,&noise );
    CameraGetDenoise3DParams(camera,&noise3D,&count,&weight,&weights);
    CameraGetRotate(camera,&rotate);
    int flat_field_corrent=0;
    CameraFlatFieldingCorrectGetEnable(camera,&flat_field_corrent);

    int dead_pixels_correct=0;
    CameraGetCorrectDeadPixel(camera,&dead_pixels_correct);

    unsigned int pixels_count = 0;
    CameraReadDeadPixels(camera,nullptr,nullptr,&pixels_count);

    vector<unsigned short> x_array(pixels_count),y_array(pixels_count);
    CameraReadDeadPixels(camera,y_array.data(),x_array.data(),&pixels_count);
    stringstream x_list,y_list;
    if(x_array.size()) {
        for(auto x : x_array) x_list << x << ',';
        x_list.seekp(-1,ios::end); x_list << ' ';
        for(auto y : y_array) y_list << y << ',';
        y_list.seekp(-1,ios::end); y_list << ' ';
    } else {
        x_list << "None ";
        y_list << "None ";
    }

    int undistort;
    CameraGetUndistortEnable(camera,&undistort);

    cout << "True" << endl
         << m_bHflip << ' ' << m_bVflip << ' '
         << capability.sSharpnessRange.iMin << ' ' <<  capability.sSharpnessRange.iMax << ' ' << m_Sharpness << ' '
         << noise << ' ' << noise3D << ' ' << count << ' '
         << rotate << ' '
         << flat_field_corrent << ' '
         << dead_pixels_correct << ' '
         << x_list.str()
         << y_list.str()
         << undistort << ' '
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

void MindVision::flat_field_corrent(int enable) {
    cerr << CameraFlatFieldingCorrectSetEnable(camera,enable) << " CameraFlatFieldingCorrectSetEnable" << endl;
    cout << "True " << endl;
}

void MindVision::flat_field_init(int light) {
    tSdkFrameHead	frameHead;
    BYTE			*rawBuffer;
    unsigned char* rgb_buffer;

    auto status = CameraSnapToBuffer(camera,&frameHead,&rawBuffer,2000);
    cerr << status << " CameraSnapToBuffer " << frameHead.uiMediaType << " " << frameHead.iWidth << " " << frameHead.iHeight << endl;
    if(status != CAMERA_STATUS_SUCCESS) {
        cout << "False" << endl;
        return;
    }

    auto rgbBufferLength = frameHead.iWidth * frameHead.iHeight * (frameHead.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? 1 : 3);

    if(light) {
        if(light_buffer) delete light_buffer;
        rgb_buffer = light_buffer = new unsigned char[rgbBufferLength];
        light_frame_head = frameHead;
    } else {
        if(dark_buffer) delete dark_buffer;
        delete light_buffer; light_buffer = nullptr;
        rgb_buffer = dark_buffer = new unsigned char[rgbBufferLength];
        dark_frame_head = frameHead;
    }

    cerr << CameraImageProcess(camera,rawBuffer,rgb_buffer,&frameHead) << " CameraImageProcess" << endl;
    cerr << CameraReleaseImageBuffer(camera, rawBuffer) << " CameraReleaseImageBuffer" << endl;

    if(dark_buffer && light_buffer) {
        status = CameraFlatFieldingCorrectSetParameter(camera,dark_buffer,&dark_frame_head,light_buffer,&light_frame_head);
        cerr << status << " CameraFlatFieldingCorrectSetParameter" << endl;
    }

    if(status != CAMERA_STATUS_SUCCESS) {
        cout << "False" << endl
             << status << endl;
        return;
    }

    cout << "True" << endl;
}

void MindVision::flat_field_params_save(string filepath) {
    cerr << CameraFlatFieldingCorrectSaveParameterToFile(camera,filepath.c_str()) << endl;
    cout << "True" << endl;
}

void MindVision::flat_field_params_load(string filepath) {
    cerr << CameraFlatFieldingCorrectLoadParameterFromFile(camera,filepath.c_str()) << endl;
    cout << "True" << endl;
}

void MindVision::dead_pixels_correct(int enable) {
    CameraSetCorrectDeadPixel(camera,enable);
    cout << "True " << endl;
}

void MindVision::dead_pixels(string x_list,string y_list) {
    cerr << CameraRemoveAllDeadPixels(camera) << " CameraRemoveAllDeadPixels" << endl;

    if(x_list != "None") {
        regex p(R"(,)");

        sregex_token_iterator x_begin(x_list.begin(),x_list.end(),p,-1),y_begin(y_list.begin(),y_list.end(),p,-1),end;
        vector<unsigned short> x_array,y_array;

        std::transform(x_begin,end,back_inserter(x_array),[](auto x)->unsigned short {
            return (unsigned short)stoi(x);
        });

        std::transform(y_begin,end,back_inserter(y_array),[](auto y)->unsigned short {
            return (unsigned short)stoi(y);
        });

        cerr << CameraAddDeadPixels(camera,y_array.data(),x_array.data(),x_array.size()) << " CameraAddDeadPixels" << endl;
    }

    cerr << CameraSaveDeadPixels(camera) << " CameraSaveDeadPixels" << endl;
    cout << "True" << endl;
}

void MindVision::undistort(int enable) {
    cerr << CameraSetUndistortEnable(camera,enable) << " CameraSetUndistortEnable" << endl;
    cout << "True" << endl;
}

void MindVision::undistory_params(int w,int h,string camera_matrix,string distort_coeffs) {
    regex p(R"(,)");

    sregex_token_iterator m1_begin(camera_matrix.begin(),camera_matrix.end(),p,-1),m2_begin(distort_coeffs.begin(),distort_coeffs.end(),p,-1),end;
    vector<double> m1,m2;

    std::transform(m1_begin,end,back_inserter(m1),[](auto v) { return (double)stod(v); });
    std::transform(m2_begin,end,back_inserter(m2),[](auto v) { return (double)stod(v); });

    cerr << CameraSetUndistortParams(camera,w,h,m1.data(),m2.data()) << " CameraSetUndistortParams" << endl;
    cout << "True" << endl;
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

    stringstream ss,ss2;
    for(auto i=0;i < capability.iImageSizeDesc;i++)
        ss << capability.pImageSizeDesc[i].acDescription << ',';
    ss.seekp(-1,ios::end); ss << " ";

    ss2 << resolution.iHOffsetFOV << ',' << resolution.iVOffsetFOV << ',' << resolution.iWidth << ',' << resolution.iHeight;

    cout << "True" << endl
         << (resolution.iIndex != 0xFF ? 0 : 1) << ',' << resolution.iIndex << endl
         << ss2.str() << endl
         << ss.str()
         << endl;
}

void MindVision::resolution(int index) {
    CameraSetImageResolution(camera,&capability.pImageSizeDesc[index]);
    cout << "True " << endl;
}

void MindVision::resolution(int x,int y,int w,int h) {
    w = w < 50 ? 50 : w;
    h = h < 50 ? 50 : h;
    x -= x % 16;
    y -= y % 4;
    w -= w % 16;
    h -= h % 4;

    tSdkImageResolution sRoiResolution = { 0 };
    sRoiResolution.iIndex = 0xff;
    sRoiResolution.iWidth = w;
    sRoiResolution.iWidthFOV = w;
    sRoiResolution.iHeight = h;
    sRoiResolution.iHeightFOV = h;
    sRoiResolution.iHOffsetFOV = x;
    sRoiResolution.iVOffsetFOV = y;
    sRoiResolution.iWidthZoomSw = 0;
    sRoiResolution.iHeightZoomSw = 0;
    sRoiResolution.uBinAverageMode = 0;
    sRoiResolution.uBinSumMode = 0;
    sRoiResolution.uResampleMask = 0;
    sRoiResolution.uSkipMode = 0;

    cerr << CameraSetImageResolution(camera, &sRoiResolution) << " CameraSetImageResolution " << x << ' ' << y << ' ' << w << ' ' << h << endl;
    cout << "True " << endl;
}

void MindVision::io() {
    unsigned int state;
    int mode;

    cout << "True\n";

    for(int i=0;i<capability.iInputIoCounts;i++) {
        CameraGetInPutIOMode(camera,i,&mode);
        CameraGetIOState(camera,i,&state);
        cout << "Input," << mode << ',' << state << "," << endl;
    }

    for(int i=0;i<capability.iOutputIoCounts;i++) {
        CameraGetOutPutIOMode(camera,i,&mode);
        CameraGetOutPutIOState(camera,i,&state);
        cout << "Output," << mode << ',' << state << "," << endl;
    }
}

void MindVision::io_mode(string type,int index,int value) {
    if(type=="Input")
        CameraSetInPutIOMode(camera,index,value);
    else
        CameraSetOutPutIOMode(camera,index,value);
    cout << "True " << endl;
}

void MindVision::io_state(string type,int index,int value) {

    if(type=="Input") {
        cout << "False" << endl; return;
    } else
        CameraSetIOState(camera,index,value);
    cout << "True" << endl;
}

void MindVision::controls() {
    int  trigger_mode=0,trigger_count=0;
    unsigned int trigger_delay=0,trigger_interval=0;
    int trigger_type=0;
    unsigned int trigger_jitter=0;
    CameraGetTriggerMode(camera,&trigger_mode);
    CameraGetTriggerCount(camera,&trigger_count);
    CameraGetTriggerDelayTime(camera,&trigger_delay);
    CameraGetExtTrigIntervalTime(camera,&trigger_interval);
    CameraGetExtTrigShutterType(camera,&trigger_type);
    CameraGetExtTrigJitterTime(camera,&trigger_jitter);

    int strobe_mode=0,strobe_polarity=0;
    unsigned int strobe_delay=0,strobe_pulse_width =0;
    CameraGetStrobeMode(camera,&strobe_mode);
    CameraGetStrobePolarity(camera,&strobe_polarity);
    CameraGetStrobeDelayTime(camera,&strobe_delay);
    CameraGetStrobePulseWidth(camera,&strobe_pulse_width);

    cout << "True" << endl
         << trigger_mode << ','
         << trigger_count << ','
         << trigger_delay << ','
         << trigger_interval << ','
         << trigger_type << ','
         << trigger_jitter << ','
         << strobe_mode << ','
         << strobe_polarity << ','
         << strobe_delay << ','
         << strobe_pulse_width << ','
         << endl;
}

void MindVision::trigger_mode(int value) {
    cerr << CameraSetTriggerMode(camera,value) << " CameraSetTriggerMode " << value << endl;
    cout << "True " << endl;
}

void MindVision::once_soft_trigger() {
    CameraSoftTrigger(camera);
    cout << "True " << endl;
}

void MindVision::trigger_frames(int value) {
    CameraSetTriggerCount(camera,value);
    cout << "True" << endl;
}

void MindVision::trigger_delay(int value) {
    CameraSetTriggerDelayTime(camera,value);
    cout << "True" << endl;
}

void MindVision::trigger_interval(int value) {
    CameraSetExtTrigIntervalTime(camera,value);
    cout << "True" << endl;
}

void MindVision::outside_trigger_mode(int value) {
    CameraSetExtTrigShutterType(camera,value);
    cout << "True" << endl;
}

void MindVision::outside_trigger_debounce(int value) {
    CameraSetExtTrigJitterTime(camera,value);
    cout << "True" << endl;
}

void MindVision::flash_mode(int value) {
    CameraSetStrobeMode(camera,value);
    cout << "True" << endl;
}

void MindVision::flash_polarity(int value) {
    CameraSetStrobePolarity(camera,value);
    cout << "True" << endl;
}

void MindVision::flash_delay(int value) {
    CameraSetStrobeDelayTime(camera,value);
    cout << "True" << endl;
}

void MindVision::flash_pulse(int value) {
    CameraSetStrobePulseWidth(camera,value);
    cout << "True" << endl;
}

void MindVision::firmware() {
    char firmware[256],iface[256],sdk[256];
    CameraGetFirmwareVersion(camera,firmware);
    CameraGetInerfaceVersion(camera,iface);
    CameraSdkGetVersionString(sdk);
    int updatable=0;
    CameraCheckFwUpdate(camera,&updatable);
    char nickname[256];
    cerr << CameraGetFriendlyName(camera,nickname) << " CameraGetFriendlyName" << endl;

    cout << "True" << endl
         << firmware << ','
         << iface << ','
         << sdk << ','
         << camera_info.acDriverVersion << ','
         << updatable << ','
         << nickname << ','
         << camera_info.acSn << endl;
}

void MindVision::name() {
    char nickname[256];
    cerr << CameraGetFriendlyName(camera,nickname) << " CameraGetFriendlyName" << endl;
    cout << "True" << endl
         << nickname
         << endl;
}

void MindVision::rename(string name) {
    cerr << CameraSetFriendlyName(camera,const_cast<char*>(name.c_str())) << " CameraSetFriendlyName" << endl;
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
    st.interrupt = false;

    st.start();
    cout << "True " << endl;
}

void MindVision::snapshot_state() {
    cout << "True " << st.isRunning() << " " << endl;
}

void MindVision::snapshot_stop() {
    st.interrupt = true;
    st.wait();
    cout << "True " << endl;
}

void MindVision::record_start(string dir,int format,int quality,int frames) {
    rt.camera = camera;
    rt.capability = capability;
    rt.dir = dir;
    rt.format = format;
    rt.quality = quality;
    rt.frames = frames;
    rt.interrupt = false;

    stringstream filename;
    filename << dir << "/mind-vision-" << std::time(0);

    cerr << CameraInitRecord(camera,format,const_cast<char*>(filename.str().c_str()),0,quality,frames)
         << " CameraInitRecord " << filename.str() << ' ' << quality << ' ' << frames
         << endl;

    rt.start();

    cout << "True " << endl;
}

void MindVision::record_state() {
    cout << "True " << rt.isRunning() << " " << endl;
}

void MindVision::record_stop() {
    rt.interrupt = true;
    rt.wait();
    cerr << CameraStopRecord(camera) << " CameraStopRecord" << endl;
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


