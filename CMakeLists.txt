cmake_minimum_required(VERSION 3.17)

set(PROJECT "Flow4DViewerPlugin")

PROJECT(${PROJECT})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /EHsc /MP /permissive- /Zc:__cplusplus")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG /NODEFAULTLIB:LIBCMT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)


# vtk Dir build needs to be in the same mode as MV is to be run. (for release MV release vtk is needed)
#set(VTK_DIR $ENV{VTK_DIR})
find_package(VTK CONFIG REQUIRED)

find_package(OpenGL REQUIRED)
find_package(Qt6 6.3.1 COMPONENTS Widgets WebEngineWidgets Xml OpenGL OpenGLWidgets REQUIRED)
find_package(OpenMP)

# Check if the directory to the ManiVault installation has been provided
if(NOT DEFINED MV_INSTALL_DIR)
    set(MV_INSTALL_DIR "" CACHE PATH "Directory where ManiVault is installed")
    message(FATAL_ERROR "Please set MV_INSTALL_DIR to the directory where ManiVault is installed")
endif()
file(TO_CMAKE_PATH ${MV_INSTALL_DIR} MV_INSTALL_DIR)



set(PLUGIN
	src/Common.h
	src/Flow4DViewerPlugin.h
	src/Flow4DViewerPlugin.cpp
)

file(GLOB ACTIONS src/*Action.*)
file(GLOB WIDGETS src/*Widget.*)
file(GLOB JSON src/*.json)


set(SOURCES ${PLUGIN} ${WIDGETS} ${ACTIONS})
set(AUX ${JSON})


source_group(Plugin FILES ${SOURCES})
source_group(Actions FILES ${ACTIONS})
source_group(Widget FILES ${WIDGETS})


add_library(${PROJECT} SHARED ${SOURCES} ${AUX})

qt_wrap_cpp(FLOW4DVIEWERPLUGIN_MOC ${PLUGIN_MOC_HEADERS} TARGET ${PROJECT})
target_sources(${PROJECT} PRIVATE ${FLOW4DVIEWERPLUGIN_MOC})

target_include_directories(${PROJECT} PRIVATE "${MV_INSTALL_DIR}/$<CONFIGURATION>/include/")
target_include_directories(${PROJECT} PRIVATE src)

target_compile_features(${PROJECT} PRIVATE cxx_std_17)

target_link_libraries(${PROJECT} PRIVATE ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT} PRIVATE Qt6::Widgets)
target_link_libraries(${PROJECT} PRIVATE Qt6::WebEngineWidgets)
target_link_libraries(${PROJECT} PRIVATE Qt6::OpenGL)
target_link_libraries(${PROJECT} PRIVATE Qt6::OpenGLWidgets)
target_link_libraries(${PROJECT} PRIVATE ${VTK_LIBRARIES})
vtk_module_autoinit(TARGETS ${PROJECT} MODULES ${VTK_LIBRARIES})

set(MV_LINK_PATH "${MV_INSTALL_DIR}/$<CONFIGURATION>/lib")
set(PLUGIN_LINK_PATH "${MV_INSTALL_DIR}/$<CONFIGURATION>/$<IF:$<CXX_COMPILER_ID:MSVC>,lib,Plugins>")
set(MV_LINK_SUFFIX $<IF:$<CXX_COMPILER_ID:MSVC>,${CMAKE_LINK_LIBRARY_SUFFIX},${CMAKE_SHARED_LIBRARY_SUFFIX}>)

set(MV_LINK_LIBRARY "${MV_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}MV_Public${MV_LINK_SUFFIX}")
set(POINTDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${MV_LINK_SUFFIX}")
set(IMAGEDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}ImageData${MV_LINK_SUFFIX}") 
set(CLUSTERDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}ClusterData${MV_LINK_SUFFIX}") 

target_link_libraries(${PROJECT} PRIVATE "${MV_LINK_LIBRARY}")
target_link_libraries(${PROJECT} PRIVATE "${POINTDATA_LINK_LIBRARY}")

install(TARGETS ${PROJECT}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
)

add_custom_command(TARGET ${PROJECT} POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        --install ${CMAKE_CURRENT_BINARY_DIR}
        --config $<CONFIGURATION>
        --prefix ${MV_INSTALL_DIR}/$<CONFIGURATION>
)

# Automatically set the debug environment (command + working directory) for MSVC
if(MSVC)
	set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${MV_INSTALL_DIR}/debug,${MV_INSTALL_DIR}/release>)
    set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,"${MV_INSTALL_DIR}/debug/ManiVault Studio.exe","${MV_INSTALL_DIR}/release/ManiVault Studio.exe">)
endif()
