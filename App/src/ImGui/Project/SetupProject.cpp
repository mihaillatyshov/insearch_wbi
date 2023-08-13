#include "SetupProject.h"

#include <filesystem>
#include <fstream>
#include <thread>

#include <imgui.h>

#include "Engine/Utils/ConsoleLog.h"
#include "Engine/Utils/FileDialogs.h"
#include "Engine/Utils/utf8.h"

#include "Python/PythonCommand.h"

namespace LM
{

    const FileDialogs::Filter kFileDialogsFilter { "InSearch Project (*.pdf)", "*.pdf" };

    void SetupProject::Draw(Ref<Project> _Project)
    {
        if (!m_IsOpen || _Project == Ref<Project>())
        {
            return;
        }

        if (ImGui::Begin(U8("Настройка проекта")))
        {
            ImGui::Text(U8("Файл проекта: %s"), _Project->GetFileName().c_str());
            ImGui::Text(U8("Папка ассетов: %s"), _Project->GetAssetsPath().c_str());
            ImGui::Text(U8("Оригинал каталога: %s"), _Project->GetCatalogBaseFilename().c_str());
            if (ImGui::Button(U8("Изменить оригинал каталога")))
            {
                if (std::string filename = FileDialogs::OpenFile(kFileDialogsFilter); filename != std::string())
                {
                    try
                    {
                        std::filesystem::remove(_Project->GetCatalogFilename());
                        if (std::filesystem::copy_file(filename, _Project->GetCatalogFilename()))
                        {
                            _Project->SetCatalogBaseFilename(filename);
                        }
                    }
                    catch (const std::filesystem::filesystem_error& err)
                    {
                        LOGE("File copy error (", filename, "),    ", "filesystem error: ", err.what());
                    }
                }
            }

            if (ImGui::Button("Test Python"))
            {
                m_IsPythonRuning = true;
                m_PythonBuffer = "";
                ImGui::OpenPopup("Test Python");

                std::thread thread(
                    [&](std::string _CatalogFileName, std::string _RawImgPath, int _ImgQuality, bool _SplitPages) {
                        std::string script = std::string(RES_FOLDER) + "assets/scripts/prepare_img_raw.py";

                        PythonCommand pythonCommand(script);
                        pythonCommand.AddArg(_CatalogFileName);
                        pythonCommand.AddArg(_RawImgPath);
                        pythonCommand.AddArg(_ImgQuality);
                        pythonCommand.AddArg(_SplitPages);

                        LOGI("Start Exec");

                        pythonCommand.Execute([&](const char* buffer) {
                            std::lock_guard lock(m_PythonBufferMtx);
                            m_PythonBuffer = buffer;
                        });

                        LOGI("End Exec");

                        // Py_Initialize();

                        // PyObject *pName, *pModule, *pFunc, *pArgs, *pValue;
                        // PyObject* sys_path = PySys_GetObject("path");
                        // PyList_Append(sys_path, PyUnicode_FromString(scriptsPath.c_str()));

                        //// pName = PyUnicode_FromString("prepare_img_raw.py");
                        // pModule = PyImport_ImportModule("prepare_img_raw.py");

                        // if (pModule != NULL)
                        //{

                        //    Py_DECREF(pName);
                        //    pFunc = PyObject_GetAttrString(pModule, (char*)"prepare_img_raw");
                        //    std::string savePath = std::string(RES_FOLDER) + "assets/test/";
                        //    pArgs = PyTuple_Pack(3, PyUnicode_FromString(savePath.c_str()), 1, true);
                        //    pValue = PyObject_CallObject(pFunc, pArgs);
                        //    auto result = _PyUnicode_AsString(pValue);
                        //    std::cout << result << std::endl;
                        //}
                        // else
                        //{
                        //    PyErr_Print();
                        //    std::cout << "Failed to load module" << std::endl;
                        //}

                        //// Run a simple file
                        ///*FILE* PS criptFile = fopen(scriptsPath.c_str(), "r");
                        // if (PScriptFile)
                        // {
                        //     PyRun_SimpleFile(PScriptFile, "test.py");
                        //     fclose(PScriptFile);
                        // }*/

                        // std::cout << "Py_FinalizeEx: " << Py_FinalizeEx() << std::endl;

                        m_IsPythonRuning = false;
                    },
                    _Project->GetCatalogFilename(), _Project->GetRawImgPath(), _Project->GetImgQuality(),
                    _Project->GetCatalogSplitPages());
                thread.detach();
            }

            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

            if (ImGui::BeginPopupModal("Test Python", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text(U8("Работает скрипт преобразования pdf в картинки"));
                ImGui::Text(U8("Это может занять несколько минут"));
                ImGui::Text(U8("После его завершения можно закрыть это окно"));
                ImGui::Separator();

                DrawPythonBuffer();

                ImGui::Text("\n");
                ImGui::Separator();

                ImGui::BeginDisabled(m_IsPythonRuning);
                if (ImGui::Button(U8("Закрыть"), ImVec2(120, 0)))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndDisabled();

                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }

    void SetupProject::DrawPythonBuffer()
    {
        std::lock_guard lock(m_PythonBufferMtx);
        ImGui::Text(m_PythonBuffer.c_str());
    }

}    // namespace LM
