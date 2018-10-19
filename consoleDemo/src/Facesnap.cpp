/*
* Copyright(C) 2010,Hikvision Digital Technology Co., Ltd 
* 
* File   name£ºCapPicture.cpp
* Discription£º
* Version    £º1.0
* Author     £ºpanyd
* Create Date£º2010_3_25
* Modification History£º
*/

#include "public.h"
#include "Facesnap.h"
#include <stdio.h>
#include <iostream>
#include "HCNetSDK.h"
#include <stdlib.h>
#include "string.h"
#include <unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>



#define GET_YEAR(_time_)      (((_time_)>>26) + 2000) 
#define GET_MONTH(_time_)     (((_time_)>>22) & 15)
#define GET_DAY(_time_)       (((_time_)>>17) & 31)
#define GET_HOUR(_time_)      (((_time_)>>12) & 31) 
#define GET_MINUTE(_time_)    (((_time_)>>6)  & 63)
#define GET_SECOND(_time_)    (((_time_)>>0)  & 63)
using namespace std;


BOOL CALLBACK FaceMessageCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser){
	
	switch(lCommand){
	       	case COMM_UPLOAD_FACESNAP_RESULT: 
		{
			NET_VCA_FACESNAP_RESULT struFaceSnap = {0};
			memcpy(&struFaceSnap, pAlarmInfo, sizeof(NET_VCA_FACESNAP_RESULT));
		       	
			NET_DVR_TIME struAbsTime = {0};
			struAbsTime.dwYear = GET_YEAR(struFaceSnap.dwAbsTime);
			struAbsTime.dwMonth = GET_MONTH(struFaceSnap.dwAbsTime);
			struAbsTime.dwDay = GET_DAY(struFaceSnap.dwAbsTime);
		       	struAbsTime.dwHour = GET_HOUR(struFaceSnap.dwAbsTime);
			struAbsTime.dwMinute = GET_MINUTE(struFaceSnap.dwAbsTime);
			struAbsTime.dwSecond = GET_SECOND(struFaceSnap.dwAbsTime);
			//save the face snap file
			if (struFaceSnap.dwFacePicLen > 0 && struFaceSnap.pBuffer1 != NULL){
				cout << "Saving the face pic";
				char cFilename[256] = {0};
				int hFile;
				char chTime[128];
				sprintf(chTime,"%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d",struAbsTime.dwYear,struAbsTime.dwMonth,struAbsTime.dwDay,struAbsTime.dwHour,struAbsTime.dwMinute,struAbsTime.dwSecond);
				sprintf(cFilename, "FaceSnapBackPic[%s][%s].jpg",struFaceSnap.struDevInfo.struDevIP.sIpV4, chTime);
				hFile = open(cFilename,O_RDWR|O_CREAT,0755);
				if (hFile < 0) {
					break;
			       	}
			       	write(hFile, struFaceSnap.pBuffer1, struFaceSnap.dwFacePicLen);
				close(hFile);
				hFile = -1;
			}
			printf("Face snap alarm![0x%x]: Abs[%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d] Dev[ip:%s,port:%d,ivmsChan:%d] \n",\
				lCommand, struAbsTime.dwYear, struAbsTime.dwMonth, struAbsTime.dwDay, struAbsTime.dwHour, \
				struAbsTime.dwMinute, struAbsTime.dwSecond, struFaceSnap.struDevInfo.struDevIP.sIpV4, \
				struFaceSnap.struDevInfo.wPort, struFaceSnap.struDevInfo.byIvmsChannel);
	       	}
		break;
	       	default:
		printf("Catch another alarm: 0x%x\n", lCommand);
		break;
	}
	return TRUE;
}







/*******************************************************************
      Function:   Demo_Facesnap
   Description:   Face snap function test.
     Parameter:   (IN)   none 
        Return:   0--success£¬-1--fail.   
**********************************************************************/
int Demo_Facesnap()
{
    NET_DVR_Init();
    NET_DVR_SetConnectTime(2000, 1);
    NET_DVR_SetReconnect(10000, true);

    
    long lUserID;
    //login
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    struLoginInfo.bUseAsynLogin = 0; //sync login mode
    strcpy(struLoginInfo.sDeviceAddress, "192.168.1.64"); //login ip
    struLoginInfo.wPort = 8000; //service port
    strcpy(struLoginInfo.sUserName, "admin"); //login name
    strcpy(struLoginInfo.sPassword, "root12345"); //passwd

    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};
    lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    if (lUserID < 0){
	    printf("Login failed, error code: %d\n", NET_DVR_GetLastError());
	    NET_DVR_Cleanup();
	    return HPR_ERROR;
    }
    
    NET_DVR_SetDVRMessageCallBack_V31(FaceMessageCallback, NULL);
    
    LONG lHandle;
    NET_DVR_SETUPALARM_PARAM  struAlarmParam={0};
    struAlarmParam.dwSize=sizeof(struAlarmParam);
    struAlarmParam.byFaceAlarmDetection = 0;// set to face snap alarm

    lHandle = NET_DVR_SetupAlarmChan_V41(lUserID, & struAlarmParam);
    if (lHandle < 0){
	    printf("NET_DVR_SetupAlarmChan_V41 error, %d\n", NET_DVR_GetLastError());
	    NET_DVR_Logout(lUserID);
	    NET_DVR_Cleanup();
	    return HPR_ERROR;
    }
    
    int userInput = 1;
    cout << "pls key in 0 when you want to quit:";
    for(;1;){
    	cin >> userInput;
	if(userInput == 0){
		break;
	}
    }


    if (!NET_DVR_CloseAlarmChan_V30(lHandle))
    {
	    printf("NET_DVR_CloseAlarmChan_V30 error, %d\n", NET_DVR_GetLastError());
	    NET_DVR_Logout(lUserID);
	    NET_DVR_Cleanup();
	    return HPR_ERROR ;
    }



    //logout
    NET_DVR_Logout(lUserID);
    NET_DVR_Cleanup();

    return HPR_OK;

}
