////////////////////////////////////////////////
//     FileName:    singlet.h
//     Author:      Liulei
//     Date:        2011-9-20 15:05
//     Description: 简单易用的单件实现基类
////////////////////////////////////////////////

#ifndef _SINGLET_H_
#define _SINGLET_H_

#include <memory>
//#include "BaseCode.h"

template<class T>
class CSingleT
{
public:

    static T * Instance()
    {
        if ( !ms_pObject )
        {
            ms_pObject = new T;
        }
        return ms_pObject;
    }

    static void Create()
    {
        if ( !ms_pObject )
        {
            ms_pObject = new T;
        }
    }

    static void Destroy()
    {
        if ( ms_pObject )
        {
            delete ms_pObject;
            ms_pObject = NULL;
        }
    }

    static T* Get()
    {
        return ms_pObject;
    }

    static void Reset()
    {
        Destroy();
        Create();
    }

protected:
    static T*	ms_pObject;
};

template<class T> 
T* CSingleT<T>::ms_pObject = NULL;

#endif