/*
MIT License

Copyright (c) 2018 Benjamin H�glinger

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdarg.h>

//
// Public
// 
#include "Indicium/Engine/IndiciumCore.h"
#include "Indicium/Engine/IndiciumDirect3D9.h"
#include "Indicium/Engine/IndiciumDirect3D10.h"
#include "Indicium/Engine/IndiciumDirect3D11.h"
#include "Indicium/Engine/IndiciumDirect3D12.h"

//
// Internal
// 
#include "Engine.h"
#include "Game/Game.h"
#include "Global.h"
#include "Utils/Logging.h"

////
//// Boost
////
//#include <boost/log/core.hpp>
//#include <boost/log/trivial.hpp>
//#include <boost/log/expressions.hpp>
//#include <boost/log/utility/setup/file.hpp>
//#include <boost/log/utility/setup/common_attributes.hpp>
//#include <boost/log/attributes/named_scope.hpp>
//
//namespace logging = boost::log;
//namespace keywords = boost::log::keywords;
//namespace attrs = boost::log::attributes;


INDICIUM_API PINDICIUM_ENGINE IndiciumEngineAlloc(void)
{
    const auto engine = static_cast<PINDICIUM_ENGINE>(malloc(sizeof(INDICIUM_ENGINE)));

    if (!engine) {
        return nullptr;
    }

    ZeroMemory(engine, sizeof(INDICIUM_ENGINE));

    return engine;
}

INDICIUM_API INDICIUM_ERROR IndiciumEngineInit(PINDICIUM_ENGINE Engine, PFN_INDICIUM_GAME_HOOKED EvtIndiciumGameHooked)
{
    //BOOST_LOG_NAMED_SCOPE(__func__);

    if (!Engine) {
        return INDICIUM_ERROR_INVALID_ENGINE_HANDLE;
    }

    //
    // Callback invoked when initialization finished
    // 
    Engine->EvtIndiciumGameHooked = EvtIndiciumGameHooked;

    //
    // Event to notify engine thread about termination
    // 
    Engine->EngineCancellationEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    //
    // Set up logging
    // 
    //logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");

    //logging::add_file_log(
    //    keywords::file_name = Indicium::Core::Util::expand_environment_variables("%TEMP%\\Indicium-Supra.log"),
    //    keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] [%Scope%] %Message%",
    //    keywords::auto_flush = true
    //);

    //logging::core::get()->set_filter
    //(
    //    logging::trivial::severity >= logging::trivial::info
    //);

    //logging::core::get()->add_global_attribute("Scope", attrs::named_scope());
    //logging::add_common_attributes();


    //BOOST_LOG_TRIVIAL(info) << "Indicium engine initialized, attempting to launch main thread";

    //
    // Kickstart hooking the rendering pipeline
    // 
    Engine->EngineThread = CreateThread(
        nullptr,
        0,
        reinterpret_cast<LPTHREAD_START_ROUTINE>(IndiciumMainThread),
        Engine,
        0,
        nullptr
    );

    if (!Engine->EngineThread) {
        //BOOST_LOG_TRIVIAL(fatal) << "Could not create main thread, library unusable";
        return INDICIUM_ERROR_CREATE_THREAD_FAILED;
    }

    //BOOST_LOG_TRIVIAL(info) << "Main thread created successfully";

    return INDICIUM_ERROR_NONE;
}

INDICIUM_API VOID IndiciumEngineShutdown(PINDICIUM_ENGINE Engine, PFN_INDICIUM_GAME_UNHOOKED EvtIndiciumGameUnhooked)
{
    //BOOST_LOG_NAMED_SCOPE(__func__);

    if (!Engine) {
        return;
    }

    //BOOST_LOG_TRIVIAL(info) << "Indicium engine shutdown requested, attempting to terminate main thread";

    const auto ret = SetEvent(Engine->EngineCancellationEvent);

    if (!ret)
    {
        //BOOST_LOG_TRIVIAL(error) << "SetEvent failed: " << std::hex << GetLastError();
    }

    const auto result = WaitForSingleObject(Engine->EngineThread, 3000);

    switch (result)
    {
    case WAIT_ABANDONED:
        //BOOST_LOG_TRIVIAL(error) << "Unknown state, host process might crash";
        break;
    case WAIT_OBJECT_0:
        //BOOST_LOG_TRIVIAL(info) << "Hooks removed, notifying caller";
        break;
    case WAIT_TIMEOUT:
        TerminateThread(Engine->EngineThread, 0);
        //BOOST_LOG_TRIVIAL(error) << "Thread hasn't finished clean-up within expected time, terminating";
        break;
    case WAIT_FAILED:
        //BOOST_LOG_TRIVIAL(error) << "Unknown error, host process might crash";
        break;
    default:
        TerminateThread(Engine->EngineThread, 0);
        //BOOST_LOG_TRIVIAL(error) << "Unexpected return value, terminating";
        break;
    }

    if (EvtIndiciumGameUnhooked) {
        EvtIndiciumGameUnhooked();
    }

    CloseHandle(Engine->EngineCancellationEvent);
    CloseHandle(Engine->EngineThread);

    //BOOST_LOG_TRIVIAL(info) << "Engine shutdown complete";
}

INDICIUM_API VOID IndiciumEngineFree(PINDICIUM_ENGINE Engine)
{
    if (!Engine) {
        return;
    }

    free(Engine);
}

INDICIUM_API VOID IndiciumEngineSetD3D9EventCallbacks(PINDICIUM_ENGINE Engine, PINDICIUM_D3D9_EVENT_CALLBACKS Callbacks)
{
    if (Engine) {
        Engine->EventsD3D9 = *Callbacks;
    }
}

INDICIUM_API VOID IndiciumEngineSetD3D10EventCallbacks(PINDICIUM_ENGINE Engine, PINDICIUM_D3D10_EVENT_CALLBACKS Callbacks)
{
    if (Engine) {
        Engine->EventsD3D10 = *Callbacks;
    }
}

INDICIUM_API VOID IndiciumEngineSetD3D11EventCallbacks(PINDICIUM_ENGINE Engine, PINDICIUM_D3D11_EVENT_CALLBACKS Callbacks)
{
    if (Engine) {
        Engine->EventsD3D11 = *Callbacks;
    }
}

INDICIUM_API VOID IndiciumEngineSetD3D12EventCallbacks(PINDICIUM_ENGINE Engine, PINDICIUM_D3D12_EVENT_CALLBACKS Callbacks)
{
    if (Engine) {
        Engine->EventsD3D12 = *Callbacks;
    }
}

INDICIUM_API VOID IndiciumEngineLogDebug(LPCSTR Format, ...)
{
    //BOOST_LOG_NAMED_SCOPE(__func__);

    va_list args;
    va_start(args, Format);

    //BOOST_LOG_TRIVIAL(debug) << Indicium::Core::Logging::format(Format, args);

    va_end(args);
}

INDICIUM_API VOID IndiciumEngineLogInfo(LPCSTR Format, ...)
{
    //BOOST_LOG_NAMED_SCOPE(__func__);

    va_list args;
    va_start(args, Format);

    //BOOST_LOG_TRIVIAL(info) << Indicium::Core::Logging::format(Format, args);

    va_end(args);
}

INDICIUM_API VOID IndiciumEngineLogWarning(LPCSTR Format, ...)
{
    //BOOST_LOG_NAMED_SCOPE(__func__);

    va_list args;
    va_start(args, Format);

    //BOOST_LOG_TRIVIAL(warning) << Indicium::Core::Logging::format(Format, args);

    va_end(args);
}

INDICIUM_API VOID IndiciumEngineLogError(LPCSTR Format, ...)
{
    //BOOST_LOG_NAMED_SCOPE(__func__);

    va_list args;
    va_start(args, Format);

    //BOOST_LOG_TRIVIAL(error) << Indicium::Core::Logging::format(Format, args);

    va_end(args);
}
