#include "RealSubApp.h"
#include "GlobalKeyValue.h"
#include "SerialHardwareManager.h"
#include <QMessageBox>


RealSubApp::RealSubApp(void)
{
	mbCameraState = false;
	mbRunChange = 0;
	mRealState = REAL_BENGINE_INIT;
}

RealSubApp::~RealSubApp(void)
{

}

void RealSubApp::ClearState()
{
	mbCameraState = false;
	mbRunChange = 0;
	mRealState = REAL_BENGINE_INIT;
	SerialHardwareManager::GetInstance()->SetDeviceLightLockState(SerialHard_Gyro, 0, 0);
}
void RealSubApp::BeginApply()
{
	mbRunChange = 1;
	mRealState = REAL_BENGINE_INIT;
}

void RealSubApp::ChangeApply()
{
	mbRunChange = 2;
}
RealSubApp* RealSubApp::GetInstance(void)
{
	static RealSubApp* deviceInstance = new RealSubApp();
	return deviceInstance;
}

void RealSubApp::Updata()
{
	if (1 == mbRunChange)
	{
		BeginCheck();
	}else if (2 == mbRunChange)
	{
		RunReplaceCheck();
	}
}

void RealSubApp::BeginCheck()
{
	// gResetButtonLeft == 0 器械伸进
	if (GlobalKeyValue::gRealDrawerState && mRealState != REAL_BENGINE_OPT_PULL)
	{
		if (0 == GlobalKeyValue::gResetButtonLeft &&0 ==  GlobalKeyValue::gResetButtonRight)
		{
			// 摄像头开启 训练开始
			mbCameraState = true;
			SerialHardwareManager::GetInstance()->SetDeviceLightLockState(SerialHard_Gyro, 1, 0);
		//	QMessageBox::information(0,"BeginCheck", "Begine");
			mRealState = REAL_BENGINE_SUCCESS;
			mbRunChange = 0;

		}else
		{
			// 插入器械 提示
			mbCameraState = false;
			SerialHardwareManager::GetInstance()->SetDeviceLightLockState(SerialHard_Gyro, 0, 0);
		//	QMessageBox::information(0,"BeginCheck", "push");
			mRealState = REAL_BENGINE_OPT_INSERT;
		}
	}else if (GlobalKeyValue::gRealDrawerState && mRealState == REAL_BENGINE_OPT_PULL)
	{
		mbCameraState = true;
		SerialHardwareManager::GetInstance()->SetDeviceLightLockState(SerialHard_Gyro, 1, 0);
		mRealState = REAL_BENGINE_ERROR_SUCCESS;
		mbRunChange = 0;
	}
	else 
	{
		mbCameraState = false;
		SerialHardwareManager::GetInstance()->SetDeviceLightLockState(SerialHard_Gyro, 0, 0);

		if (0 == GlobalKeyValue::gResetButtonLeft || 0 ==GlobalKeyValue::gResetButtonRight)
		{
			// 拔出器械 提示
		//	QMessageBox::information(0,"BeginCheck", "get");
			mRealState = REAL_BENGINE_OPT_PULL;
		}else if ( 1 == GlobalKeyValue::gResetButtonLeft && 1 == GlobalKeyValue::gResetButtonRight)
		{
			mRealState = REAL_BENGINE_INIT;
		}else if ( 0 == GlobalKeyValue::gResetButtonLeft && 0 == GlobalKeyValue::gResetButtonRight)
		{
			// 关闭抽屉 提示
		//	QMessageBox::information(0,"BeginCheck", "close");
			mRealState = REAL_BENGINE_OPT_CLOSE;
		}
	}

}

void RealSubApp::RunReplaceCheck()
{
	// 更换器械 提示
	if (GlobalKeyValue::gRealDrawerState)
	{		
		if (GlobalKeyValue::gResetButtonLeft > 0 && GlobalKeyValue::gResetButtonRight > 0)
		{
			// 拔出器械 提示
			SerialHardwareManager::GetInstance()->SetDeviceLightLockState(SerialHard_Gyro, 0, 1);
		//	QMessageBox::information(0,"RunReplaceCheck", "change");
			mRealState = REAL_REPLACE_OPD_PULL;
		}else if (GlobalKeyValue::gResetButtonLeft <= 0 && GlobalKeyValue::gResetButtonRight <= 0)
		{
			// 拔出器械 提示
			SerialHardwareManager::GetInstance()->SetDeviceLightLockState(SerialHard_Gyro, 0, 1);
			//	QMessageBox::information(0,"RunReplaceCheck", "change");
			mRealState = REAL_REPLACE_OPT_PULL;
		}
	}else
	{
		mbCameraState = false;
		if (GlobalKeyValue::gResetButtonLeft <=0 || GlobalKeyValue::gResetButtonRight <=0)
		{
			// 拔出器械 提示
		//	QMessageBox::information(0,"RunReplaceCheck", "get");
			mRealState = REAL_REPLACE_OPT_PULL;
		}
		if (GlobalKeyValue::gResetButtonLeft > 0 && GlobalKeyValue::gResetButtonRight > 0)
		{
			mbRunChange = 1;
			mRealState = REAL_REPLACE_SUCCESS;
			SerialHardwareManager::GetInstance()->SetDeviceLightLockState(SerialHard_Gyro, 0, 0);
		}
	}

}