SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4275 /wd4193")

# qt
set(QT_QMAKE_EXECUTABLE ${QT_ROOT}/bin/qmake)
INCLUDE_DIRECTORIES(${QT_ALL_INCLUDE_DIRECTORIES})
FILE(GLOB MXRobot_HEADERS "Include/lwInception.h" "Include/lwWidget.h" "Include/ogrewidget.h")
QT5_WRAP_CPP(MXRobot_HEADERS_MOC ${MXRobot_HEADERS})

FILE(GLOB HEADERS "include/*.h" "Common/*.h" "ResourceManager/*.h")
FILE(GLOB SOURCES "src/*.cpp" "Common/*.cpp" "ResourceManager/*.cpp")
FILE(GLOB SOURCES_NewTrain "NewTrain/*")
FILE(GLOB SOURCES_Instruments "Instruments/*")
FILE(GLOB SOURCES_EditorTool "EditorTool/*")
FILE(GLOB SOURCES_Event "Event/*")
SOURCE_GROUP(NewTrain FILES ${SOURCES_NewTrain})
SOURCE_GROUP(Instruments FILES ${SOURCES_Instruments})
SOURCE_GROUP(EditorTool FILES ${SOURCES_EditorTool})
SOURCE_GROUP(Event FILES ${SOURCES_Event})


add_definitions("-DTIXML_USE_STL")
add_definitions("-DOGREMAX_SHARED_LIBRARY_IMPORT")
add_definitions(-DUNICODE -D_UNICODE)


add_library(MXRobot  ${SOURCES_Event} ${HEADERS} ${SOURCES} ${MXRobot_HEADERS_MOC} ${SOURCES_NewTrain} ${SOURCES_Instruments} ${SOURCES_EditorTool})
target_link_libraries(MXRobot SYCommon)

INCLUDE_DIRECTORIES(".")
INCLUDE_DIRECTORIES("include")
INCLUDE_DIRECTORIES("NewTrain")
INCLUDE_DIRECTORIES("Common")
INCLUDE_DIRECTORIES("ResourceManager")
INCLUDE_DIRECTORIES("EditorTool")
INCLUDE_DIRECTORIES("Event")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/Include/")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/MXCommon/include/")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/MXXMLWrapper/Common/include/")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/MXXMLWrapper/Robot/include/")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/HardwareSystem/")
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/PhantomLib/")

############################################################
# GPSDK
############################################################
INCLUDE_DIRECTORIES("${GPSDK_DIR}")
INCLUDE_DIRECTORIES("${GPSDK_DIR}/pthread/include/")
LINK_DIRECTORIES("${GPSDK_DIR}")

############################################################
# OpenHaptics
############################################################
INCLUDE_DIRECTORIES("${OPENHAPTICS_DIR}/include/")
INCLUDE_DIRECTORIES("${OPENHAPTICS_DIR}/utilities/include/")
LINK_DIRECTORIES("${OPENHAPTICS_DIR}/lib/")
LINK_DIRECTORIES("${OPENHAPTICS_DIR}/utilities/lib/")

############################################################
# BASS
############################################################
INCLUDE_DIRECTORIES(${BASS_INCLUDE_DIRECTORIES})
LINK_DIRECTORIES(${BASS_LIBARY_DIRECTORIES})
INSTALL(FILES ${BASS_RUNTIME_FILES} DESTINATION .)

############################################################
# Ogre
############################################################
INCLUDE_DIRECTORIES(${OGRE_INCLUDE_DIRECTORIES})
LINK_DIRECTORIES(${OGRE_LIBRARY_DIRECTORIES})
