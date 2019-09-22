#include "StdAfx.h"
#include "MXOgreGraphic.h"
#include <QFile>
#include <OgreConfigFile.h>
#include "OgreTextureManager.h"
#include "OgreMaterialManager.h"
#include "OgreParticleSystemManager.h"
#include "OgreOverlayManager.h"
#include "OgreCompositorManager.h"
#include "OgreCompositor.h"
#include "HelperLogics.h"
#include "DynamicObjectRenderable.h"
//#include "DynamicStrips.h"
#include "ITraining.h"
#include "ShadowMap.h"
#include "NewTrain\VeinConnectRender.h"


void ApplyTextureToMaterial(Ogre::MaterialPtr mat , Ogre::TexturePtr tex , const Ogre::String & unitName)
{	
	if(mat.isNull() == false && tex.isNull() == false)
	{
		Ogre::Technique * tech = mat->getTechnique(0);

		if(tech->getNumPasses() > 0)
		{
			Ogre::Pass * pass = tech->getPass(0);
			for (int  t = 0; t < pass->getNumTextureUnitStates() ; t++)
			{
				Ogre::TextureUnitState * texunit = pass->getTextureUnitState(t);
				if(texunit->getTextureNameAlias() == unitName )
					texunit->setTextureName(tex->getName());
			}
		}
	}
}

void ApplyTextureToMaterial(Ogre::String materialname, Ogre::TexturePtr texturetoapp , Ogre::String textureunitname)
{
	Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(materialname);

	if(material.isNull() == false && texturetoapp.isNull() == false)
	{
		Ogre::Technique * tech = material->getTechnique(0);

		if(tech->getNumPasses() > 0)
		{
			Ogre::Pass * pass = tech->getPass(0);
			for (int  t = 0; t < pass->getNumTextureUnitStates() ; t++)
			{
				Ogre::TextureUnitState * texunit = pass->getTextureUnitState(t);
				if(texunit->getTextureNameAlias() == textureunitname )
					texunit->setTextureName(texturetoapp->getName());
			}
		}
	}
}

void ApplyTextureToMaterial(Ogre::String materialname, Ogre::TexturePtr effecttex , int texunit)
{
	if(effecttex.isNull() == false)
	{
		Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(materialname);
		if(material.isNull() == false &&material->getNumTechniques() > 0 )
		{
			Ogre::Pass * pass = material->getTechnique(0)->getPass(0);
			if(pass && pass->getNumTextureUnitStates() >= texunit+1)
				material->getTechnique(0)->getPass(0)->getTextureUnitState(texunit)->_setTexturePtr(effecttex);
		}
	}
}

bool GetMaterialTextureName(Ogre::String materialname, const Ogre::String & nameAlias , Ogre::String & texName)
{
	Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(materialname);

	if(material.isNull() == false)
	{
		Ogre::Technique * tech = material->getTechnique(0);

		if(tech->getNumPasses() > 0)
		{
			Ogre::Pass * pass = tech->getPass(0);
			for (int  t = 0; t < pass->getNumTextureUnitStates() ; t++)
			{
				Ogre::TextureUnitState * texunit = pass->getTextureUnitState(t);
				if(texunit->getTextureNameAlias() == nameAlias )
				{
					texName = texunit->getFrameTextureName(0);
					return true;
				}
			}
		}
	}
	return false;
}

Ogre::GpuProgramParametersSharedPtr GetShaderParamterPtr(Ogre::String materialName, CG_PROGRAME_TYPE type, int techindex, int passindex)
{
	Ogre::GpuProgramParametersSharedPtr ShaderParamPtr;

	Ogre::MaterialPtr pMatPtr = Ogre::MaterialManager::getSingleton().getByName(materialName);
	if (pMatPtr.isNull())
		return Ogre::GpuProgramParametersSharedPtr();	

	if(techindex >= 0 && techindex < pMatPtr->getNumTechniques())
	{
		Ogre::Technique * theTech = pMatPtr->getTechnique(techindex);
		if (theTech == NULL)
			return Ogre::GpuProgramParametersSharedPtr();

		if(passindex >= 0 && passindex < theTech->getNumPasses())
		{
			Ogre::Pass* thePass = theTech->getPass(passindex);
			if (thePass == NULL)
				return Ogre::GpuProgramParametersSharedPtr();

			switch (type)
			{
			case VERTEX_PROGRAME:
				if (!thePass->hasVertexProgram())
				{
					return Ogre::GpuProgramParametersSharedPtr();
				}
				else
				{
					ShaderParamPtr = thePass->getVertexProgramParameters();
					return ShaderParamPtr;
				}
				break;

			case	FRAGMENT_PROGRAME:
				if (!thePass->hasFragmentProgram())
				{
					return Ogre::GpuProgramParametersSharedPtr();
				}
				else
				{
					ShaderParamPtr = thePass->getFragmentProgramParameters();
					return ShaderParamPtr;
				}
				break;
			default:
				return Ogre::GpuProgramParametersSharedPtr();
			}
		}
	}

	return Ogre::GpuProgramParametersSharedPtr();;
}

//===============================================================================================
bool CalcPlaneNormalByRegress(const GFPhysAlignedVectorObj<GFPhysVector3> & positions,GFPhysVector3 & normal)
{
	int m = (int)positions.size();

	GFPhysVectorObj<Real> X,Y,Z;    

	for(size_t c = 0 ; c < positions.size() ; c++)
	{
		GFPhysVector3 position = positions[c];//(*iter);
		X.push_back(position.m_x);
		Y.push_back(position.m_y);
		Z.push_back(position.m_z);
	}

	Real SumX = 0.0f;
	for (int i = 0;i<m;i++)
	{
		SumX += X[i];
	}
	Real SumY = 0.0f;
	for (int i = 0;i<m;i++)
	{
		SumY += Y[i];
	}
	Real SumZ = 0.0f;
	for (int i = 0;i<m;i++)
	{
		SumZ += Z[i];
	}
	Real SumXY = 0.0f;
	for (int i = 0;i<m;i++)
	{
		SumXY += X[i]*Y[i];
	}
	Real SumXX = 0.0f;
	for (int i = 0;i<m;i++)
	{
		SumXX += X[i]*X[i];
	}
	Real SumYY = 0.0f;
	for (int i = 0;i<m;i++)
	{
		SumYY += Y[i]*Y[i];
	}
	Real SumXZ = 0.0f;
	for (int i = 0;i<m;i++)
	{
		SumXZ += X[i]*Z[i];
	}
	Real SumYZ = 0.0f;
	for (int i = 0;i<m;i++)
	{
		SumYZ += Y[i]*Z[i];
	}
	Real L11 = SumXX - SumX*SumX / (Real)m;
	Real L12 = SumXY - SumX*SumY / (Real)m;
	Real L21 = L12;
	Real L22 = SumYY - SumY*SumY / (Real)m;
	Real L1y = SumXZ - SumX*SumZ / (Real)m;
	Real L2y = SumYZ - SumY*SumZ / (Real)m;
	Real a;
	Real b;
	Real numer = L21*L12-L22*L11;
	if (fabsf(numer) < GP_EPSILON || fabsf(L12) < GP_EPSILON)
	{
		Ogre::LogManager::getSingleton().logMessage("regress failed!");        
		return false;
	}
	else
	{
		a = (L2y*L12 - L1y*L22)/numer;
		b = (L1y -L11*a)/L12;
	}
	normal = GFPhysVector3(a,b,-1);    
	return true;
}
//===============================================================================================
bool CalcPlaneNormalBySVD(const GFPhysAlignedVectorObj<GFPhysVector3> & positions,GFPhysVector3 & normal,GFPhysVector3 & com)
{
	int m = (int)positions.size();

	GFPhysVectorObj<Real> X,Y,Z;    

	for(size_t c = 0 ; c < positions.size() ; c++)
	{
		GFPhysVector3 position = positions[c];
		X.push_back(position.m_x);
		Y.push_back(position.m_y);
		Z.push_back(position.m_z);
	}

	Real SumX = 0.0f;
	for (int i = 0;i<m;i++)
	{
		SumX += X[i];
	}
	Real MeanX = SumX/(Real)m;
	Real SumY = 0.0f;
	for (int i = 0;i<m;i++)
	{
		SumY += Y[i];
	}
	Real MeanY = SumY/(Real)m;
	Real SumZ = 0.0f;
	for (int i = 0;i<m;i++)
	{
		SumZ += Z[i];
	}
	Real MeanZ = SumZ/(Real)m;

	com.m_x = MeanX;
	com.m_y = MeanY;
	com.m_z = MeanZ;

	for (int i = 0; i < m; i++)
	{
		X[i] -= MeanX;
		Y[i] -= MeanY;
		Z[i] -= MeanZ;
	}    

	Real element[3][3] = {{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f}};

	for (int i = 0; i < m; i++)
	{
		element[0][0] += X[i]*X[i];
		element[0][1] += X[i]*Y[i];
		element[0][2] += X[i]*Z[i];

		element[1][0] += Y[i]*X[i];
		element[1][1] += Y[i]*Y[i];
		element[1][2] += Y[i]*Z[i];

		element[2][0] += Z[i]*X[i];
		element[2][1] += Z[i]*Y[i];
		element[2][2] += Z[i]*Z[i];
	}
	Ogre::Matrix3 temp = Ogre::Matrix3(element);
	Real frac = 1/((Real)m -1);
	Ogre::Matrix3 cov = temp * frac;
	Ogre::Matrix3 rkL;
	Ogre::Vector3 rkS;
	Ogre::Matrix3 rkR;
	cov.SingularValueDecomposition(rkL,rkS,rkR);
	int indexmin = 0;
	for (int i = 1;i < 3; i++)
	{
		if (rkS[i] < rkS[indexmin])
		{
			indexmin = i;
		}        
	}
	//Ogre::Vector3 traget = rkR.GetColumn(indexmin);

	Real* traget = rkR[indexmin];  
	normal = GFPhysVector3(traget[0],traget[1],traget[2]);

	return true;
}


Real calculateZ( const Real & x,const Real & y, const GFPhysVector3 & n,/*normal vector */ const GFPhysVector3 & p/*point on plane */ )
{     
    if (fabsf(n.m_z)>GP_EPSILON )
    {
        return p.m_z -((x - p.m_x)*n.m_x + (y - p.m_y)*n.m_y)/ n.m_z;
    }
    else
        return 0.0f;
}