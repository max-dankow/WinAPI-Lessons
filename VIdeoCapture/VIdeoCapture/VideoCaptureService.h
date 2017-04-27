#pragma once
#include <windows.h>
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include <vector>

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

    // TODO: постоянно исключение с IbaseFilter
    ~CComHolder() {
        if (object != NULL) {
            //object->Release();
        }
    }
    T *object;
};

struct VideoDevice {
    std::wstring name;
    CComHolder<IMoniker> moniker;
};

class CVideoCaptureService
{
public:
    CVideoCaptureService::CVideoCaptureService() : graph(NULL), build(NULL), window(NULL) { }

    void Init(HWND window);
    std::vector<std::wstring> GetAvailableVideoDevicesInfo();
    void SelectVideoDevice(size_t index);
    void StartPreview();
    BITMAPINFOHEADER* ObtainCurrentImage();

    static const UINT MessageMediaEvent = WM_APP + 1;

private:
    HRESULT CVideoCaptureService::initCaptureGraphBuilder(CComHolder<IGraphBuilder> &graph, CComHolder<ICaptureGraphBuilder2> &build);
    std::vector<VideoDevice> obtainAvailableVideoDevices();
    void prepareGraph();

    CComHolder<IGraphBuilder> graph;
    CComHolder<ICaptureGraphBuilder2> build;
    CComHolder<IMediaControl> pControl;
    CComHolder<IMediaEvent> pEvent;
    CComHolder<IBaseFilter> pCap;
    CComHolder<IBaseFilter> pVmr;
    CComHolder<IMediaControl> pMC;
    CComHolder<IVMRMixerControl9> pMix;
    CComHolder<IVMRFilterConfig9> pConfig;
    CComHolder<IVMRWindowlessControl9> pWC;

    HWND window;
    std::vector<VideoDevice> availableDevices;
    size_t selectedDevice;
};

