#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

#ifdef WIN32
#include <Windows.h>
#undef min
#endif
#include <CameraApi.h>

INT YAcc(BYTE *pbyInBuffer,tSdkFrameHead *psFrameInfo,UINT *puiAccY)
{
        USHORT nWidth, nHeight;
        INT uHoff,uVoff,uWidth,uHeight;
        nWidth  = psFrameInfo->iWidth;
        nHeight = psFrameInfo->iHeight;
        BYTE *pTemp = pbyInBuffer;
        int i,j;
        UINT64 RAll = 0, GAll = 0, BAll = 0;
        float Rmean = 0;
        float Gmean = 0;
        float Bmean = 0;
        ULONG nSize = 0;
        //检查窗口是否在图像内部，不在内部，则归到中间4分之1大小,需要提示用户区域不能设置的太小，
        //但是此处不能限定，可能有些用户有特殊用途，需要设置很小的窗口

        uHoff =0;
        uWidth = psFrameInfo->iWidth;
        uVoff = 0;
        uHeight = psFrameInfo->iHeight;

        if (uWidth <= 0 || uHeight <= 0)
        {
                return CAMERA_STATUS_PARAMETER_INVALID;
        }

        if (psFrameInfo->uiMediaType == CAMERA_MEDIA_TYPE_MONO8)
        {
                for(j =uVoff; j< (uHeight + uVoff); j++)
                {
                        pTemp = pbyInBuffer + (j*nWidth) + uHoff;

                        for(i=0; i<uWidth; i++)
                        {
                                RAll += *pTemp++;
                                nSize ++;
                        }
                }
                *puiAccY = (RAll / nSize);
        }
        else
        {
                for(j =uVoff; j< (uHeight + uVoff); j++)
                {
                        pTemp = pbyInBuffer + (j*nWidth*3) + uHoff*3;

                        for(i=0; i<uWidth; i++)
                        {
                                BAll += *pTemp++;
                                GAll += *pTemp++;
                                RAll += *pTemp++;
                                nSize ++;
                        }
                }


                Rmean = (BYTE)(RAll/nSize);
                Gmean = (BYTE)(GAll/nSize);
                Bmean = (BYTE)(BAll/nSize);

                *puiAccY = (UINT)(0.299*Rmean+0.587*Gmean + 0.114*Bmean);
        }

        return CAMERA_STATUS_SUCCESS;
}
#endif // BRIGHTNESS_H
