#include "mindvision.h"
#include "defectpixelalg.h"
#include "brightness.h"
#include "GF120.h"

#include <QStandardPaths>
#include <QSettings>

MindVision::MindVision() : camera(0)
  , dark_buffer(nullptr)
  , light_buffer(nullptr)
  , playing(false)
{
    QSettings settings("MindVision","Example");
    CameraSdkInit("zh" == settings.value("language","zh"));    //sdk初始化  0 English 1中文

}

MindVision::~MindVision()
{
    if(!camera) return;

    pipeName.clear();
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
        int opened = 0;
        CameraIsOpened(&cameraEnumList[i],&opened);
        cout << cameraEnumList[i].acProductSeries << ','
           << cameraEnumList[i].acProductName << ','
           << cameraEnumList[i].acFriendlyName << ','
           << cameraEnumList[i].acLinkName << ','
           << cameraEnumList[i].acDriverVersion << ','
           << cameraEnumList[i].acSensorType << ','
           << cameraEnumList[i].acPortType << ','
           << cameraEnumList[i].acSn << ','
           << opened;

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

    if(status != CAMERA_STATUS_SUCCESS) {
        cout << "False " << endl;
        throw runtime_error("相机初始化失败！");
    }

    if(capability.sIspCapacity.bMonoSensor) status = CameraSetIspOutFormat(camera,CAMERA_MEDIA_TYPE_MONO8);
    else status = CameraSetIspOutFormat(camera,CAMERA_MEDIA_TYPE_RGB8);

    if(status != CAMERA_STATUS_SUCCESS) {
        cout << "False " << endl;
        throw runtime_error("相机初始化失败！");
    }

    pipeName = cameraName;

    auto rgbBufferMaxLength = sizeof(frame_head) + capability.sResolutionRange.iWidthMax * capability.sResolutionRange.iHeightMax * (capability.sIspCapacity.bMonoSensor ? 1 : 3);
    this->status(camera_info.acPortType);

    sm.setKey(QString::fromStdString(pipeName + ".0.sm"));
    if(sm.isAttached()) sm.detach();
    sm.create(rgbBufferMaxLength);

    infrared_sm.setKey(QString::fromStdString(pipeName + ".infrared.sm"));
    if(infrared_sm.isAttached()) infrared_sm.detach();
    infrared_sm.create(sizeof(GF120_get_param));

    infrared_params_sm.setKey(QString::fromStdString(pipeName + ".infrared-params.sm"));
    if(infrared_params_sm.isAttached()) infrared_params_sm.detach();
    infrared_params_sm.create(sizeof(GF120_set_param_t));
    pGF120_set_param = reinterpret_cast<GF120_set_param_t*>(infrared_params_sm.data());

    infrared_temperature_roi_sm.setKey(QString::fromStdString(pipeName + ".infrared_temperature_roi.sm"));
    if(infrared_temperature_roi_sm.isAttached()) infrared_temperature_roi_sm.detach();
    infrared_temperature_roi_sm.create(sizeof(GF120_roi_temp));


    parameter_init();

    cout << "True " << pipeName << ' ' << endl;
}

void MindVision::frame() {
    if(pipeName.empty()) return;

    tSdkFrameHead frameHead;
    unsigned char* rawBuffer = nullptr;

    if (CAMERA_STATUS_SUCCESS != CameraGetImageBufferPriority(camera,&frameHead,&rawBuffer,2000,CAMERA_GET_IMAGE_PRIORITY_NEWEST))
        return;

    frame_head.num = frame_head.num+1;
    frame_head.width = frameHead.iWidth;
    frame_head.height = frameHead.iHeight;
    frame_head.bits = (frameHead.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? 1 : 3);
    frame_head.snapshot_status = st.isRunning();
    frame_head.record_status = rt.isRunning();

    sm.lock();

    auto frameHeadBuffer = reinterpret_cast<unsigned char*>(sm.data());
    auto rgbBuffer = frameHeadBuffer + sizeof(frame_head);

    memcpy(frameHeadBuffer,&frame_head,sizeof(frame_head));
    CameraImageProcess(camera,rawBuffer,rgbBuffer,&frameHead);
    CameraFlipFrameBuffer(rgbBuffer,&frameHead,1);

    sm.unlock();

    CameraReleaseImageBuffer(camera,rawBuffer);
}

void MindVision::exposure(bool full) {
    BOOL            mode = 0;
    int             brightness = 0;
    double          exposureTime = 0;
    int             analogGain = 0;
    BOOL            flicker = 0;
    int             frequencySel = 0;
    double	        expLineTime = 0; //当前的行曝光时间，单位为us
    double exposureRangeMinimum=0.,exposureRangeMaximum=0.;
    int uiAnalogGainMin=0,uiAnalogGainMax=0;
    double exposureRangeMinimum2=0.,exposureRangeMaximum2=0.;

    int threshould = 0;
    int x,y,w,h;

    if(full) {
        CameraGetAeState(camera,&mode);
        CameraGetAeTarget(camera,&brightness);
        CameraGetAntiFlick(camera,&flicker);
        CameraGetLightFrequency(camera,&frequencySel);
        CameraGetExposureTimeRange(camera,&exposureRangeMinimum,&exposureRangeMaximum,&expLineTime);
        CameraGetAeThreshold(camera,&threshould);
        CameraGetAeWindow(camera,&x,&y,&w,&h);
        CameraGetAeAnalogGainRange(camera,&uiAnalogGainMin,&uiAnalogGainMax);
        CameraGetAeExposureRange(camera,&exposureRangeMinimum2,&exposureRangeMaximum2);
    }

    CameraGetAnalogGain(camera,&analogGain);
    CameraGetExposureTime(camera,&exposureTime);

    stringstream ss;
    ss.precision(2);
    ss << "True\n"
         << mode << ','
         << capability.sExposeDesc.uiTargetMin << ','
         << capability.sExposeDesc.uiTargetMax << ','
         << brightness << ','
         << flicker << ','
         << frequencySel << ','
         << threshould << ','
         << (int)(capability.sExposeDesc.fAnalogGainStep * capability.sExposeDesc.uiAnalogGainMin * 100.) << ','
         << (int)(capability.sExposeDesc.fAnalogGainStep * capability.sExposeDesc.uiAnalogGainMax * 100.) << ','
         << (int)(capability.sExposeDesc.fAnalogGainStep * analogGain * 100.) << ','
         << (int)(capability.sExposeDesc.uiExposeTimeMin * expLineTime) << ','
         << (int)(capability.sExposeDesc.uiExposeTimeMax * expLineTime) << ','
         << (int)exposureTime << ','
         << (int)exposureRangeMinimum << ','
         << (int)exposureRangeMaximum << ","
         << (int)(capability.sExposeDesc.fAnalogGainStep * uiAnalogGainMin * 100.) << ','
         << (int)(capability.sExposeDesc.fAnalogGainStep * uiAnalogGainMax * 100.) << ','
         << (int)exposureRangeMinimum2 << ','
         << (int)exposureRangeMaximum2 << ",\n"
         << x << ',' << y << ',' << w << ',' << h << ','
         << endl;

    ss.str().copy(frame_head.exposure_status,ss.str().size());

    if(full) {
        cout << ss.str();
        cout.flush();
    }
}

void MindVision::exposure_mode(int value) {
    CameraSetAeState(camera,value);
}

void MindVision::brightness(int value) {
    cerr << CameraSetAeTarget(camera,value) << " CameraSetAeTarget" << endl;
}

void MindVision::threshold(int value) {
    cerr << CameraSetAeThreshold(camera,value) << " CameraSetAeThreshold" << endl;
}

void MindVision::flicker(int value) {
    CameraSetAntiFlick(camera,value);

}

void MindVision::gain(int value) {
    CameraSetAnalogGain(camera,value / capability.sExposeDesc.fAnalogGainStep / 100.f);
}

void MindVision::gain_range(int minimum,int maximum) {
    CameraSetAeAnalogGainRange(camera,minimum / capability.sExposeDesc.fAnalogGainStep / 100.f,maximum / capability.sExposeDesc.fAnalogGainStep / 100.f);

}

void MindVision::exposure_time(int pos) {
    CameraSetExposureTime(camera,pos);

}

void MindVision::exposure_time_range(double minimum,double maximum) {
    cerr << CameraSetAeExposureRange(camera,minimum,maximum) << " CameraSetAeExposureRange " << minimum << ',' << maximum << endl;

}

void MindVision::frequency(int value) {
    CameraSetLightFrequency(camera,value);

}

void MindVision::exposure_window(int x,int y,int w,int h) {
    CameraSetAeWindow(camera,x,y,w,h);
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

    stringstream colorTemplates;
    colorTemplates << "Automation,";
    for(auto i=0;i < capability.iClrTempDesc;i++)
        colorTemplates << capability.pClrTempDesc[i].acDescription << ',';

    constexpr int IO_CONTROL_ENABLE_FPN = 37; // 启用、禁用FPN
    int fpn = 0;
    CameraSpecialControl(camera, IO_CONTROL_ENABLE_FPN, 0, &fpn);

    cout << "True" << '\n'
         << mode << ','
         << capability.sRgbGainRange.iRGainMin << ',' << capability.sRgbGainRange.iRGainMax << ',' << r << ','
         << capability.sRgbGainRange.iGGainMin << ',' << capability.sRgbGainRange.iGGainMax << ',' << g << ','
         << capability.sRgbGainRange.iBGainMin << ',' << capability.sRgbGainRange.iBGainMax << ',' << b << ','
         << capability.sSaturationRange.iMin << ',' << capability.sSaturationRange.iMax << ',' << saturation << ','
         << monochrome << ',' << inverse << ',' << algorithm << ',' << color_temrature << ','
         << x << ',' << y << ',' << w << ',' << h << ',' << fpn << '\n'
         << colorTemplates.str()
         << endl;
}

void MindVision::white_balance_mode(int index) {
    cerr << CameraSetWbMode(camera,index) << " CameraSetWbMode" << endl;

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


}

void MindVision::once_white_balance() {
    cerr << CameraSetOnceWB(camera) << " CameraSetOnceWB" << endl;

}

void MindVision::white_balance_window(int x,int y,int w,int h) {
    cerr << CameraSetWbWindow(camera,x,y,w,h) << " CameraSetWbWindow" << endl;

}

void MindVision::rgb(int r,int g,int b) {
    cerr << CameraSetGain(camera,r,g,b) << " CameraSetGain " << r << ' ' << g << ' ' << b << endl;

}

void MindVision::saturation(int value) {
    cerr << CameraSetSaturation(camera,value) << " CameraSetSaturation " << value << endl;

}

void MindVision::monochrome(int enable) {
    cerr << CameraSetMonochrome(camera,enable) << " CameraSetMonochrome" << endl;

}

void MindVision::inverse(int enable) {
    cerr << CameraSetInverse(camera,enable) << " CameraSetInverse" << endl;

}

void MindVision::algorithm(int index) {
    cerr << CameraSetBayerDecAlgorithm(camera,ISP_PROCESSSOR_PC,index) << " CameraSetBayerDecAlgorithm" << endl;

}

void MindVision::lookup_table_mode() {
    int mode = -1;
    cerr << CameraGetLutMode(camera,&mode) << " CameraGetLutMode" << endl;
    cout << "True "
         << mode << ' ' << endl;
}

void MindVision::lookup_table_mode(int index) {
    cerr << CameraSetLutMode(camera,index) << " CameraSetLutMode" << endl;

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

}

void MindVision::contrast_ratio(int value) {
    CameraSetContrast(camera,value);

}

void MindVision::transform() {
    BOOL        m_bHflip=FALSE,m_bHflipHard=-1;
    BOOL        m_bVflip=FALSE,m_bVflipHard=-1;
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
    stringstream filename;

    auto dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation).replace('/','\\').toLocal8Bit().data();
    filename << dir << '/' << camera << "-manual-pixels.txt";

    ofstream f;
    f.open(filename.str());

    for(int i=0;i < x_array.size();i++){
        f << x_array[i] << ',' << y_array[i] << ',' << endl;
    }

    f.close();

    int undistort;
    CameraGetUndistortEnable(camera,&undistort);

    CameraGetHardwareMirror(camera, MIRROR_DIRECTION_HORIZONTAL, &m_bHflipHard);
    CameraGetHardwareMirror(camera, MIRROR_DIRECTION_VERTICAL,   &m_bVflipHard);

    cout << "True" << endl
         << m_bHflip << ' ' << m_bVflip << ' '
         << capability.sSharpnessRange.iMin << ' ' <<  capability.sSharpnessRange.iMax << ' ' << m_Sharpness << ' '
         << noise << ' ' << noise3D << ' ' << count << ' '
         << rotate << ' '
         << flat_field_corrent << ' '
         << dead_pixels_correct << ' '
         << filename.str() << ' '
         << undistort << ' '
         << m_bHflipHard << ' ' << m_bVflipHard
         << endl;
}

void MindVision::horizontal_mirror(int hard,int value) {
    if(hard) {
        CameraSetHardwareMirror(camera, MIRROR_DIRECTION_HORIZONTAL, value);
    } else {
        CameraSetMirror(camera, MIRROR_DIRECTION_HORIZONTAL, value);
    }

}

void MindVision::vertical_mirror(int hard,int value) {
    if(hard) {
        CameraSetHardwareMirror(camera, MIRROR_DIRECTION_VERTICAL, value);
    } else {
        CameraSetMirror(camera, MIRROR_DIRECTION_VERTICAL, value);
    }

}

void MindVision::acutance(int value) {
    CameraSetSharpness(camera,value);

}

void MindVision::noise(int enable) {
    CameraSetNoiseFilter(camera,enable);

}

void MindVision::noise3d(int enable,int value) {
    CameraSetDenoise3DParams(camera,enable,value,nullptr);

}

void MindVision::rotate(int value) {
    CameraSetRotate(camera,value);

}

void MindVision::flat_field_corrent(int enable) {
    cerr << CameraFlatFieldingCorrectSetEnable(camera,enable) << " CameraFlatFieldingCorrectSetEnable" << endl;

}

void MindVision::flat_field_init(int light) {
    tSdkFrameHead	frameHead;
    BYTE			*rawBuffer;
    unsigned char* rgb_buffer;

    auto status = CameraGetImageBuffer(camera,&frameHead,&rawBuffer,2000);
    cerr << status << " CameraGetImageBuffer " << frameHead.uiMediaType << " " << frameHead.iWidth << " " << frameHead.iHeight << endl;
    if(status != CAMERA_STATUS_SUCCESS) {
        cout << "False" << endl;
        return;
    }

    if(light) {
        if(light_buffer) delete light_buffer;
        rgb_buffer = light_buffer = new unsigned char[frameHead.uBytes];
        light_frame_head = frameHead;
    } else {
        if(dark_buffer) delete dark_buffer;
        delete light_buffer; light_buffer = nullptr;
        rgb_buffer = dark_buffer = new unsigned char[frameHead.uBytes];
        dark_frame_head = frameHead;
    }

    memcpy(rgb_buffer,rawBuffer,frameHead.uBytes);
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
}

void MindVision::flat_field_params_save(string filepath) {
    cerr << CameraFlatFieldingCorrectSaveParameterToFile(camera,filepath.c_str()) << endl;
}

void MindVision::flat_field_params_load(string filepath) {
    cerr << CameraFlatFieldingCorrectLoadParameterFromFile(camera,filepath.c_str()) << endl;
}

void MindVision::dead_pixels_correct(int enable) {
    CameraSetCorrectDeadPixel(camera,enable);

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
}

void MindVision::dead_pixels_analyze_for_bright(int threshold) {
    vector<MvPoint16> brightPixels;

    tSdkFrameHead frameHead;
    unsigned char* rawBuffer = nullptr;
    CameraGetImageBuffer(camera,&frameHead,&rawBuffer,10000);
    AnalyzeBrightPixelFromImage(brightPixels,rawBuffer,threshold,frameHead.iWidth,frameHead.iHeight);
    CameraReleaseImageBuffer(camera,rawBuffer);

    stringstream filename;
    auto dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation).replace('/','\\').toLocal8Bit().data();
    filename << dir << "/" << camera << "-bright-pixels.txt";

    ofstream f;
    f.open(filename.str());

    for(auto pixel : brightPixels) {
        f << pixel.x << ',' << pixel.y << ",\n";
    }

    f.close();

    cout << "True\n"
         << filename.str()
         << endl;
}

void MindVision::dead_pixels_analyze_for_dead(int threshold) {
    vector<MvPoint16> deadPixels;

    tSdkFrameHead frameHead;
    unsigned char* rawBuffer = nullptr;
    cerr << CameraGetImageBuffer(camera,&frameHead,&rawBuffer,10000) << " AnalyzeDefectPixelFromImage" << endl;

    AnalyzeDefectPixelFromImage(deadPixels,rawBuffer,threshold,frameHead.iWidth,frameHead.iHeight);
    CameraReleaseImageBuffer(camera,rawBuffer);

    stringstream filename;
    auto dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation).replace('/','\\').toLocal8Bit().data();
    filename << dir << "/" << camera << "-dead-pixels.txt";

    ofstream f;
    f.open(filename.str());

    for(auto pixel : deadPixels) {
        f << pixel.x << ',' << pixel.y << ",\n";
    }

    f.close();
    cout << "True\n"
         << filename.str()
         << endl;
}

void MindVision::undistort(int enable) {
    cerr << CameraSetUndistortEnable(camera,enable) << " CameraSetUndistortEnable" << endl;
}

void MindVision::undistory_params(int w,int h,string camera_matrix,string distort_coeffs) {
    regex p(R"(,)");

    sregex_token_iterator m1_begin(camera_matrix.begin(),camera_matrix.end(),p,-1),m2_begin(distort_coeffs.begin(),distort_coeffs.end(),p,-1),end;
    vector<double> m1,m2;

    std::transform(m1_begin,end,back_inserter(m1),[](auto v) { return (double)stod(v); });
    std::transform(m2_begin,end,back_inserter(m2),[](auto v) { return (double)stod(v); });

    cerr << CameraSetUndistortParams(camera,w,h,m1.data(),m2.data()) << " CameraSetUndistortParams" << endl;
}

void MindVision::video() {
    int speed=0,hz=0,index=-1,bits=0,max_bits=8;
    CameraGetFrameSpeed(camera,&speed);
    CameraGetFrameRate(camera,&hz);
    CameraGetMediaType(camera,&index);
    CameraGetRawStartBit(camera,&bits);
    CameraGetRawMaxAvailBits(camera,&max_bits);

    stringstream ss;

    for(int i=0;i<capability.iMediaTypdeDesc;i++)
        ss << capability.pMediaTypeDesc[i].acDescription << ',';

    stringstream frameSpeedTypes;

    for(int i=0;i < capability.iFrameSpeedDesc;i++)
        frameSpeedTypes << capability.pFrameSpeedDesc[i].acDescription << "Speed" << ',';

    cout << "True\n"
         << speed << ',' << hz << ',' << index << ',' << bits << ',' << max_bits << ",\n"
         << ss.str() << '\n'
         << frameSpeedTypes.str()
         << endl;
}

void MindVision::frame_rate_speed(int index) {
    CameraSetFrameSpeed(camera,index);

}

void MindVision::frame_rate_limit(int value) {
    CameraSetFrameRate(camera,value);

}

void MindVision::video_output_format(int index) {
    cerr << CameraSetMediaType(camera,index) << endl;
}

void MindVision::raw_output_range(int value) {
    cerr << CameraSetRawStartBit(camera,value) << " CameraSetRawStartBit " << value << endl;
}

void MindVision::resolutions() {
    tSdkImageResolution resolution;
    CameraGetImageResolution(camera,&resolution);

    stringstream ss,ss2;
    for(auto i=0;i < capability.iImageSizeDesc;i++)
        ss << capability.pImageSizeDesc[i].acDescription << ',';
    ss.seekp(-1,ios::end); ss << ",";

    ss2 << resolution.iHOffsetFOV << ',' << resolution.iVOffsetFOV << ',' << resolution.iWidth << ',' << resolution.iHeight;

    cout << "True" << endl
         << (resolution.iIndex != 0xFF ? 0 : 1) << ',' << resolution.iIndex << '\n'
         << ss2.str() << '\n'
         << ss.str()
         << endl;
}

void MindVision::resolution(int index) {
    cerr << CameraSetImageResolution(camera,&capability.pImageSizeDesc[index]) << " CameraSetImageResolution " << index << endl;

}

void MindVision::resolution(int x,int y,int w,int h) {
    w = w < 16 ? 16 : w;
    h = h < 4 ? 4 : h;
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

}

void MindVision::io() {
    unsigned int state;
    int mode;

    cout << "True\n";

    for(int i=0;i<capability.iInputIoCounts;i++) {
        CameraGetInPutIOMode(camera,i,&mode);
        CameraGetIOState(camera,i,&state);
        cout << "Input," << mode << ',' << state << ",\n";
    }

    for(int i=0;i<capability.iOutputIoCounts;i++) {
        CameraGetOutPutIOMode(camera,i,&mode);
        CameraGetOutPutIOState(camera,i,&state);
        cout << "Output," << mode << ',' << state << ",\n";
    }

    cout.flush();
}

void MindVision::io_mode(string type,int index,int value) {
    if(type=="Input")
        cerr << CameraSetInPutIOMode(camera,index,value) <<  " CameraSetInPutIOMode " << type << ',' << index <<',' << value << endl;
    else
        cerr << CameraSetOutPutIOMode(camera,index,value) << " CameraSetOutPutIOMode " << type << ',' << index <<',' << value << endl;

}

void MindVision::io_state(string type,int index,int value) {

    if(type=="Input") {
        cout << "False" << endl; return;
    } else
        CameraSetIOStateEx(camera,index,value);
}

void MindVision::controls() {
    int  trigger_mode=0,trigger_count=0,trigger_signal=0;
    unsigned int trigger_delay=0,trigger_interval=0;
    int trigger_type=0;
    unsigned int trigger_jitter=0;
    CameraGetTriggerMode(camera,&trigger_mode);
    CameraGetTriggerCount(camera,&trigger_count);
    CameraGetTriggerDelayTime(camera,&trigger_delay);
    CameraGetExtTrigIntervalTime(camera,&trigger_interval);
    CameraGetExtTrigSignalType(camera,&trigger_signal);
    CameraGetExtTrigShutterType(camera,&trigger_type);
    CameraGetExtTrigJitterTime(camera,&trigger_jitter);

    int strobe_mode=0,strobe_polarity=0;
    unsigned int strobe_delay=0,strobe_pulse_width =0;
    CameraGetStrobeMode(camera,&strobe_mode);
    CameraGetStrobePolarity(camera,&strobe_polarity);
    CameraGetStrobeDelayTime(camera,&strobe_delay);
    CameraGetStrobePulseWidth(camera,&strobe_pulse_width);

    UINT mask;
    CameraGetExtTrigCapability(camera,&mask);

    stringstream signalTypes,shutterTypes;
    signalTypes << "RisingEdge,FallingEdge,";
    if(mask & EXT_TRIG_MASK_LEVEL_MODE) signalTypes << "HighLevel,LowLevel,";
    if(mask & EXT_TRIG_MASK_DOUBLE_EDGE) signalTypes << "DoubleEdge,";

    shutterTypes << "Standard,";
    if(mask & EXT_TRIG_MASK_GRR_SHUTTER) shutterTypes << "GRR,";

    cout << "True" << endl
         << trigger_mode << ','
         << trigger_count << ','
         << trigger_delay << ','
         << trigger_interval << ','
         << trigger_signal << ','
         << trigger_jitter << ','
         << strobe_mode << ','
         << strobe_polarity << ','
         << strobe_delay << ','
         << strobe_pulse_width << ','
         << trigger_type << ',' << '\n'
         << signalTypes.str() << '\n'
         << shutterTypes.str()
         << endl;
}

void MindVision::trigger_mode(int value) {
    cerr << CameraSetTriggerMode(camera,value) << " CameraSetTriggerMode " << value << endl;
};

void MindVision::once_soft_trigger() {
    CameraSoftTrigger(camera);
}

void MindVision::trigger_frames(int value) {
    cerr << CameraSetTriggerCount(camera,value) << " CameraSetTriggerCount " << value << endl;

}

void MindVision::trigger_delay(unsigned int value) {
    cerr << CameraSetTriggerDelayTime(camera,value) << " CameraSetTriggerDelayTime " << value << endl;

}

void MindVision::trigger_interval(unsigned int value) {
    cerr << CameraSetExtTrigIntervalTime(camera,value) << " CameraSetExtTrigIntervalTime " << value << endl;

}

void MindVision::outside_trigger_mode(int value) {
    cerr << CameraSetExtTrigSignalType(camera,value) << " CameraSetExtTrigSignalType " << value << endl;

}

void MindVision::outside_trigger_debounce(unsigned int value) {
    cerr << CameraSetExtTrigJitterTime(camera,value) << " CameraSetExtTrigJitterTime " << value << endl;
}

void MindVision::outside_shutter(int index) {
    cerr << CameraSetExtTrigShutterType(camera,index) << " CameraSetExtTrigShutterType " << index << endl;

}

void MindVision::flash_mode(int value) {
    cerr << CameraSetStrobeMode(camera,value) << " CameraSetStrobeMode " << value << endl;
}

void MindVision::flash_polarity(int value) {
    CameraSetStrobePolarity(camera,value);
}

void MindVision::flash_delay(unsigned int value) {
    cerr << CameraSetStrobeDelayTime(camera,value) << " CameraSetStrobeDelayTime " << value << endl;

}

void MindVision::flash_pulse(unsigned int value) {
    cerr << CameraSetStrobePulseWidth(camera,value) << " CameraSetStrobePulseWidth " << value << endl;

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
}

void MindVision::params_reset() {
    cerr << CameraLoadParameter(camera,PARAMETER_TEAM_DEFAULT) << " CameraLoadParameter" << endl;
}

void MindVision::params_save(int value) {
    cerr << CameraSaveParameter(camera,value) << " CameraSaveParameter " << value << endl;
}

void MindVision::params_load(int value) {
    cerr << CameraLoadParameter(camera,value) << " CameraLoadParameter "<< value << endl;
}

void MindVision::params_save_to_file(string filename) {
    cerr << CameraSaveParameterToFile(camera,const_cast<char*>(filename.c_str())) << " CameraSaveParameterToFile " << filename << endl;
}

void MindVision::params_load_from_file(string filename) {
    cerr << CameraReadParameterFromFile(camera,const_cast<char*>(filename.c_str())) << " CameraReadParameterFromFile " << filename<< endl;
}

void MindVision::snapshot_resolution() {
    tSdkImageResolution resolution;
    CameraGetResolutionForSnap(camera,&resolution);
    cout << "True " << resolution.iIndex << endl;
}

void MindVision::snapshot_resolution(int index) {
    CameraSetResolutionForSnap(camera,&capability.pImageSizeDesc[index]);
}

void MindVision::snapshot_start(string dir,int resolution,int format,int period) {
    snapshot_state();
    st.dir = dir;
    st.format = format;
    st.camera = camera;
    st.capability = capability;
    st.resolution = resolution;
    st.period = period;
    st.interrupt = false;

    st.start();

}

void MindVision::snapshot_state() {

}

void MindVision::snapshot_stop() {
    st.interrupt = true;
    st.wait();
}

void MindVision::record_start(string dir,int format,int quality,int frames) {
    record_state();
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
}

void MindVision::record_state() {
}

void MindVision::record_stop() {
    rt.interrupt = true;
    rt.wait();
    cerr << CameraStopRecord(camera) << " CameraStopRecord" << endl;
}

void MindVision::play() {
    playing = true;
    cerr << CameraPlay(camera) << " CameraPlay"  << endl;
}

void MindVision::pause() {
    playing = false;
    cerr << CameraPause(camera) << " CameraPause"  << endl;
}

void MindVision::stop() {
    pipeName.clear();
    playing = false;
    cerr << CameraStop(camera) << " CameraStop"  << endl;
}

void MindVision::status(string type) {
    const int IO_CONTROL_DEVICE_TEMPERATURE	= 20;
    const int IO_CONTROL_GET_FRAME_RESEND   = 17;
    const int IO_CONTROL_GET_LINK_SPEED	= 26;
    int iResend = 0;
    float fDevTemperature = 0.f;
    int iFrameLost = 0;
    UINT sensorFPS = 0;
    UINT uLinkSpeed = 0;

    CameraSpecialControl(camera, IO_CONTROL_GET_FRAME_RESEND, 0, &iResend);
    CameraSpecialControl(camera,IO_CONTROL_DEVICE_TEMPERATURE,0,&fDevTemperature);
    CameraSpecialControl(camera, IO_CONTROL_GET_LINK_SPEED, 0, &uLinkSpeed);

    tSdkFrameStatistic statistic;
    CameraGetFrameStatistic(camera,&statistic);

    static int capture;
    auto captureFPS = statistic.iCapture - capture;

    if (memcmp(type.c_str(), "NET", 3) == 0)
    {
        const int IO_CONTROL_GET_FRAME_LOST = 14;
        const int IO_CONTROL_GET_GEVREG		= 0x1000;

        int iFrameResend = 0;
        CameraSpecialControl(camera,IO_CONTROL_GET_FRAME_LOST,0,&iFrameLost);
        CameraSpecialControl(camera,IO_CONTROL_GET_FRAME_RESEND,0,&iFrameResend);

        UINT uPackSize = 0;
        CameraSpecialControl(camera, IO_CONTROL_GET_GEVREG, 0xD04, &uPackSize);
        CameraSpecialControl(camera, IO_CONTROL_GET_GEVREG, 0x10000510, &sensorFPS);
        sensorFPS /= 4.f;

        UINT TrigCount = 0;
        CameraSpecialControl(camera, IO_CONTROL_GET_GEVREG, 0x1000050C, &TrigCount);

        char GvspDevTemp[32] = { 0 };
        CameraCommonCall(camera, "get_gvsp_dev_temp()", GvspDevTemp, sizeof(GvspDevTemp));

        status_string.clear();
        status_string.seekp(0);
        status_string << "True\n"
             << statistic.iCapture << ','
             << captureFPS << ','
             << sensorFPS << ','
             << fDevTemperature << ','
             << iFrameLost << ','
             << iFrameResend << ','
             << (uPackSize & 0xffff) << ','
             << uLinkSpeed << ','
             << endl;
    } else if (memcmp(type.c_str(), "USB3", 4) == 0) {
        const int IO_CONTROL_GET_FRAME_DISCARD    = 15;
        const int IO_CONTROL_FPGA_READ			= 8;
        const int IO_CONTROL_GET_RECOVER_COUNT = 24;

        CameraSpecialControl(camera, IO_CONTROL_FPGA_READ, 0x75, &sensorFPS);
        sensorFPS /= 4.f;
        CameraSpecialControl(camera, IO_CONTROL_GET_FRAME_DISCARD, 0, &iFrameLost);

        int iRecover = 0;
        CameraSpecialControl(camera, IO_CONTROL_GET_RECOVER_COUNT, 0, &iRecover);

        status_string.clear();
        status_string.seekp(0);
        status_string << "True\n"
             << statistic.iCapture << ','
             << captureFPS << ','
             << sensorFPS << ','
             << fDevTemperature << ','
             << iFrameLost << ','
             << iResend << ','
             << iRecover << ','
             << uLinkSpeed << ','
             << endl;
    }
    else
    {
        status_string.clear();
        status_string.seekp(0);
        status_string << "True\n"
             << statistic.iCapture << ','
             << captureFPS << ','
             << sensorFPS << ','
             << 0 << ','
             << statistic.iLost << ','
             << 0 << ','
             << 0 << ','
             << uLinkSpeed << ','
             << endl;
    }

    capture = statistic.iCapture;

    status_string.str().copy(frame_head.camera_status,status_string.str().size());
}

void MindVision::brightness() {
    UINT acc_y = 0;

    tSdkFrameHead frameHead;
    unsigned char* rawBuffer = nullptr;

    int mode=1;
    CameraGetTriggerMode(camera,&mode);
    if(playing && mode == 0 && CAMERA_STATUS_SUCCESS == CameraGetImageBuffer(camera,&frameHead,&rawBuffer,2000)) {
        auto rgbBuffer = new unsigned char[frameHead.iWidth * frameHead.iHeight * (frameHead.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? 1 : 3)];
        CameraImageProcess(camera,rawBuffer,rgbBuffer,&frameHead);
        YAcc(rgbBuffer,&frameHead,&acc_y);
        CameraReleaseImageBuffer(camera,rawBuffer);
        delete[] rgbBuffer;
    } else {

    }

    cout << "True\n"
         << acc_y << ','
         << endl;
}

void MindVision::fpn_save(string filepath) {
    constexpr int IO_CONTROL_SAVE_FPN_TO_FILE = 33;     // 把相机里的FPN数据存到文件
    CameraSpecialControl(camera, IO_CONTROL_SAVE_FPN_TO_FILE, 0, (void*)filepath.c_str());
}

void MindVision::fpn_load(string filepath) {
    constexpr int IO_CONTROL_LOAD_FPN_TO_DEVICE = 34;   // 从文件加载FPN数据到相机里
    CameraSpecialControl(camera, IO_CONTROL_LOAD_FPN_TO_DEVICE, 0, (void*)filepath.c_str());
}

void MindVision::fpn_clear() {
    constexpr int IO_CONTROL_DELETE_FPN = 35;           // 删除相机里的FPN数据
    CameraSpecialControl(camera, IO_CONTROL_DELETE_FPN, 0, NULL);
}

void MindVision::fpn(int e) {
    constexpr int IO_CONTROL_ENABLE_FPN = 37; // 启用、禁用FPN
    CameraSpecialControl(camera, IO_CONTROL_ENABLE_FPN, e, NULL);
}

void MindVision::infrared_thermometry(int index) {
    GF120_set_param.temp_mode = index;
    CameraSpecialControl(camera, SET_TEMP_MODE,GF120_set_param.temp_mode,0);
}

void MindVision::infrared_color(int index)
{
    GF120_set_param.color_mode = index;
    CameraSpecialControl(camera, SET_COLOR_MODE,GF120_set_param.color_mode,0);
}

void MindVision::infrared_display(int index) {
    if(index == IMAGE_RGB888_ENABLE)
    {
        CameraSetMediaType(camera,IMAGE_RGB888_ENABLE);                  //伪彩色
        CameraSetIspOutFormat(camera, CAMERA_MEDIA_TYPE_RGB8);           //显示格式为RGB8
    }
    else if(index == IMAGE_TEMP_ENABLE)
    {
        CameraSetMediaType(camera,IMAGE_TEMP_ENABLE);                    //温度数据
        CameraSetIspOutFormat(camera, CAMERA_MEDIA_TYPE_MONO8);          //MONO16显示高8bit 格式为MONO8
    }
    else if(index == IMAGE_RGB888_TEMP)
    {
        CameraSetMediaType(camera,IMAGE_RGB888_TEMP);                    //伪彩色 + 温度数据
        CameraSetIspOutFormat(camera, CAMERA_MEDIA_TYPE_RGB8);           //显示格式为RGB8
    }
}

void MindVision::infrared_shutter(int checked) {
    GF120_set_param.shutter_on = checked;
    CameraSpecialControl(camera, SET_SHUTTER_MODE,GF120_set_param.shutter_on,0);
}

void MindVision::infrared_cool(int checked) {
    GF120_set_param.cold_mode = checked;
    CameraSpecialControl(camera, SET_COLD_MODE,GF120_set_param.cold_mode,0);
}

void MindVision::infrared_emissivity(int value) {
    GF120_set_param.user_emissivity = value;			//设置的发射率
    CameraSpecialControl(camera, SET_USER_EMISSIVITY,GF120_set_param.user_emissivity,0);
}

void MindVision::infrared_sharpen(int value) {
    GF120_set_param.sharpen_grade = value;				//锐化等级 设置我0 不锐化
    CameraSpecialControl(camera, SET_SHARPEN_GRADE,GF120_set_param.sharpen_grade,0);
}

void MindVision::infrared_dde(int value) {
    GF120_set_param.dde_grade = value;				//锐化等级 设置我0 不锐化
    CameraSpecialControl(camera, SET_DDE_GRADE,GF120_set_param.dde_grade,0);
}

void MindVision::infrared_exposure(int value)
{
    if(GF120_set_param.sample_mode == RESPONSE_SAMPLE_MODE) {
       GF120_set_param.exposure_time = value;
       CameraSpecialControl(camera, SET_EXPOSURE_TIME,value,0);
    }
}


void MindVision::infrared_factory_check_exposure(int value)
{
    CameraSpecialControl(camera, SET_EXPOSURE_TIME,value,0);
}


void MindVision::infrared_status(bool output) {
    //判断相机的名称
    auto gf120_enc = 0;
    if(strstr(camera_info.acProductName,"MV-GF120-3516") != 0)
        gf120_enc = 1;

    char cmd_buff[64];
    char result[512];
    if(gf120_enc)
    {
        memset(cmd_buff,0x0,sizeof(cmd_buff));
        sprintf(cmd_buff,"get_gf120_status()");
        CameraCommonCall(camera,cmd_buff,result,512);
        sscanf(result,"%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
               &GF120_get_param.gf120_dev_version,
               &GF120_get_param.gst417m_temp,
               &GF120_get_param.gst417m_tec_temp,
               &GF120_get_param.outside_temp,
               &GF120_get_param.shutter_temp,
               &GF120_get_param.gst417m_ctia,
               &GF120_get_param.low_temp,
               &GF120_get_param.low_temp_width,
               &GF120_get_param.low_temp_hight,
               &GF120_get_param.high_temp,
               &GF120_get_param.high_temp_width,
               &GF120_get_param.high_temp_hight,
               &GF120_get_param.center_temp,
               &GF120_get_param.user_temp,
               &GF120_get_param.ad_value,
               &GF120_get_param.diff_ad_value,
               &GF120_get_param.diff_temp_value,
               &GF120_get_param.calibration_high_temp,
               &GF120_get_param.calibration_low_temp,
               &GF120_get_param.gst417m_frame_cnt,
               &GF120_get_param.gst417m_frame_rate
               );
    }
    else
    {
        CameraSpecialControl(camera, GET_GF120_PARAM,0,&GF120_get_param);
    }

    infrared_sm.lock();
    memcpy(infrared_sm.data(),&GF120_get_param,sizeof(GF120_get_param));
    infrared_sm.unlock();

    if(output) {
        cout << "True" <<endl;
    }
}

void MindVision::infrared_manual(bool checked,short value) {
    GF120_set_param.manual_temp = value;
    if(checked)
    {
        GF120_set_param.manual_mode = 1;
        CameraSpecialControl(camera, SET_MANUAL_MODE,GF120_set_param.manual_mode,&GF120_set_param.manual_temp);
    }
    else
    {
        GF120_set_param.manual_mode = 0;
        CameraSpecialControl(camera, SET_MANUAL_MODE,GF120_set_param.manual_mode,&GF120_set_param.manual_temp);
    }
}

void MindVision::infrared_temperature_check() {
    CameraSpecialControl(camera, SET_TEMP_CHECK,0,0);
}

void MindVision::infrared_temperature_check_stop(bool checked) {
    GF120_set_param.stop_correct = checked;
    CameraSpecialControl(camera, SET_STOP_CORRECT,GF120_set_param.stop_correct,0);
}

void MindVision::infrared_factory_check_temperature_check_stop() {
    CameraSpecialControl(camera, SET_STOP_CORRECT,1,0);
}

void MindVision::infrared_shutter_temperature_raise_sample(bool checked) {
    GF120_set_param.sample_mode = checked ? SHUTTER_TEMP_SAMPLE_MODE : NO_SAMPLE_MODE;
    CameraSpecialControl(camera, SET_SAMPLE_MODE,GF120_set_param.sample_mode,0);
}

void MindVision::infrared_factory_check_detect(bool checked) {
    GF120_set_param.frame_temp_cnt = 10;
    CameraSpecialControl(camera, SET_FRAME_CAL,GF120_set_param.frame_temp_cnt,0);
    CameraSpecialControl(camera, SET_FACTORY_CHECK,checked,0);
}

void MindVision::infrared_response_rate_sample(bool checked) {
    GF120_set_param.sample_mode = checked ? RESPONSE_SAMPLE_MODE : NO_SAMPLE_MODE;
    CameraSpecialControl(camera, SET_SAMPLE_MODE,GF120_set_param.sample_mode,0);
}

void MindVision::infrared_temperature_curve_sample(bool checked) {
    GF120_set_param.sample_mode = checked ? TEMP_SAMPLE_MODE : NO_SAMPLE_MODE;
    CameraSpecialControl(camera, SET_SAMPLE_MODE,GF120_set_param.sample_mode,0);
}


void MindVision::infrared_frame_temp_cnt(int value) {
    GF120_set_param.frame_temp_cnt = 10;
    CameraSpecialControl(camera, SET_FRAME_CAL,GF120_set_param.frame_temp_cnt,0);
}

void MindVision::infrared_factory_check() {
    GF120_set_param.sample_mode = FACTORY_SAMPLE_MODE;
    CameraSpecialControl(camera, SET_SAMPLE_MODE,GF120_set_param.sample_mode,0);
}

void MindVision::infrared_params_status() {
    infrared_params_sm.lock();
    memcpy(infrared_params_sm.data(),&GF120_set_param,sizeof(GF120_set_param));
    infrared_params_sm.unlock();
}

void MindVision::infrared_sample_path(string path) {

}

void MindVision::infrared_response_rate_start(int value,string path) {
    CameraSpecialControl(camera, SET_RESPONSE_RATE_START,value,&path);
}

void MindVision::infrared_response_rate_status() {
    auto gf120_enc = 0;
    if(strstr(camera_info.acProductName,"MV-GF120-3516") != 0)
        gf120_enc = 1;

    if(gf120_enc == 0)
       CameraSpecialControl(camera, GET_RESPONSE_RATE_STATUS,0,&rt);
}

void MindVision::infrared_response_rate_stop() {
    CameraSpecialControl(camera, SET_RESPONSE_RATE_STOP,GF120_set_param.collect_response_temp,0);
}

void MindVision::infrared_load_response_rate_file(string path,string path2) {
    memset(GF120_set_param.low_response_file,0x00,sizeof(GF120_set_param.low_response_file));
    sprintf(GF120_set_param.low_response_file,"%s",path.c_str());

    memset(GF120_set_param.high_response_file,0x00,sizeof(GF120_set_param.high_response_file));
    sprintf(GF120_set_param.high_response_file,"%s",path2.c_str());

    CameraSpecialControl(camera, SET_LOAD_RESPONSE_FILE,0,&GF120_set_param.low_response_file);
}

void MindVision::infrared_cover_start(int value,string file_path) {
    CameraSpecialControl(camera, SET_COVER_START,value,&file_path);
}

void MindVision::infrared_cover_status() {
    auto rt = 0;
    CameraSpecialControl(camera, GET_COVER_STATUS,0,&rt);
}

void MindVision::infrared_cover_stop() {
    CameraSpecialControl(camera, SET_COVER_STOP,GF120_set_param.collect_cover_temp,0);
}

void MindVision::infrared_load_cover_file(string path,string path2) {
    memset(GF120_set_param.low_cover_file,0x00,sizeof(GF120_set_param.low_cover_file));
    sprintf(GF120_set_param.low_cover_file,"%s",path.c_str());

    memset(GF120_set_param.low_cover_file,0x00,sizeof(GF120_set_param.low_cover_file));
    sprintf(GF120_set_param.low_cover_file,"%s",path2.c_str());

    CameraSpecialControl(camera, SET_LOAD_COVER_FILE,0,&GF120_set_param.low_cover_file);
}

void MindVision::infrared_save_config(vector<string> filenames) {
    CameraSpecialControl(camera, SET_WRITE_CONFIG_FILES, 0, filenames.data());
}

void MindVision::infrared_delete_config() {
    CameraSpecialControl(camera, SET_WRITE_CONFIG_FILES, 0, NULL);
}

void MindVision::infrared_cmd(string cmd) {
    char result[64];
    CameraCommonCall(camera,cmd.c_str(),result,64);
}

void MindVision::infrared_osd(bool checked) {
    GF120_set_param.osd_enable = checked;
    CameraSpecialControl(camera, SET_OSD_ENABLE,GF120_set_param.osd_enable,0);
}

void MindVision::infrared_temperature_display(bool checked) {
    GF120_set_param.temp_display_enable = checked;
    if(checked)
    {
        GF120_set_param.osd_enable = 1;
        CameraSpecialControl(camera, SET_OSD_ENABLE,GF120_set_param.osd_enable,0);
    }
    CameraSpecialControl(camera, SET_OSD_TEMP_DISPLAY,GF120_set_param.temp_display_enable,0);
}

void MindVision::infrared_temperature_roi_status(bool output) {
    infrared_temperature_roi_sm.lock();
    memcpy(infrared_temperature_roi_sm.data(),&GF120_roi_temp,sizeof(GF120_roi_temp));
    infrared_temperature_roi_sm.unlock();

    if(output) {
        cout << "True" <<endl;
    }
}

void MindVision::infrared_roi(bool checked,int index,int user_width_start,int user_width_number,int user_high_start,int user_high_number,int user_roi_emissivity) {
    GF120_roi_temp[index].user_roi_enable = checked ? 1 : 0;

    GF120_roi_temp[index].user_width_start = user_width_start;
    GF120_roi_temp[index].user_width_number = user_width_number;
    GF120_roi_temp[index].user_high_start = user_high_start;
    GF120_roi_temp[index].user_high_number = user_high_number;
    GF120_roi_temp[index].user_roi_emissivity = user_roi_emissivity;

    char cmd_buff[64];
    char result[64];
    memset(cmd_buff,0x0,sizeof(cmd_buff));
    sprintf(cmd_buff,"set_user_roi_pos(%d %d %d %d %d %d)",index+1,user_width_start,user_high_start,user_width_number,user_high_number, user_roi_emissivity);
    printf("set_user_roi_pos buff:%s \r\n",cmd_buff);
    CameraCommonCall(camera,cmd_buff,NULL,0);

    memset(cmd_buff,0x0,sizeof(cmd_buff));
    sprintf(cmd_buff,"get_user_roi_pos(%d)",index+1);
    printf("get_user_roi_pos buff:%s \r\n",cmd_buff);
    memset(result,0x0,sizeof(result));
    CameraCommonCall(camera,cmd_buff,result,64);
    printf("get_user_roi_pos result:%s \r\n",result);

    memset(cmd_buff,0x0,sizeof(cmd_buff));
    sprintf(cmd_buff,"set_user_roi_enable(%d %s)",index+1,GF120_roi_temp[index].user_roi_enable ? "true":"flase");
    printf("set_user_roi_enable buff:%s \r\n",cmd_buff);
    CameraCommonCall(camera,cmd_buff,NULL,0);

    memset(cmd_buff,0x0,sizeof(cmd_buff));
    sprintf(cmd_buff,"get_user_roi_enable(%d)",index+1);
    printf("get_user_roi_enable buff:%s \r\n",cmd_buff);
    memset(result,0x0,sizeof(result));
    CameraCommonCall(camera,cmd_buff,result,64);
    printf("get_user_roi_enable result:%s \r\n",result);
}


void MindVision::infrared_blackbody_calibrate(bool checked,int blackbody_temprature,int user_width_start,int user_width_end,int user_high_start,int user_high_end) {
    GF120_set_param.GF120_calibration.user_calibration_enable = checked;
    GF120_set_param.GF120_calibration.user_calibration_temp = blackbody_temprature*100;
    GF120_set_param.GF120_calibration.user_width_start = user_width_start;
    GF120_set_param.GF120_calibration.user_width_end = user_width_end;
    GF120_set_param.GF120_calibration.user_high_start = user_high_start;
    GF120_set_param.GF120_calibration.user_high_end = user_high_end;
    CameraSpecialControl(camera, SET_CALIBRATION_ROI,0,&GF120_set_param.GF120_calibration);

    //取消黑体炉标定 马上进行一次校正
    if(GF120_set_param.GF120_calibration.user_calibration_enable == 0)
    {
        CameraSpecialControl(camera, SET_TEMP_CHECK,0,0);
    }
}


void MindVision::infrared_color_map(bool checked,int low,int high) {
    GF120_set_param.GF120_color.user_color_enable = checked;
    if(checked)
    {
        GF120_set_param.GF120_color.user_color_low= low;
        GF120_set_param.GF120_color.user_color_high = high;
        CameraSpecialControl(camera, SET_COLOR_PART,0,&GF120_set_param.GF120_color);
    }
    else
    {
         CameraSpecialControl(camera, SET_COLOR_PART,0,&GF120_set_param.GF120_color);
    }
}

void MindVision::infrared_temperature_compensation(int value) {
    GF120_set_param.user_compensate_temp = value;
    GF120_set_param.compensate_temp = GF120_set_param.user_compensate_temp + GF120_set_param.humidity_compensate_temp;
    CameraSpecialControl(camera, SET_COMPENSATE_TEMP,GF120_set_param.user_compensate_temp,0);
}

void MindVision::infrared_distance_compensation(int value) {
    GF120_set_param.distance_compensate_temp = value;
    CameraSpecialControl(camera, SET_DISTANCE_COMPENSATE_TEMP,GF120_set_param.distance_compensate_temp,0);
}

void MindVision::infrared_humidity_compensation(int value) {
    GF120_set_param.humidity_compensate_temp = value;
    GF120_set_param.compensate_temp = GF120_set_param.user_compensate_temp + GF120_set_param.humidity_compensate_temp;
    CameraSpecialControl(camera, SET_COMPENSATE_TEMP,GF120_set_param.compensate_temp,0);
}

void MindVision::infrared_high_warm(bool checked,int temperature) {
    if(checked)
    {
        GF120_set_param.high_temp_warm_temp = temperature;
        GF120_set_param.high_temp_warm_enable = 1;
    }
    else
    {
        GF120_set_param.high_temp_warm_enable = 0;
    }
}

void MindVision::infrared_low_warm(bool checked,int temperature) {
    if(checked)
    {
        GF120_set_param.low_temp_warm_temp = temperature;
        GF120_set_param.low_temp_warm_enable = 1;
    }
    else
    {
        GF120_set_param.low_temp_warm_enable = 0;
    }
}
