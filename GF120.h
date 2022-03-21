#ifndef GF120_H
#define GF120_H

typedef unsigned short USHORT;
typedef short SHORT;
typedef unsigned int UINT;

//#define	GF120_VERSION		"V1.0.3"    //
//#define	GF120_VERSION		"V1.0.4"    //修改参数定义 方便控制
//#define	GF120_VERSION		"V1.0.6"    //增加了出厂一键校正功能
//#define	GF120_VERSION		"V1.0.7"    //增加温度数和伪彩色显示
//#define	GF120_VERSION		"V1.0.9"        //增加伪彩色切换
//#define	GF120_VERSION		"V1.0.10"        //增加软件和硬件触发自动保存 去掉巨型帧设置
//#define	GF120_VERSION		"V1.0.11"        //增加8个用户区域测温设置
//#define	GF120_VERSION		"V1.0.12"        //增加8个用户区域测温设置 并增加每个区域的发射率设置
#define	GF120_VERSION		"V1.0.13"        //增加冷机模式


typedef struct _WIDTH_HEIGHT {
    int     display_width;
    int     display_height;
    int     xOffsetFOV;
    int     yOffsetFOV;
    int     sensor_width;
    int     sensor_height;
    int     buffer_size;
} Width_Height;


#define		MAX_USER_ROI                        8

#define		RAINBOW_COLOR						0			//彩虹色
#define		HOT_METAL_COLOR						1			//热金属
#define		WHITE_FEVER_COLOR					2			//白热
#define		BLACK_FEVER_COLOR					3			//黑热

#define		TEMP_20_50      					0			//20-50摄氏度
#define		TEMP_40_150      					1			//-40-150摄氏度
#define		TEMP_40_500      					2			//40-500摄氏度

#define		IMAGE_TEMP_ENABLE                   0           //输出帧数据为温度数据
#define		IMAGE_RGB888_ENABLE                 1           //输出帧数据为RGB88数据
#define		IMAGE_RGB888_TEMP                	2           	//输出帧数据为RGB88数据和温度数据

#define		STOP_CORRECT_ENABLE                 1           //关闭自动校正
#define		STOP_CORRECT_DISABLE                0           //打开自动校正

#define		WINDOWS_QT_ENABLE                   1           //使用windows的qt例程
#define		WINDOWS_QT_DISABLE                  0           //没有使用windows的qt例程

#define		GF120_IO_CTL_BASE					(0x8000)
//用户需要使用的命令
#define		SET_TEMP_MODE						(GF120_IO_CTL_BASE+0) 	//设置测温模式
#define		SET_COLOR_MODE						(GF120_IO_CTL_BASE+1)	//伪彩色模式							彩虹色	热金属	白热	黑热
#define		SET_USER_EMISSIVITY					(GF120_IO_CTL_BASE+2)	//设置发射率							默认发射率0.98
#define		SET_SHARPEN_GRADE					(GF120_IO_CTL_BASE+3)	//设置锐化等级							默认锐化等级	0
#define		SET_RGB888_ENABLE					(GF120_IO_CTL_BASE+4)	//输出16bit灰度数据 或者伪彩色数据		输出为RGB伪彩色或者0.01摄氏度的温度值
#define		SET_TEMP_CHECK						(GF120_IO_CTL_BASE+5)	//执行一次温度校正
#define     SET_STOP_CORRECT                    (GF120_IO_CTL_BASE+6)	//停止快门校正
#define		GET_GF120_PARAM						(GF120_IO_CTL_BASE+7)	//获取所有实时数据
#define		SET_QT_IMG							(GF120_IO_CTL_BASE+8)	//windows下QT 数据需要RB对调和上下镜像 linux下QT不需要 演示SDK不需要
#define		SET_COLOR_PART						(GF120_IO_CTL_BASE+9)  //设置需要微彩映射的温度范围

#define		SET_OSD_ENABLE						(GF120_IO_CTL_BASE+10)	//打开OSD功能
#define		SET_OSD_TEMP_DISPLAY				(GF120_IO_CTL_BASE+11)	//打开OSD温度显示功能
#define		SET_OSD_ROI_DISPLAY					(GF120_IO_CTL_BASE+12)	//打开OSD区域温度显示功能

#define		SET_COMPENSATE_TEMP					(GF120_IO_CTL_BASE+13)  //设置总的温度补偿
#define		SET_CALIBRATION_ROI					(GF120_IO_CTL_BASE+14)	//设置黑体标定区域

#define		SET_SAMPLE_MODE                     (GF120_IO_CTL_BASE+16)	//设置采样模式
#define		SET_MANUAL_MODE                     (GF120_IO_CTL_BASE+17)	//设置手动模式
#define     SET_DISTANCE_COMPENSATE_TEMP        (GF120_IO_CTL_BASE+18)	//设置距离补偿

#define		SET_COVER_TEMP						(GF120_IO_CTL_BASE+19)	//锅盖采集温度
#define		SET_COVER_START						(GF120_IO_CTL_BASE+20) 	//开始采集指定温度的锅盖样本
#define		SET_LOAD_COVER_FILE					(GF120_IO_CTL_BASE+21)	//加载锅盖文件

#define		SET_FRAME_CAL						(GF120_IO_CTL_BASE+22)	//每多少帧统计一次温度 取温度均值
#define		SET_FACTORY_CHECK					(GF120_IO_CTL_BASE+23)	//
#define		SET_DDE_GRADE						(GF120_IO_CTL_BASE+24)	//DDE 增强等级
#define		SET_FRAME_RATE						(GF120_IO_CTL_BASE+25)	//设置帧率
#define		SET_ROI_COLOR						(GF120_IO_CTL_BASE+26)	//设置ROI色彩

#define		SET_COLD_MODE                       (GF120_IO_CTL_BASE+27)	//设置非连续模式

//校正需要使用命令 用户不需要使用
#define		SET_RESPONSE_RATE_TEMP				(GF120_IO_CTL_BASE+32)	//响应率采集温度
#define		SET_RESPONSE_RATE_START				(GF120_IO_CTL_BASE+33)  //开始采集指定温度的响应率样本
#define		SET_COVER_STOP						(GF120_IO_CTL_BASE+34)  //开始采集指定温度的锅盖样本
#define		SET_RESPONSE_RATE_STOP				(GF120_IO_CTL_BASE+35) 	//开始采集指定温度的响应率样本
#define		SET_LOAD_RESPONSE_FILE				(GF120_IO_CTL_BASE+36)	//加载响应率文件
#define		SET_EXPOSURE_TIME					(GF120_IO_CTL_BASE+37)	//设置曝光时间
#define		SET_SHUTTER_MODE                    (GF120_IO_CTL_BASE+38)	//打开或者关闭快门
#define		GET_COVER_STATUS					(GF120_IO_CTL_BASE+39)	//获取锅盖采样结果
#define		GET_RESPONSE_RATE_STATUS			(GF120_IO_CTL_BASE+40)	//获取响应率采样结果
#define		SET_WRITE_CONFIG_FILES				(GF120_IO_CTL_BASE+41)	//保存校正文件到相机


#define		NO_SAMPLE_MODE                      0
#define		RESPONSE_SAMPLE_MODE                1	//响应率采集
#define		TEMP_SAMPLE_MODE                    2	//温度曲线采集
#define		SHUTTER_TEMP_SAMPLE_MODE            3	//快门温升采集
#define		SENSOR_TEMP_SAMPLE_MODE             4	//SENSOR板温升采集
#define		COVER_SAMPLE_MODE                   5	//锅盖采集
#define		FACTORY_SAMPLE_MODE                 6	//出厂校正

struct GF120_roi_t {
    USHORT		user_roi_enable;			//用户设置区域检测温度功能打开
    USHORT		user_width_start;			//用户设置区域检测温度区宽开始点
    USHORT		user_width_number;			//用户设置区域检测温度区宽像素点个数
    USHORT		user_high_start;			//用户设置区域检测温度区高开始点
    USHORT		user_high_number;			//用户设置区域检测温度区高素点个数
    USHORT		user_roi_emissivity;
} GF120_roi_temp[MAX_USER_ROI];


struct GF120_calibration_t{
    USHORT		user_calibration_enable;	//用户设置区域检测温度功能打开
    USHORT		user_calibration_temp;		//黑体炉设置温度
    USHORT		user_width_start;			//用户设置区域检测温度区宽开始点
    USHORT		user_width_end;				//用户设置区域检测温度区宽结束点
    USHORT		user_high_start;			//用户设置区域检测温度区高开始点
    USHORT		user_high_end;				//用户设置区域检测温度区高结束点
};

struct GF120_color_t
{
    USHORT		user_color_enable;				//用户设置 温度微彩范围 三段曲线的中间两个点
    USHORT		user_color_high;                //用户设置 低温色彩
    USHORT		user_color_low;                 //用户设置 高温色彩
};

//用户设置的参数和用户需要获取的参数
//用户设置的参数和用户需要获取的参数
struct GF120_set_param_t
{
    USHORT		temp_mode;					//三种测温区间	-40℃~150℃	-40~800℃	20~50℃
    USHORT		sample_mode;				//样本采样模式
    USHORT      manual_mode;                //手动模式
    SHORT      manual_temp;                 //手动模式温度
    USHORT		stop_correct;				//停止校正
    USHORT		shutter_on;					//快门强制操作
    USHORT		collect_cover_sample;				//采集锅盖现象样本使用
    SHORT		collect_cover_temp;				//采集锅盖现象样本使用
    USHORT		collect_response_sample;
    SHORT		collect_response_temp;
    USHORT		response_rate_sample;		//响应率样本采集 高低温个一个
    USHORT		color_mode;					//伪彩色编码种类 支持用户调色板 256个RGB色度值
    USHORT		user_emissivity;			//设置的发射率
    USHORT		sharpen_grade;				//锐化等级 设置我0 不锐化
    USHORT		exposure_time;				//曝光时间
    USHORT		out_rgb888_enable;				//输出8bit伪彩色图形 否则输出16位灰度数据
    USHORT		osd_enable;					//在输出伪彩色图像时是否添加OSD显示，最高最低温度
    USHORT		qt_img;						//qt需要RB对调 和上下镜像
    USHORT		temp_display_enable;					//在输出伪彩色图像时是否添加OSD显示，最高最低温度
    SHORT		user_compensate_temp;                   //用户自定义补偿
    SHORT		humidity_compensate_temp;               //湿度补偿
    SHORT		distance_compensate_temp;               //距离补偿
    SHORT		compensate_temp;                        //三种补偿和
    USHORT      high_temp_warm_enable;              //高温报警使能
    USHORT      high_temp_warm_temp;                 //高温报警温度
    USHORT      low_temp_warm_enable;              //高温报警使能
    USHORT      low_temp_warm_temp;                 //高温报警温度
    char		file_path[256];                     //低温锅盖采样文件
    char		low_cover_file[256];				//低温锅盖采样文件
    char		high_cover_file[256];				//高温锅盖采样文件
    char		low_response_file[256];				//低温响应率采样文件
    char		high_response_file[256];			//高温响应率采样文件
    GF120_roi_t		GF120_roi;
    GF120_color_t	GF120_color;
    GF120_calibration_t  GF120_calibration;
    UINT		frame_temp_cnt;					//多少帧进行一次温度统计 伪彩色使用25帧统计一次 抓图可以这是为1帧统计一
    UINT		dde_grade;						//锐化等级 设置我0 不锐化
    USHORT		frame_rate_set;						//锐化等级 设置我0 不锐化
    USHORT		ROI_R_COLOR;						//ROI R色彩
    USHORT		ROI_G_COLOR;						//ROI G色彩
    USHORT		ROI_B_COLOR;						//ROI B色彩
    USHORT		cold_mode;                          //非连续模式
}*pGF120_set_param;

#define GF120_set_param (*pGF120_set_param)


struct GF120_get_param_t
{
    char        gf120_dev_version[32];  //sdk的版本号
    USHORT		frame_val;				//指示改帧是否有效，在校正其间传输的是校正前的静止图像
    USHORT		gst417m_temp;			//红外相机焦面温度
    USHORT		gst417m_tec_temp;		//红外相机设置温度
    SHORT		outside_temp;           //红外相机设置温度
    SHORT		shutter_temp;			//红外相机快门温度
    USHORT		gst417m_ctia;           //相机参数
    int			low_temp;				//最低温值
    USHORT		low_temp_width;			//最低温宽坐标
    USHORT		low_temp_hight;			//最低温高坐标
    int			high_temp;				//最高温值
    USHORT		high_temp_width;		//最高温宽坐标
    USHORT		high_temp_hight;		//最高温高坐标
    int			center_temp;			//图像中心位置温度
    int			user_temp;				//用户设置区域温度
    USHORT		ad_value;
    SHORT		diff_ad_value;
    SHORT		diff_temp_value;
    char		config_dir[256];		//配置文件目录
    int			calibration_high_temp;				//高温黑体炉温度
    int			calibration_low_temp;				//低温黑体炉温度
    int			gst417m_frame_cnt;					//相机输出帧个数
    int			gst417m_frame_rate;					//统计到的帧率
} GF120_get_param;

#include <memory>

void parameter_init()
{
    memset(&GF120_set_param,0x00,sizeof(GF120_set_param));
    GF120_set_param.temp_mode = 1;					//三种测温区间	-40℃~150℃	-40~800℃	20~50℃
    GF120_set_param.manual_mode = 0;
    GF120_set_param.collect_cover_sample = 0;				//采集锅盖现象样本使用
    GF120_set_param.collect_cover_temp = 20;
    GF120_set_param.response_rate_sample = 0;		//响应率样本采集 高低温个一个
    GF120_set_param.collect_response_temp = 40;
    GF120_set_param.color_mode = 0;		//伪彩色编码种类 支持用户调色板 256个RGB色度值
    GF120_set_param.user_emissivity = 98;			//设置的发射率
    GF120_set_param.sharpen_grade = 0;	//锐化等级 设置我0 不锐化
    GF120_set_param.exposure_time = 128;
    GF120_set_param.out_rgb888_enable = 2;				//输出8bit伪彩色图形 否则输出16位灰度数据
    GF120_set_param.osd_enable = 1;					//在输出伪彩色图像时是否添加OSD显示，最高最低温度
    GF120_set_param.qt_img = 1;
    GF120_set_param.temp_display_enable = 1;
    GF120_set_param.user_compensate_temp = 0;
    GF120_set_param.humidity_compensate_temp = 0;
    GF120_set_param.distance_compensate_temp = 150;
    GF120_set_param.compensate_temp = GF120_set_param.user_compensate_temp + GF120_set_param.humidity_compensate_temp;                        //三种补偿和
    GF120_set_param.high_temp_warm_enable = 0;
    GF120_set_param.high_temp_warm_temp = 3750;
    GF120_set_param.low_temp_warm_enable = 0;
    GF120_set_param.low_temp_warm_temp = 0;




    GF120_set_param.GF120_roi.user_roi_enable = 0;			//用户设置区域检测温度功能打开
    GF120_set_param.GF120_roi.user_width_start = 270;			//用户设置区域检测温度区宽开始点
    GF120_set_param.GF120_roi.user_width_number = 280;			//用户设置区域检测温度区宽结束点
    GF120_set_param.GF120_roi.user_high_start = 180;			//用户设置区域检测温度区高开始点高结束点
    GF120_set_param.GF120_roi.user_high_number = 190;			//用户设置区域检测温度区

    memset(&GF120_roi_temp,0x00,sizeof(GF120_roi_temp));
    int m;
    for(m = 0 ; m < 8 ; m ++)
    {
        GF120_roi_temp[m].user_roi_enable = 0;			//用户设置区域检测温度功能打开
        GF120_roi_temp[m].user_width_start = 20 + 40*m;			//用户设置区域检测温度区宽开始点
        GF120_roi_temp[m].user_width_number =   40;			//用户设置区域检测温度区宽结束点
        GF120_roi_temp[m].user_high_start = 20 + 30*m;			//用户设置区域检测温度区高开始点高结束点
        GF120_roi_temp[m].user_high_number = 25;			//用户设置区域检测温度区
        GF120_roi_temp[m].user_roi_emissivity = 98;
    }
    GF120_set_param.GF120_color.user_color_enable = 0;	//默认不打开自定义色彩映射
    GF120_set_param.GF120_color.user_color_low = 2000;
    GF120_set_param.GF120_color.user_color_high = 4000;

    GF120_set_param.GF120_calibration.user_calibration_enable = 0;//默认不打开黑体校正
    GF120_set_param.GF120_calibration.user_calibration_temp = 3700;
    GF120_set_param.GF120_calibration.user_width_start = 340;
    GF120_set_param.GF120_calibration.user_width_end = 380;
    GF120_set_param.GF120_calibration.user_high_start = 240;
    GF120_set_param.GF120_calibration.user_high_end = 280;
    GF120_set_param.frame_temp_cnt = 1;
}

#endif // GF120_H
