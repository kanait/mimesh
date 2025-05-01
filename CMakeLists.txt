if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "DO NOT BUILD in-tree.")
endif()

cmake_minimum_required ( VERSION 3.11 )
project( mimesh )

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)

SET(CMAKE_C_COMPILER_WORKS 1 CACHE INTERNAL "")
SET(CMAKE_CXX_COMPILER_WORKS 1 CACHE INTERNAL "")

if(UNIX AND NOT APPLE)
        set(OpenGL_GL_PREFERENCE GLVND)
endif()

add_executable( ${PROJECT_NAME}
                render-1.0/envDep.h
                render-1.0/mydef.h
                render-1.0/myGL.hxx
                render-1.0/MR_Mesh.cxx
                render-1.0/MR_Mesh.h
                render-1.0/MR_Patch.cxx
                render-1.0/MR_Patch.h
                render-1.0/MR_Vec.cxx
                render-1.0/MR_Vec.h
                render-1.0/rend.cxx
                render-1.0/tga.c
                render-1.0/tga.h
                render-1.0/Vec3f.cxx
                render-1.0/Vec3f.h
)

target_compile_options( ${PROJECT_NAME} PUBLIC -O3 -Wall)
target_compile_features( ${PROJECT_NAME} PUBLIC cxx_std_14 )

target_include_directories( ${PROJECT_NAME}
                            PRIVATE
                            ${OPENGL_INCLUDE_DIR}
                            ${GLEW_INCLUDE_DIR}
                            ${GLUT_INCLUDE_DIR}
)

target_link_libraries( ${PROJECT_NAME}
                       PRIVATE
                       ${OPENGL_LIBRARIES}
                       GLEW::GLEW
                       GLUT::GLUT
)

# Set StartUp Project (Option)
set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "mimesh" )

