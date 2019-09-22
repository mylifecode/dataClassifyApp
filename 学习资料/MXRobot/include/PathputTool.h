#ifndef _PATHPUTTOOL_
#define _PATHPUTTOOL_
#include <map>
#include <vector>
#include "ogre.h"
#include "NewTrain/VeinConnectObject.h"

class CBasicTraining;
class VeinConnectObject;
enum EditType
{
    ET_None,
    ET_CreatePath,
    ET_AddPathNode,
};

class EditPointInOrgan
{
public:
    EditPointInOrgan()
    {
        m_Organ = 0;
        //m_suspenddist = 0;
        m_Face = 0;
    }
    float m_weights[3];
    GFPhysSoftBodyFace * m_Face;
    Ogre::Vector3 m_position;

    //Ogre::Vector3 m_susptangent;
    //Ogre::Vector3 m_suspennorm;

    //Ogre::Vector3 m_suspendir;
    //float m_suspenddist;
    MisMedicOrgan_Ordinary * m_Organ;
};

class EditClusterInOrgan
{
public:
    // static int s_gpairid;
    EditClusterInOrgan()
    {
        m_pairobjA = m_pairobjB = 0;
        m_finisha = m_finishb = false;
        m_IsInEditState = false;
        //m_pairid = s_gpairid;
        //s_gpairid++;
    }
    void Reset()
    {
        m_pairobjA = m_pairobjB = 0;
        m_finisha = m_finishb = false;
        m_IsInEditState = false;
        m_pairpointA.clear();
        m_pairpointB.clear();
    }
    std::vector<EditPointInOrgan> m_pairpointA;
    std::vector<EditPointInOrgan> m_pairpointB;

    MisMedicOrgan_Ordinary * m_pairobjA;
    MisMedicOrgan_Ordinary * m_pairobjB;

    bool m_finisha;
    bool m_finishb;

    //int    m_pairid;

    bool m_IsInEditState;
};


class PathPutToolEventListener
{
public:
    PathPutToolEventListener(){}
    ~PathPutToolEventListener(){}
    virtual void OnConnectLoaded() = 0;

    virtual void OnConnectPairChanged() = 0;
};

class PathPutTool : public OgreWidgetEventListener
{
public:
    //bool m_ingroupeditmode;

    static PathPutTool * GetCurrentTool();

    PathPutTool();

    ~PathPutTool();

    void Construct(Ogre::SceneManager * scenemgr, CBasicTraining * basictrain);

    void AddConnectionFromVeinObject(VeinConnectObject * veinobj);

    void SetCurrentSelectedConnect(int selectedid);

    void RemoveSelectedCluster();

    void RefreshClusterLists();

    //@overridden listener
    void OnMousePressed(char button, int x, int y);//(float x , float y, Ogre::Camera * camera ,  CBasicTraining * pTraining);

    //@overridden listener
    void OnMouseMoved(char button, int x, int y);//(float x , float y , int buttonpress, Ogre::Camera * camera ,  CBasicTraining * pTraining) ;

    //@override listener
    void OnMouseReleased(char button, int x, int y);//(float x , float y , int buttonpress, Ogre::Camera * camera ,  CBasicTraining * pTraining);

    void OnKeyPress(int whichButton);
    void OnKeyRelease(int whichButton);
    void checkClusterInEdit();

    EditPointInOrgan PickNearestEditPointInScene(Ogre::Camera * camera, float mousex, float mousey);

    EditPointInOrgan PickEditPointOnObject(Ogre::Camera * camera, float mousex, float mousey, MisMedicOrgan_Ordinary* dynobj);

    void  ChangeSelectedPairSuspendDist(float heighta, float heightb);

    void  ChangeSelectedPairSuspendDistInPartA(float heighta);

    void  ChangeSelectedPairSuspendDistInPartB(float heightb);

    void  Reset();

    void ExportToObjFile(char * objfilename, char * mapfilename);

    void mapobjfiletoConnection(char * filemap, char * fileobj, char * outputfilename);

    void SetPathToEdit(int pathid);

    void serialize(Ogre::String filename);

    void deserialize(Ogre::String filename, CBasicTraining * basictrain);

    void AutoDetectConnectAdhersion(Ogre::String filename, DynObjMap & dynmap);

    void AutoGenAdhersion();
    void AutoGenAdhersion1();
    void AutoGenAdhersion2();

    void update(float dt, Ogre::Camera * camera);

    void  AddEventListener(PathPutToolEventListener * listener);

    void  RemoveEventListener(PathPutToolEventListener * listener);

    void HideSelectedOgran();
public:
    Ogre::ManualObject * m_renderobj;


    EditType m_edittype;

    int m_operatepathid;

    Ogre::Vector2 m_lastputmouth;

    int m_puttedcount;

    std::vector<int>   m_ConnectPairs;

    EditClusterInOrgan m_ClusterInEdit;
    std::vector<EditPointInOrgan> m_vecPointPreview;


    std::vector<PathPutToolEventListener*>m_editorlistener;
    int m_selectedconnect;

    static float s_putdensity;

    bool m_hiddenselected;
    bool m_hiddenunselected;
    bool m_IsCtrlPressed;

    VeinConnectObject * m_EditedObject;

    CBasicTraining * m_hosttrain;

    std::set<int> m_SelectedClusterId;
    bool m_SendDebuginfo;
protected:
    void DrawOneEditPair(const EditClusterInOrgan & editpoint, Ogre::ColourValue boxcolor, int & startoffset);

    void DrawOneSelectedCluster(const VeinConnectCluster & cluster, Ogre::ColourValue boxcolor, int & startoffset);

    void DrawPreviewPoint(const std::vector<EditPointInOrgan> & vecEditPoint, Ogre::ColourValue boxcolor, int & startoffset);

    int  drawonepoint(Ogre::Vector3 position, int startoffset, Ogre::ColourValue color);
    int  drawoneline(Ogre::Vector3 position[2], int startoffset, Ogre::ColourValue color);
    int  GetRGBFromAreaTesture(float cx, float cy, uint8& red, uint8& green, uint8& blue);
    Ogre::TexturePtr m_area;

    int m_areaHeight;
    int m_areaWidth;

    Ogre::uint* m_pDest;
};
#endif