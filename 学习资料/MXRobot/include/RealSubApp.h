#pragma once

enum REAL_DATA_STATE
{
	REAL_BENGINE_INIT	= 0,
	REAL_BENGINE_SUCCESS = 1,
	REAL_BENGINE_ERROR_SUCCESS = 2,

	REAL_BENGINE_OPT_INSERT = 3,
	REAL_BENGINE_OPT_PULL	= 11,
	REAL_BENGINE_OPT_CLOSE	= 12,

	REAL_REPLACE_SUCCESS	= 21,
	REAL_REPLACE_OPT_PULL	= 22,
	REAL_REPLACE_OPT_CLOSE	= 23,

	REAL_REPLACE_OPD_PULL	=33,
};

class RealSubApp
{
public:
	RealSubApp(void);
	~RealSubApp(void);

public:
	void Updata();
	void ClearState();
	
	void BeginApply();
	void ChangeApply();
protected:
	void BeginCheck();
	void RunReplaceCheck();
public:
	static RealSubApp* GetInstance(void);

public:
	inline bool getCameraState(){return mbCameraState;}; 
	inline REAL_DATA_STATE getRealState(){return mRealState;};
private:
	bool mbCameraState;

	REAL_DATA_STATE mRealState;
	int mbRunChange;
};
