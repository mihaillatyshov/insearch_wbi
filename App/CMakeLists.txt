cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0025 NEW)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
    cmake_policy(SET CMP0093 NEW)
endif()

project(App VERSION 1.0)

set(SOURCES     
    src/main.cpp
    
    src/EditorLayer.cpp                                         src/EditorLayer.h    
    
    src/Python/PythonCommand.cpp                                src/Python/PythonCommand.h

    src/Project/Project.cpp                                     src/Project/Project.h
    src/Project/Catalog.cpp                                     src/Project/Catalog.h
    src/Project/CatalogCutPattern.cpp                           src/Project/CatalogCutPattern.h
    src/Project/Processing/GenImgsByCutPattern.cpp              src/Project/Processing/GenImgsByCutPattern.h
    src/Project/Processing/GenRawExcel.cpp                      src/Project/Processing/GenRawExcel.h

    src/ImGui/Tables/Table.cpp                                  src/ImGui/Tables/Table.h
    
    src/ImGui/Overlays/Overlay.cpp                              src/ImGui/Overlays/Overlay.h
    src/ImGui/Overlays/ScriptPopup.cpp                          src/ImGui/Overlays/ScriptPopup.h

    src/ImGui/Project/CreateNewProject.cpp                      src/ImGui/Project/CreateNewProject.h
    src/ImGui/Project/OpenProject.cpp                           src/ImGui/Project/OpenProject.h
    src/ImGui/Project/SetupProject.cpp                          src/ImGui/Project/SetupProject.h
    src/ImGui/Project/PageView/PageViewManager.cpp              src/ImGui/Project/PageView/PageViewManager.h
    src/ImGui/Project/PageView/IPageView.cpp                    src/ImGui/Project/PageView/IPageView.h
    src/ImGui/Project/PageView/IImgPageView.cpp                 src/ImGui/Project/PageView/IImgPageView.h
    src/ImGui/Project/PageView/RawImgPageView.cpp               src/ImGui/Project/PageView/RawImgPageView.h
    src/ImGui/Project/PageView/CutByPatternImgPageView.cpp      src/ImGui/Project/PageView/CutByPatternImgPageView.h

    src/ImGui/Constructions/SelectConstructionFromTree.cpp      src/ImGui/Constructions/SelectConstructionFromTree.hpp

    src/ImGui/Img/ScalableImg.cpp                               src/ImGui/Img/ScalableImg.h
    
    src/Serializer/Serializer.cpp                               src/Serializer/Serializer.h

    src/Managers/TextureManager.cpp                             src/Managers/TextureManager.h

    src/Excel/ExcelFileEditable.cpp                             src/Excel/ExcelFileEditable.h
    
    src/Utils/FileFormat.h
    src/Utils/FileSystemUtils.h
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

include(FetchContent)

FetchContent_Declare(
    xlnt
    GIT_REPOSITORY https://github.com/tfussell/xlnt.git
    GIT_TAG 297b331435d6dee09bf89c8a5ad974b01f18039b
)
FetchContent_MakeAvailable(xlnt)

# find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
# find_package(PythonLibs REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${PYTHON_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE Engine xlnt)
# target_link_libraries(${PROJECT_NAME} PRIVATE Engine ${PYTHON_LIBRARIES} ${PYTHON_DEBUG_LIBRARY})

# add_subdirectory(tests)

if(MSVC)
    set_target_properties(xlnt PROPERTIES FOLDER "deps")
endif()
