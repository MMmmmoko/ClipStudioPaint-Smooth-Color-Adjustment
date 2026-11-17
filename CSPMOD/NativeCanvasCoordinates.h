#pragma once

namespace NativeCanvasCoordinates
{
using Observer = void (*)(const void* view, const double* position);
bool SetObserver(Observer observer);
void SetFilterObserver(Observer observer);
void Convert(double* output, void* retainedView, double* position);
}
