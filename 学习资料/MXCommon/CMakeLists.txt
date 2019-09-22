SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4275 /wd4193")

add_definitions("-DTIXML_USE_STL")
add_definitions("-DOGREMAX_SHARED_LIBRARY_EXPORT")
add_definitions("-DOGREMAX_VIEWER_NO_LUA")
add_definitions(-DUNICODE -D_UNICODE)


############################################################
# Ogre
############################################################
INCLUDE_DIRECTORIES(${OGRE_INCLUDE_DIRECTORIES})
LINK_DIRECTORIES(${OGRE_LIBRARY_DIRECTORIES})


############################################################
# BASS
############################################################
INCLUDE_DIRECTORIES(${BASS_INCLUDE_DIRECTORIES})
LINK_DIRECTORIES(${BASS_LIBARY_DIRECTORIES})
INSTALL(FILES ${BASS_RUNTIME_FILES} DESTINATION .)

############################################################
# qt
############################################################
set(QT_QMAKE_EXECUTABLE ${QT_ROOT}/bin/qmake)
INCLUDE_DIRECTORIES(${QT_ALL_INCLUDE_DIRECTORIES})
LINK_DIRECTORIES(${QT_ALL_LIBRARY_DIRECTORIES})
add_definitions(${QT_DEFINITIONS})
FILE(GLOB QT_RUNTIME ${QT_ROOT}/bin/*.dll)
INSTALL(FILES ${QT_RUNTIME} DESTINATION .)
INSTALL(DIRECTORY ${QT_ROOT}/plugins/ DESTINATION ./plugins)

INCLUDE_DIRECTORIES("./")
INCLUDE_DIRECTORIES("../Include/")
INCLUDE_DIRECTORIES("../MXXMLWrapper/Common/include/")
INCLUDE_DIRECTORIES("include/")

FILE(GLOB HEADERS "./*.h" "include/*.h" "include/*.hpp")
FILE(GLOB SOURCES "./*.cpp" "src/*.cpp" "include/*.cpp")

LINK_DIRECTORIES("${PROJECT_SOURCE_DIR}/lib/")
add_library(SYCommon ${HEADERS} ${SOURCES})

set(CMAKE_LINK_LIBRARY_SUFFIX "")
TARGET_LINK_LIBRARIES(SYCommon SYXMLWrapper)
TARGET_LINK_LIBRARIES(SYCommon Strmiids.lib.lib ${BASS_LIBRARIES})
TARGET_LINK_LIBRARIES(SYCommon ${OGRE_LIBRARIES} ${QT_ALL_LIBARIES})



