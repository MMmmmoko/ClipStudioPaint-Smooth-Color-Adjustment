#include "NativeCanvasCoordinates.h"

#include "AddressTable.h"
#include "CSPMOD.h"

#include <atomic>
#include <mutex>

namespace NativeCanvasCoordinates
{
namespace
{
using Converter = int64_t (*)(double*, void*, double*);
Converter originalConvert = nullptr;
thread_local Observer canvasObserver = nullptr;
std::atomic<Observer> filterObserver{nullptr};

int64_t ObserveAndConvert(double* output, void* view, double* position)
{
    if (canvasObserver)
        canvasObserver(view, position);
    if (const auto observer = filterObserver.load())
        observer(view, position);
    return originalConvert(output, view, position);
}

bool Install()
{
    static std::once_flag once;
    std::call_once(once, [] {
        if (auto* address = AddressTable::GetAddress("CSPCustomFilter_WindowPosToCanvasPos_Func"))
            CSPMOD::Hook(address, ObserveAndConvert, reinterpret_cast<void**>(&originalConvert), CSPMOD::_HOOKREG_R11);
    });
    return originalConvert != nullptr;
}
}

bool SetObserver(Observer observer)
{
    if (observer && !Install())
        return false;
    canvasObserver = observer;
    return originalConvert != nullptr;
}

void SetFilterObserver(Observer observer)
{
    if (!observer || Install())
        filterObserver.store(observer);
}

void Convert(double* output, void* retainedView, double* position)
{
    if (originalConvert)
        originalConvert(output, retainedView, position);
}
}
