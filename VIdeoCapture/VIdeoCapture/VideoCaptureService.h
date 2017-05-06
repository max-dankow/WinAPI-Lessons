#pragma once
#include <windows.h>
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include <vector>
#include "Bitmap.h"

template <typename T>
class CComHolder {
public:
    CComHolder(T*&& object = NULL) : object(object) {
        object = NULL;
    }

    CComHolder(const CComHolder&) = delete;
    CComHolder operator=(const CComHolder&) = delete;

    CComHolder(CComHolder&& other) {
        this->object = other.object;
        other.object = NULL;
    }

    CComHolder& operator=(CComHolder&& other) {
        this->object = other.object;
        other.object = NULL;
        return *this;
    }

    ~CComHolder() {
        if (Exist()) {
            object->Release();
        }
    }

    bool Exist() {
        return object != NULL;
    }

    void Set(T*&& pointer) {
        if (Exist()) {
            object->Release();
            object = NULL;
        }
        object = pointer;
        pointer = NULL;
    }

    T* Object() {
        return object;
    }

private:
    T *object;
};

struct VideoDevice {
    std::wstring name;
    CComHolder<IMoniker> moniker;
};

class CVideoCaptureService
{
public:
    // TODO: CoInitialize is strange
    CVideoCaptureService() : graph(NULL), builder(NULL), clientWindow(NULL) {}

    // Необходимо вызывать перед началом каких-либо действий с сервисом.
    // HWND window - окно в которое встраивается сервис.
    // Создает нужные COM объекты, выбирает первое доступное устройство видеозахвата.
    void Init(HWND window);

    std::vector<std::wstring> GetAvailableVideoDevicesInfo();

    // Устанавливает порядковый номер устройства захвата(согласно GetAvailableVideoDevicesInfo()),
    // которое следует использовать.
    void SelectVideoDevice(size_t index);

    void CVideoCaptureService::StartPreview(RECT previewRect = { 0, 0, 0, 0 });

    BITMAPINFOHEADER* CVideoCaptureService::ObtainCurrentImage();

    static const UINT MediaEventMessage = WM_APP + 1;

private:
    void CVideoCaptureService::initCaptureGraphBuilder();
    std::vector<VideoDevice> obtainAvailableVideoDevices();
    void buildGraph();
    void initRenderer();
    bool isInitialized();

    CComHolder<IGraphBuilder> graph;
    CComHolder<ICaptureGraphBuilder2> builder;
    CComHolder<IMediaControl> mediaControl;
    CComHolder<IMediaEvent> mediaEvent;
    CComHolder<IBaseFilter> captureFilter;
    CComHolder<IBaseFilter> videoMixingRenderer9;
    //CComHolder<IVMRMixerControl9> mixerControl;
    //CComHolder<IVMRFilterConfig9> filterConfig;
    CComHolder<IVMRWindowlessControl9> windowlessControl;

    // Окно использущее сервис
    HWND clientWindow;
    std::vector<VideoDevice> availableDevices;
    size_t selectedDevice;
};

