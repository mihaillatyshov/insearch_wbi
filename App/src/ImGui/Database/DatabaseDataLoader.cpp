#include "DatabaseDataLoader.hpp"

#include "Database/DatabaseData.hpp"
#include "ImGui/Configs/SharedConnectionConfigSetup.hpp"
#include "ImGui/Overlays/Overlay.h"
#include "ImGui/Overlays/ScriptPopup.h"
#include "Python/PythonCommand.h"

#include <imgui.h>

namespace LM
{

    void DatabaseDataLoader::Draw()
    {
        if (!m_IsOpen)
        {
            return;
        }

        if (ImGui::Begin("Данные из БД", &m_IsOpen))
        {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Выбранная конфигурация подключения: %s",
                               SharedConnectionConfigSetup::GetCurrentConfigName().data());

            ImGui::Separator();

            if (ImGui::Button("Загрузить данные конструкций и полей"))
            {
                LoadInsearchEditorData();
            }
            ImGui::SetItemTooltip("Содержимое папки %s будет удалено и заполнено заново.",
                                  DatabaseData::kConstructionsFolder.string().c_str());

            if (ImGui::Button("Перечитать данные из файлов"))
            {
                DatabaseData::Get().Reload();
            }
            ImGui::SetItemTooltip("Перечитать файлы из папки %s без обращения к БД.",
                                  DatabaseData::kConstructionsFolder.string().c_str());
        }
        ImGui::End();
    }

    void DatabaseDataLoader::LoadInsearchEditorData()
    {
        const SharedConnectionConfig* config = SharedConnectionConfigSetup::GetCurrentConfig();
        if (!config)
        {
            Overlay::Get()->Start("Не выбрана конфигурация подключения к БД");
            return;
        }

        PythonCommand pythonCommand(kInsearchEditorDataScriptPath.string());
        pythonCommand.AddPathArg(DatabaseData::kConstructionsFolder, "--save_path");
        pythonCommand.AddArg(config->DbHost, "--db_host");
        pythonCommand.AddArg(config->DbPort, "--db_port");
        pythonCommand.AddArg(config->DbUser, "--db_user");
        pythonCommand.AddArg(config->DbPassword, "--db_password");

        ScriptPopup::Get()->AddToQueue(pythonCommand,
                                       { "Загрузка данных конструкций и полей из БД",
                                         []() {
                                             ImGui::Text("Работает скрипт загрузки данных конструкций и полей из БД");
                                             ImGui::Text("Это может занять несколько минут");
                                             ImGui::Text("После его завершения можно закрыть это окно");
                                         },
                                         // Папка очищается в начале скрипта, поэтому перечитываем и при ошибке
                                         [](int) { DatabaseData::Get().Reload(); } });
    }

}    // namespace LM
