#pragma once
#include <windows.h>
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include <vector>
#include "ComHolder.h"

// ��������� ���������� ������������
struct VideoDevice {
    std::wstring name;
    CComHolder<IMoniker> moniker;
};


// ������������ ��� ������ �� ������������
// ��������� ���������� ����������� � �������� ����, � ��� �� �������� ��������� �����.
class CVideoCaptureService
{
public:
    CVideoCaptureService() : graph(NULL), builder(NULL), clientWindow(NULL), isRunning(false) {}

    // ���������� �������� ����� ������� �����-���� �������� � ��������. ��������� ����� �� ��������.
    // HWND window - ���� � ������� ������������ ������.
    // ������� ������ COM �������, �������� ������ ��������� ���������� ������������.
    void Init(HWND window);

    // ���������� ������ �������� ��������� ��������� ������������
    std::vector<std::wstring> GetAvailableVideoDevicesInfo();

    // ������������� ���������� ����� ���������� �������(�������� GetAvailableVideoDevicesInfo()),
    // ������� ������� ������������. �������� ������������ ����� �������.
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

    // ICaptureGraphBuilder2 ����� ��� �������� ����� ������������
    CComHolder<ICaptureGraphBuilder2> builder;

    // IMediaControl ��������� ��� ���������� �������� ����� start/stop
    CComHolder<IMediaControl> mediaControl;

    CComHolder<IMediaEvent> mediaEvent;
    CComHolder<IBaseFilter> captureFilter;
    CComHolder<IBaseFilter> videoMixingRenderer9;
    CComHolder<IVMRWindowlessControl9> windowlessControl;

    // ���� ����������� ������
    HWND clientWindow;
    RECT previewRect;
    std::vector<VideoDevice> availableDevices;
    size_t selectedDevice;
    bool isRunning;
};
