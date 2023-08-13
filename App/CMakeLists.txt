cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0025 NEW)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
    cmake_policy(SET CMP0093 NEW)
endif()

project(App VERSION 1.0)

set(SOURCES     src/main.cpp
    
    src/EditorLayer.cpp                     src/EditorLayer.h    
    
    src/Python/PythonCommand.cpp            src/Python/PythonCommand.h

    src/Project/Project.cpp                 src/Project/Project.h
    src/Project/Catalog.cpp                 src/Project/Catalog.h

    src/ImGui/Tables/Table.cpp              src/ImGui/Tables/Table.h
    src/ImGui/Overlays/Overlay.cpp          src/ImGui/Overlays/Overlay.h
    src/ImGui/Project/SetupProject.cpp      src/ImGui/Project/SetupProject.h
    
    src/Serializer/Serializer.cpp           src/Serializer/Serializer.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(PythonLibs REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${PYTHON_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE Engine ${PYTHON_LIBRARIES} ${PYTHON_DEBUG_LIBRARY})
# add_subdirectory(tests)

# if(MSVC)
#     set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "App")
# endif()
