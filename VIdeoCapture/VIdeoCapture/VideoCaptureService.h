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

    // TODO: ��������� ���������� � IbaseFilte?
    ~CComHolder() {
        if (object != NULL) {
            object->Release();
        }
    }

    bool Exist() {
        return object != NULL;
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
    CVideoCaptureService::CVideoCaptureService() : graph(NULL), build(NULL), clientWindow(NULL) { }

    // ���������� �������� ����� ������� �����-���� �������� � ��������.
    // HWND window - ���� � ������� ������������ ������.
    // ������� ������ COM �������, �������� ����������� ���������� ������������.
    void Init(HWND window);

    std::vector<std::wstring> GetAvailableVideoDevicesInfo();

    // ������������� ���������� ����� ���������� �������(�������� GetAvailableVideoDevicesInfo()),
    // ������� ������� ������������.
    void SelectVideoDevice(size_t index);

    void StartPreview();
    BITMAPINFOHEADER* ObtainCurrentImage();

    static const UINT MediaEventMessage = WM_APP + 1;

private:
    void CVideoCaptureService::initCaptureGraphBuilder(CComHolder<IGraphBuilder> &graph, CComHolder<ICaptureGraphBuilder2> &build);
    std::vector<VideoDevice> obtainAvailableVideoDevices();
    void prepareGraph();
    void prepareRenderer();
    bool isInitialized();
    bool isRendererReady();
    bool isCaptureDevice();

    CComHolder<IGraphBuilder> graph;
    CComHolder<ICaptureGraphBuilder2> build;
    CComHolder<IMediaControl> mediaControl;
    CComHolder<IMediaEvent> mediaEvent;
    CComHolder<IBaseFilter> captureFilter;
    CComHolder<IBaseFilter> videoMixingRendered9;
    CComHolder<IVMRMixerControl9> mixerControl;
    CComHolder<IVMRFilterConfig9> filterConfig;
    CComHolder<IVMRWindowlessControl9> windowlessControl;

    // ���� ����������� ������
    HWND clientWindow;
    std::vector<VideoDevice> availableDevices;
    size_t selectedDevice;
};

