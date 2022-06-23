#include "Game/Game.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/KerningFont.hpp"
#include "Engine/Core/WebP.hpp"

#include "Engine/Input/InputSystem.hpp"

#include "Engine/Math/Disc2.hpp"

#include "Engine/Renderer/AnimatedSprite.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Material.hpp"

#include "Engine/Services/ServiceLocator.hpp"
#include "Engine/Services/IAppService.hpp"
#include "Engine/Services/IRendererService.hpp"
#include "Engine/Services/IFileLoggerService.hpp"

#include "Engine/UI/UISystem.hpp"

#include "Game/GameCommon.hpp"
#include "Game/GameConfig.hpp"

#include <algorithm>
#include <chrono>
#include <execution>
#include <memory>
#include <numeric>
#include <string>

std::vector<std::unique_ptr<Texture>> frames{};
std::vector<TimeUtils::FPMilliseconds> frame_times{};

std::unique_ptr<FileUtils::WebP> sprite{};

Game::~Game() noexcept {
    for(auto& f : frames) {
        f.reset();
    }
    frames.clear();
    frames.shrink_to_fit();
    sprite.reset();
}



void Game::Initialize() noexcept {
    //const auto n = MathUtils::nCr(11, 5);
    //std::cout << "nCr: " << n << '\n';

    g_theRenderer->RegisterFontsFromFolder(FileUtils::GetKnownFolderPath(FileUtils::KnownPathID::GameFonts));
    g_theRenderer->RegisterMaterialsFromFolder(FileUtils::GetKnownFolderPath(FileUtils::KnownPathID::GameMaterials));

    _cameraController = OrthographicCameraController();
    _cameraController.SetPosition(Vector2::Zero);

    const auto image_src = std::filesystem::path{ "Data/Images/tifa.webp" };
    sprite = std::make_unique<FileUtils::WebP>(std::filesystem::path{ image_src });

}

void Game::BeginFrame() noexcept {
    /* DO NOTHING */
}

void Game::Update(TimeUtils::FPSeconds deltaSeconds) noexcept {
    g_theRenderer->UpdateGameTime(deltaSeconds);

    HandleDebugInput(deltaSeconds);
    HandlePlayerInput(deltaSeconds);

    _ui_camera2D.Update(deltaSeconds);
    _cameraController.Update(deltaSeconds);

    sprite->Update(deltaSeconds);

    {
        if(ImGui::Begin("Debug")) {
            static int min_begin_frame{ 0u };
            static int max_begin_frame{ static_cast<int>(sprite->GetFrameCount() - 1u) };
            static int begin_frame{ 0u };
            ImGui::SliderInt("###Begin Frame", &begin_frame, min_begin_frame, max_begin_frame);
            ImGui::SameLine();
            if(ImGui::Button("Set Begin Frame")) {
                min_begin_frame = begin_frame;
                sprite->SetBeginFrame(min_begin_frame);
            }
            ImGui::SameLine();
            if(ImGui::Button("Reset##BeginFrame")) {
                min_begin_frame = 0;
                begin_frame = 0;
                sprite->SetBeginFrame(0);
            }
            static int end_frame{ static_cast<int>(sprite->GetFrameCount() - 1u) };
            ImGui::SliderInt("###End Frame", &end_frame, min_begin_frame, max_begin_frame);
            ImGui::SameLine();
            if(ImGui::Button("Set End Frame")) {
                max_begin_frame = end_frame;
                sprite->SetEndFrame(max_begin_frame);
            }
            ImGui::SameLine();
            if(ImGui::Button("Reset##EndFrame")) {
                max_begin_frame = static_cast<int>(sprite->GetFrameCount() - 1u);
                end_frame = static_cast<int>(sprite->GetFrameCount() - 1u);
                sprite->SetEndFrame(sprite->GetFrameCount() - 1u);
            }
        }
        ImGui::End();
    }

}

void Game::Render() const noexcept {
    g_theRenderer->BeginRenderToBackbuffer();

    //World View
    sprite->Render();

    g_theRenderer->SetModelMatrix();
    g_theRenderer->SetMaterial(g_theRenderer->GetMaterial("__2D"));
    // HUD View
    {
        const auto ui_view_height = static_cast<float>(GetSettings().GetWindowHeight());
        const auto ui_view_width = ui_view_height * _ui_camera2D.GetAspectRatio();
        const auto ui_view_extents = Vector2{ ui_view_width, ui_view_height };
        const auto ui_view_half_extents = ui_view_extents * 0.5f;
        const auto ui_cam_pos = Vector2::Zero;
        g_theRenderer->BeginHUDRender(_ui_camera2D, ui_cam_pos, ui_view_height);
    }
}

void Game::EndFrame() noexcept {
    static bool max_elapsed_time_needs_update = false;
    if(max_elapsed_time_needs_update) {
        max_elapsed_time_needs_update = false;
        //max_elapsed_time = std::reduce(std::execution::par_unseq, std::cbegin(frame_times) + begin_frame, std::cend(frame_times) - (max_frames - end_frame), TimeUtils::FPSeconds::zero());
    }
}

const GameSettings& Game::GetSettings() const noexcept {
    return GameBase::GetSettings();
}

GameSettings& Game::GetSettings() noexcept {
    return GameBase::GetSettings();
}

void Game::HandlePlayerInput(TimeUtils::FPSeconds deltaSeconds) {
    HandleKeyboardInput(deltaSeconds);
    HandleControllerInput(deltaSeconds);
    HandleMouseInput(deltaSeconds);
}

void Game::HandleKeyboardInput(TimeUtils::FPSeconds /*deltaSeconds*/) {
    if(g_theInputSystem->WasKeyJustPressed(KeyCode::Esc)) {
        auto* app = ServiceLocator::get<IAppService, NullAppService>();
        app->SetIsQuitting(true);
        return;
    }
}

void Game::HandleControllerInput(TimeUtils::FPSeconds /*deltaSeconds*/) {

}

void Game::HandleMouseInput(TimeUtils::FPSeconds /*deltaSeconds*/) {

}

void Game::HandleDebugInput(TimeUtils::FPSeconds deltaSeconds) {
    HandleDebugKeyboardInput(deltaSeconds);
    HandleDebugMouseInput(deltaSeconds);
}

void Game::HandleDebugKeyboardInput(TimeUtils::FPSeconds /*deltaSeconds*/) {
    if(g_theUISystem->WantsInputKeyboardCapture()) {
        return;
    }
    if(g_theInputSystem->WasKeyJustPressed(KeyCode::Up)) {
        static const auto length = 1;
        if(cur_index < length - 1) {
            cur_index += 1;
        }
    } else if(g_theInputSystem->WasKeyJustPressed(KeyCode::Down)) {
        if(0 < cur_index) {
            cur_index -= 1;
        }
    }
    if(g_theInputSystem->WasKeyJustPressed(KeyCode::F4)) {
        g_theUISystem->ToggleImguiDemoWindow();
    }
}

void Game::HandleDebugMouseInput(TimeUtils::FPSeconds /*deltaSeconds*/) {
    if(g_theUISystem->WantsInputMouseCapture()) {
        return;
    }
}
