#pragma once
#include <windows.h>
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include <vector>
#include "ComHolder.h"

// Описывает устройство видеозахвата
struct VideoDevice {
    std::wstring name;
    CComHolder<IMoniker> moniker;
};


// Осуществляет всю работу по видеозахвату
// Позволяет встраивать видеопревью в заданное окно, а так же получать отдельные кадры.
class CVideoCaptureService
{
public:
    CVideoCaptureService() : graph(NULL), builder(NULL), clientWindow(NULL), isRunning(false) {}

    // Необходимо вызывать перед началом каких-либо действий с сервисом. Повторный вызов не допустим.
    // HWND window - окно в которое встраивается сервис.
    // Создает нужные COM объекты, выбирает первое доступное устройство видеозахвата.
    void Init(HWND window);

    // Возвращает список названий доступных устройств видеозахвата
    std::vector<std::wstring> GetAvailableVideoDevicesInfo();

    // Устанавливает порядковый номер устройства захвата(согласно GetAvailableVideoDevicesInfo()),
    // которое следует использовать. Вызывает перестроение графа захвата.
    void SelectVideoDevice(size_t index);

    void StartPreview(RECT previewRect = { 0, 0, 0, 0 });
    void PausePreview();
    void StopPreview();

    bool IsRunning() const {
        return isRunning;
    }

    BITMAPINFOHEADER* CVideoCaptureService::ObtainCurrentImage();

private:
    void CVideoCaptureService::initCaptureGraphBuilder();
    std::vector<VideoDevice> obtainAvailableVideoDevices();
    void initRenderer();
    void buildGraph();
    bool isInitialized();

    CComHolder<IGraphBuilder> graph;

    // ICaptureGraphBuilder2 нужен для создания графа видеозахвата
    CComHolder<ICaptureGraphBuilder2> builder;

    // IMediaControl необходим для управления захватом видео start/stop
    CComHolder<IMediaControl> mediaControl;

    CComHolder<IMediaEvent> mediaEvent;
    CComHolder<IBaseFilter> captureFilter;
    CComHolder<IBaseFilter> videoMixingRenderer9;
    CComHolder<IVMRWindowlessControl9> windowlessControl;

    // Окно использущее сервис
    HWND clientWindow;
    RECT previewRect;
    std::vector<VideoDevice> availableDevices;
    size_t selectedDevice;
    bool isRunning;
};
