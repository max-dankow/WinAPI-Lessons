#pragma once
#include <windows.h>
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include <vector>

class CBitmap {
public:
    CBitmap() : bitmap(NULL) {}

    CBitmap(const CBitmap&) = delete;
    CBitmap operator=(const CBitmap&) = delete;

    CBitmap(CBitmap&& other) {
        this->bitmap = other.bitmap;
        other.bitmap = NULL;
    }

    CBitmap& operator=(CBitmap&& other) {
        this->bitmap = other.bitmap;
        other.bitmap = NULL;
        return *this;
    }

    CBitmap(BITMAPINFOHEADER*&& bitmap) {
        this->bitmap = bitmap;
        bitmap = NULL;
    }

    ~CBitmap() {
        Release();
    }

    bool IsNull() const {
        return bitmap == NULL;
    }

    BITMAPINFOHEADER* GetImage() const {
        return bitmap;
    }

    void Release() {
        if (bitmap != NULL) {
            CoTaskMemFree(bitmap);
            bitmap = NULL;
        }
    }
private:
    BITMAPINFOHEADER* bitmap;
};

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

    // ���������� �������� ����� ������� �����-���� �������� � ��������.
    // HWND window - ���� � ������� ������������ ������.
    // ������� ������ COM �������, �������� ������ ��������� ���������� ������������.
    void Init(HWND window);

    std::vector<std::wstring> GetAvailableVideoDevicesInfo();

    // ������������� ���������� ����� ���������� �������(�������� GetAvailableVideoDevicesInfo()),
    // ������� ������� ������������.
    void SelectVideoDevice(size_t index);

    void CVideoCaptureService::StartPreview(RECT previewRect = { 0, 0, 0, 0 });

    CBitmap CVideoCaptureService::ObtainCurrentImage();

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

    // ���� ����������� ������
    HWND clientWindow;
    std::vector<VideoDevice> availableDevices;
    size_t selectedDevice;
};

