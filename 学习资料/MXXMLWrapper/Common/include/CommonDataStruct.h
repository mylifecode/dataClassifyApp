/**Author:zx**/
#pragma once
#include <string>
using namespace std;

#include "IMXDefine.h"

namespace cds
{
	typedef unsigned short VARTYPE;

	enum VARENUM
    {	
		SYVT_EMPTY  = 0,  //nothing
		SYVT_BOOL   = 1,  //True=1, False=0
		//VT_I1     = 2,  //signed char
		//VT_I2     = 3,  //2 byte signed int
		SYVT_I4     = 4,  //4 byte signed int
		//VT_UI1    = 5,  //unsigned char
		//VT_UI2    = 6,  //unsigned short
		//VT_UI4    = 7,  //unsigned long
		//VT_I8     = 8,  //signed 64-bit int
		//VT_UI8    = 9,  //unsigned 64-bit int
		SYVT_R4     = 10, //4 byte real
		//VT_R8     = 11, //8 byte real
		SYVT_STR    = 12, //string
    } ;

	struct Variant
	{
		//struct __Variant
		//{
			VARTYPE vt;
			union
			{
				long long llVal;
				long lVal;
				short iVal;
				float fltVal;
				double dblVal;
				bool bVal;
				unsigned short uiVal;
				unsigned long ulVal;
				unsigned long long ullVal;
			};
			
		//};
		Ogre::String strVal;
		Variant()
		{
			this->vt = SYVT_EMPTY;
			this->strVal = Ogre::String("");
		}
		Variant(const Variant & value)
		{
			this->vt = value.vt;
			switch (vt)
			{
			case SYVT_EMPTY:
				break;
			case SYVT_BOOL:
				this->bVal = value.bVal;
				break;
			//case VT_I1:
			//	break;
			//case VT_I2:
				//break;
			case SYVT_I4:
				this->lVal = value.lVal;
				break;
			//case VT_UI1:
			//	break;
			//case VT_UI2:
			//	break;
			///case VT_UI4:
			//	break;
			//case VT_I8:
			//	this->llVal = value.llVal;
			//	break;
			//case VT_UI8:
			//	break;
			case SYVT_R4:
				this->fltVal = value.fltVal;
				break;
			//case VT_R8:
			//	this->dblVal = value.dblVal;
				//break;
			case SYVT_STR:
				this->strVal = value.strVal;
				break;
			}
		}
	};
	
	
}
