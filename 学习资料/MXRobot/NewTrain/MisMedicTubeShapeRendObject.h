#ifndef _MISMEDICTUBESHAPERENDOBJECT_
#define  _MISMEDICTUBESHAPERENDOBJECT_
#include <Ogre.h>
#define TUBERENDSHAPECAPVERTNUM 6

class MisMedicTubeShapeRendObject
{
public:
	class RopeRendSection
	{
	public:
		RopeRendSection(Ogre::Vector3 center , float radius)
		{
			m_center = center;
			m_radius = radius;
		}
		Ogre::Vector3 m_capVertex[TUBERENDSHAPECAPVERTNUM];
		float m_radius;
		Ogre::Vector3 m_center;
	};

	MisMedicTubeShapeRendObject();
	
	~MisMedicTubeShapeRendObject();

	void CreateRendPart(const Ogre::String & name , Ogre::SceneManager * scenemgr);

	void UpdateRendSegment(std::vector<Ogre::Vector3> & centerPosition, float RendRadius, std::string materialName = "", bool isLoop = false);

	void UpdateSegmentTagColor(std::vector<Ogre::Vector3> & centerPosition , 
							   std::vector<bool> & bTagged,
							   std::vector<Ogre::ColourValue> & TagColor,
							   float RendRadius);

	void UpdateRendSegments(std::vector<std::vector<Ogre::Vector3>> & centerPosition , float RendRadius , bool isLoop = false);

protected:

	void UpdateRendPart(std::vector<Ogre::Vector3> & centerPosition , 
		               float RendRadius  , 
					   int type,
					   int subtype);

	Ogre::SceneNode * m_SceneNode;

	Ogre::ManualObject * m_Object;

	std::vector<RopeRendSection> m_RopeRendSect;

};
#endif