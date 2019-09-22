/**×¢ÉäÆ÷**/
#pragma once
#include "tool.h"
#include	"Painting.h"

class CXMLWrapperTool;



class CInjectNeedle : public CTool , public GFPhysSoftBodyConstraint
{
public:
	struct NeedleHeadData
	{
		GFPhysVector3 m_HeadPos;
		GFPhysVector3 m_EndPos;
		
		GFPhysVector3 m_InsertAxis;
		GFPhysVector3 m_Axis0;	//like right axis
		GFPhysVector3 m_Axis1;//like up axis


		GFPhysVector3 m_Temp1;
		GFPhysVector3 m_Temp2;


		GFPhysVector3 m_TempX;
		GFPhysVector3 m_TempY;
		GFPhysVector3 m_TempZ;
	};

	struct InsertInfoOfFace
	{
		GFPhysSoftBodyFace * m_pFace;

		float m_PartOfAxis0;

		float m_PartOfAxis1;

		float m_PartOfInsertAxis;

		float m_DistOfHeadToUncorrect;
		
		float m_PosInsertedWeights[3];

		GFPhysVector3 m_LocalAnchor[3];

		InsertInfoOfFace():m_pFace(NULL){}
	};

	CInjectNeedle();
	CInjectNeedle(CXMLWrapperTool * pToolConfig);
	~CInjectNeedle();

	bool Initialize(CXMLWrapperTraining * pTraining);
	std::string GetCollisionConfigEntryName();

	virtual bool Update(float dt);
	
	virtual void onFrameUpdateStarted(float timeelapsed);

	virtual void onFrameUpdateEnded();

	virtual void InternalSimulationStart(int currStep , int TotalStep , float dt);
	
	virtual void InternalSimulationEnd(int currStep , int TotalStep , float dt);

	virtual GFPhysVector3 CalculateToolCustomForceFeedBack();

	//@overridden GFPhysSoftBodyConstraint
	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	//@overridden GFPhysSoftBodyConstraint
	void SolveConstraint(Real Stiffniss,Real TimeStep);

	GFPhysRigidBody * GetRigidBodyOfNeedleHead();
	
	void ClearInfoOfInsertion();

	//GFPhysSoftBodyFace * GetFaceInjected() { return m_pFaceInjected; }
	int GetIndexOfFaceInjected() { return m_IndexOfFaceInjected;}
	
	//if m_pFaceInjected == NULL , return false
	bool GetInjectedPosWeights(float weights[]);
	
	GFPhysSoftBody * GetObjectInjected() { return m_pBodyInserted;}

	
private:
	void CheckAfterInserting(float dt);

	void CheckWhenNotInserted(float dt);

//info of insertion
	GFPhysQuaternion m_QuatAtInsertion;

	int m_IndexOfFaceInjected;

	GFPhysSoftBodyFace * m_pFaceInserted;

	float m_PosInsertedWeights[3];

	GFPhysSoftBody * m_pBodyInserted;
//info of insertion
	InsertInfoOfFace m_InsertInfo;

	bool m_WillInsert;
	bool m_IsInsert;

	std::vector<GFPhysSoftBodyFace *> m_pFacesContact;
	
	NeedleHeadData m_NeedleHeadLocal;
	NeedleHeadData m_NeedleHeadWorld;

	GFPhysVector3 m_LastInsertDir;
	GFPhysVector3 m_LastHeadPos;

	GFPhysVector3 m_Force;

	float m_PunctureValue;

	PaintingTool m_painting;

};
