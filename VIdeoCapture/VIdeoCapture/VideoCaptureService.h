#pragma once
#include <windows.h>
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include <vector>
#include "ComHolder.h"

// ќписывает устройство видеозахвата
struct VideoDevice {
    std::wstring name;
    CComHolder<IMoniker> moniker;
};


// ќсуществл€ет всю работу по видеозахвату
// ѕозвол€ет встраивать видеопревью в заданное окно, а так же получать отдельные кадры.
class CVideoCaptureService
{
public:
    CVideoCaptureService() : graph(NULL), builder(NULL), clientWindow(NULL), isRunning(false) {}

    // Ќеобходимо вызывать перед началом каких-либо действий с сервисом. ѕовторный вызов не допустим.
    // HWND window - окно в которое встраиваетс€ сервис.
    // —оздает нужные COM объекты, выбирает первое доступное устройство видеозахвата.
    void Init(HWND window);

    // ¬озвращает список названий доступных устройств видеозахвата
    std::vector<std::wstring> GetAvailableVideoDevicesInfo();

    // ”станавливает пор€дковый номер устройства захвата(согласно GetAvailableVideoDevicesInfo()),
    // которое следует использовать. ¬ызывает перестроение графа захвата.
    void SelectVideoDevice(size_t index);

    void StartPreview(RECT previewRect);
    void PausePreview();
    void StopPreview();

    bool IsRunning() const {
        return isRunning;
    }

    // ѕолучает текущее изображение с рендера видеопотока
    BITMAPINFOHEADER* CVideoCaptureService::ObtainCurrentImage();

private:
    void CVideoCaptureService::initCaptureGraphBuilder();
    std::vector<VideoDevice> obtainAvailableVideoDevices();
    void initRenderer();
    void buildGraph();
    bool isInitialized();

    CComHolder<IGraphBuilder> graph;

    // ICaptureGraphBuilder2 нужен дл€ создани€ графа видеозахвата
    CComHolder<ICaptureGraphBuilder2> builder;

    // IMediaControl необходим дл€ управлени€ захватом видео start/stop
    CComHolder<IMediaControl> mediaControl;

    CComHolder<IMediaEvent> mediaEvent;
    CComHolder<IBaseFilter> captureFilter;
    CComHolder<IBaseFilter> videoMixingRenderer9;
    CComHolder<IVMRWindowlessControl9> windowlessControl;

    // ќкно использущее сервис
    HWND clientWindow;
    RECT previewRect;
    std::vector<VideoDevice> availableDevices;
    size_t selectedDevice;
    bool isRunning;
};
