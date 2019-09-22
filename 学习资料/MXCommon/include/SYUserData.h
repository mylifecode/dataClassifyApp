#pragma once
#include <MXCommon.h>
#include <QString>

enum SYUserDataType
{
	UDT_UserData,
	UDT_Train,
	UDT_Step,
	UDT_ScoreItem,
	UDT_ScorePointDetail
};

class MXCOMMON_API SYUserData
{
public:
	SYUserData(SYUserData* parent = nullptr);

	SYUserData(const QString& code);

	virtual ~SYUserData();

	SYUserData* GetParent() const { return m_parent; }
	void SetParent(SYUserData* parent) { m_parent = parent; }

	const QString OldCode() const { return m_oldCode; }
	const QString& Code() const { return m_code; }
	void SetCode(const QString& code) 
	{ 
		if(code != m_code){
			m_oldCode = m_code;
			m_code = code;
			m_dataHasChanged = true;
			m_codeHasChanged = true;
		}
	}

	bool DataHasChanged() const { return m_dataHasChanged; }

	bool CodeHasChanged() const { return m_codeHasChanged; }

	virtual void ResetDataState()
	{
		m_dataHasChanged = false;
		m_codeHasChanged = false;
	}

	SYUserDataType DataType() { return m_dataType; }

	void SetFlag(int flag) { m_flag = flag; }
	int GetFlag() const { return m_flag; }


protected:
	SYUserData* m_parent;

	SYUserDataType m_dataType;

	/// �Ƿ������ݱ��޸�
	bool m_dataHasChanged;

	/// ����ֵ�Ƿ����ı�
	bool m_codeHasChanged;

private:
	/// ���ݱ���
	QString m_code;

	/// �ɵ����ݱ���
	QString m_oldCode;

	/// Ĭ��Ϊ0
	int m_flag;
};

class MXCOMMON_API SYTrainData : public SYUserData
{
public:
	SYTrainData();

	SYTrainData(const QString& code);

	void SetName(const QString& name) 
	{
		if(name != m_name){
			m_name = name;
			m_dataHasChanged = true;
		}
		
	}
	const QString& GetName() const { return m_name; }

private:
	/// ѵ����
	QString m_name;
};

class MXCOMMON_API SYStepData : public SYUserData
{
public:
	SYStepData();

	SYStepData(const QString& code);

	void SetStepDescription(const QString& description)
	{ 
		if(description != m_stepDescription){
			m_stepDescription = description;
			m_dataHasChanged = true;
		}
	}

	const QString& GetStepDescription() const { return m_stepDescription; }

	void SetStepDetailDescription(const QString& detailedDescription) 
	{
		if(detailedDescription != m_stepDetailedDescription){
			m_stepDetailedDescription = detailedDescription;
			m_dataHasChanged = true;
		}
		
	}
	const QString& GetStepDetailedDescription() const { return m_stepDetailedDescription; }

	/** �ؼ������Ƿ񱻸ı� */
	bool KeyStepChanged() const { return m_keyStepChanged; }
	int OldKeyStep() const { return m_oldKeyStepValue; }

	int IsKeyStep() const { return m_isKeyStep; }
	void SetKeyStep(bool isKeyStep) 
	{ 
		int curValue = isKeyStep ? 1 : 0;
		if(curValue != m_isKeyStep){
			m_oldKeyStepValue = m_isKeyStep;
			m_isKeyStep = curValue;
			//m_codeHasChanged = true;		//�Ƿ�Ϊ�ؼ�����Ϊ�����һ����
			m_dataHasChanged = true;
			m_keyStepChanged = true;
		}
	}

	virtual void ResetDataState()
	{
		SYUserData::ResetDataState();
		m_keyStepChanged = false;
	}



private:
	/// ����˵������Ӧ���ֱ��е�[��������]
	QString m_stepDescription;

	/// ������ϸ˵������Ӧ���ֱ��е�[��ȷ����]
	QString m_stepDetailedDescription;

	/// �Ƿ�Ϊ�ؼ���������,Ĭ��Ϊ1
	int m_isKeyStep;
	int m_oldKeyStepValue;

	bool m_keyStepChanged;
};

/**
	��Ӧ���ֱ��еķ�����
*/
class MXCOMMON_API SYScoreItemData : public SYUserData
{
public:
	SYScoreItemData();

	SYScoreItemData(const QString& code);

	void SetScoreContent(const QString& content) 
	{ 
		if(content != m_scoreContent){
			m_scoreContent = content;
			m_dataHasChanged = true;
		}
	}
	const QString& GetScoreContent() const { return m_scoreContent; }

	int GetScoreValue() const { return m_scoreValue; }
	void SetScoreValue(int value) 
	{ 
		if(value != m_scoreValue){
			m_scoreValue = value;
			m_dataHasChanged = true;
		}
	}

private:

	/// ��������
	QString m_scoreContent;

	/// ��ֵ
	int m_scoreValue;
};


/** ���ֵ�ϸ�� */
class MXCOMMON_API SYScorePointDetailData : public SYUserData
{
public:
	SYScorePointDetailData();

	SYScorePointDetailData(const QString& code);

	void SetDescription(const QString& des) 
	{ 
		if(des != m_description){
			m_description = des;
			m_dataHasChanged = true;
		}
	}

	const QString& GetDescription() const { return m_description; }

	void SetScoreValue(int value) 
	{ 
		if(m_scoreValue != value){
			m_scoreValue = value;
			m_dataHasChanged = true;
		}
	}

	int GetScoreValue() const { return m_scoreValue; }

//	void SetType(int type) { m_type = type; }

	void SetAbilitys(int abilitys) 
	{ 
		if(abilitys != m_abilitys){
			m_abilitys = abilitys;
			m_dataHasChanged = true;
		}
	}

	int GetAbilitys() const { return m_abilitys; }

private:
	/// ���ֵ���������Ӧ���ֱ���[���ֵ���ϸ�����������]
	QString m_description;

	int m_scoreValue;

//	/// ��Ӧ���ֱ��е�������������
//	int m_type;

//	/// �Ƿ�Ϊ�ؼ���������
//	int m_isKeyStep;

	/// ��������ֵ������϶������ֵ
	int m_abilitys;
};