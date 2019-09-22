/**Author:zx**/
#include <string>
#include <OgreVector2.h>
#include <OgreVector3.h>
#include <Windows.h>

// wstring to string, zx
std::string ws2s(const std::wstring& ws)
{
	std::string curLocale = setlocale(LC_ALL, NULL);        // curLocale = "C";
	setlocale(LC_ALL, "chs");
	const wchar_t* _Source = ws.c_str();
	size_t _Dsize = 2 * ws.size() + 1;
	char *_Dest = new char[_Dsize];
	memset(_Dest,0,_Dsize);
	wcstombs(_Dest,_Source,_Dsize);
	std::string result = _Dest;
	delete []_Dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

// string to wstring, zx
std::wstring s2ws(const std::string& s)
{
	setlocale(LC_ALL, "chs"); 
	const char* _Source = s.c_str();
	size_t _Dsize = s.size() + 1;
	wchar_t *_Dest = new wchar_t[_Dsize];
	wmemset(_Dest, 0, _Dsize);
	mbstowcs(_Dest,_Source,_Dsize);
	std::wstring result = _Dest;
	delete []_Dest;
	setlocale(LC_ALL, "C");
	return result;
}

// judge whether spheres intersect, zx
bool SphereIntersect(Ogre::Vector3 v3Sphere1Center, float fSphere1Radius,
					 Ogre::Vector3 v3Sphere2Center, float fSphere2Radius)
{
	float fDist = sqrt( (v3Sphere1Center.x - v3Sphere2Center.x) * (v3Sphere1Center.x - v3Sphere2Center.x)
						+ (v3Sphere1Center.y - v3Sphere2Center.y) * (v3Sphere1Center.y - v3Sphere2Center.y)
						+ (v3Sphere1Center.z - v3Sphere2Center.z) * (v3Sphere1Center.z - v3Sphere2Center.z)
				);
	if (fSphere1Radius + fSphere2Radius > fDist)
	{
		return true;
	}
	return false;
}

// if point is in sphere's area, vector3's version, zx
bool PointInSphere(Ogre::Vector3 v3SphereCenter, float fSphereRadius, Ogre::Vector3 v3Pos)
{
	float fDist = sqrt( (v3SphereCenter.x - v3Pos.x) * (v3SphereCenter.x - v3Pos.x)
						+ (v3SphereCenter.y - v3Pos.y) * (v3SphereCenter.y - v3Pos.y)
						+ (v3SphereCenter.z - v3Pos.z) * (v3SphereCenter.z - v3Pos.z)
						);	
	if (fDist < fSphereRadius)
	{
		return true;
	}
	else return false;
}

// if point is in sphere's area, vector2's version, zx
bool PointInSphere(Ogre::Vector2 v2SphereCenter, float fSphereRadius, Ogre::Vector2 v2Pos)
{
	float fDist = sqrt( (v2SphereCenter.x - v2Pos.x) * (v2SphereCenter.x - v2Pos.x)
		+ (v2SphereCenter.y - v2Pos.y) * (v2SphereCenter.y - v2Pos.y)
		);	
	if (fDist < fSphereRadius)
	{
		return true;
	}
	else return false;
}

std::wstring TrimEnter(std::wstring strTarget)
{
	std::wstring strRet;
	for (int i = 0; i < strTarget.size(); i++)
	{
		if (strTarget.at(i) != TEXT('\n'))
		{
			strRet += strTarget.at(i);
		}
	}
	return strRet;
}

//O + Dt
Ogre::Vector3 CalIntersectPoint(const Ogre::Vector3 & dir, float t)
{
	Ogre::Vector3 vRet = dir * t;
	return vRet;
}

//(1 - u - v)V0 + uV1 + vV2
Ogre::Vector3 CalIntersectPoint(Ogre::Vector3 & v0, Ogre::Vector3 & v1, Ogre::Vector3 & v2, float u, float v)
{
	Ogre::Vector3 vRet = (1 - u - v) * v0 + u * v1 + v * v2;
	return vRet;
}

// zx
//	| t |                       | Q dot E2| 
//  | u | = (1 / | P dot E1|) * | P dot T |
//  | v |					    | Q dot D |
//
// Determine whether a ray intersect with a triangle 
// Parameters 
// orig: origin of the ray 
// dir: direction of the ray 
// v0, v1, v2: vertices of triangle 
// t(out): weight of the intersection for the ray 
// u(out), v(out): barycentric coordinate of intersection
bool IntersectTriangle(const Ogre::Vector3 & orig, const Ogre::Vector3 & dir, 
					   Ogre::Vector3 & v0, Ogre::Vector3 & v1, Ogre::Vector3 & v2,
					   float * t, float * u, float * v)
{
	// E1
	Ogre::Vector3 E1 = v1 - v0;

	// E2
	Ogre::Vector3 E2 = v2 - v0;

	// P
	Ogre::Vector3 P = dir.crossProduct(E2);

	// determinant
	float det = E1.dotProduct(P);

	// keep det > 0, modify T accordingly
	Ogre::Vector3 T;
	if (det > 0)
	{
		T = orig - v0;
	}
	else
	{
		T = v0 - orig;
		det *= -1;
	}

	// if determinant is near zero, ray lies in plane of the trialge
	if (det < 0.0001f)
	{
		return false;
	}

	// calculate u and make sure u <= 1
	*u = T.dotProduct(P);
	if (*u < 0.0f || *u > det)
	{
		return false;
	}

	// Q
	Ogre::Vector3 Q = T.crossProduct(E1);

	// calculate v and make sure u + v <= 1
	*v = dir.dotProduct(Q);
	if (*v < 0.0f || *u + *v > det)
	{
		return false;
	}

	// calculate t, scale parameters, ray intersects triangle
	*t = E2.dotProduct(Q);

	float fInvDet = 1.0f / det;
	*t *= fInvDet;
	*u *= fInvDet;
	*v *= fInvDet;

	return true;
}




