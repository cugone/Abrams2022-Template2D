#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineBase.hpp"
#include "Engine/Core/DataUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Engine/Platform/Win.hpp"

#include "Game/Game.hpp"

#pragma warning(push)
#pragma warning(disable: 28251)

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    UNUSED(hInstance);
    UNUSED(hPrevInstance);
    UNUSED(nCmdShow);

    auto cmdString = StringUtils::ConvertUnicodeToMultiByte(std::wstring(pCmdLine ? pCmdLine : L""));
    Engine<Game>::Initialize("Abrams 2022 2D Template", cmdString);
    if(Engine<Game>::Available()) {
        Engine<Game>::Run();
        Engine<Game>::Shutdown();
    }
}

#pragma warning(pop)
