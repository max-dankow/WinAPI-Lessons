#pragma once
#include <windows.h>
#include <dshow.h>
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

    CComHolder operator=(CComHolder&& other) {
        this->object = other.object;
        other.object = NULL;
    }

    ~CComHolder() {
        if (object != NULL) {
            object->Release();
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
    CVideoCaptureService();
    ~CVideoCaptureService();
   
    void Init();
    std::vector<std::wstring> GetAvailableVideoDevicesInfo();
    void SelectVideoDevice(size_t index);
    void StartPreview();

private:
    HRESULT initCaptureGraphBuilder(IGraphBuilder*& pGraph, ICaptureGraphBuilder2*& pBuild);
    std::vector<VideoDevice> obtainAvailableVideoDevices();
    void prepareGraph();

    IGraphBuilder* pGraph;
    ICaptureGraphBuilder2 *pBuild;
    CComHolder<IBaseFilter> pCap;
    CComHolder<IMediaControl> pControl;
    CComHolder<IMediaEvent> pEvent;


    std::vector<VideoDevice> availableDevices;
    size_t selectedDevice;
};

