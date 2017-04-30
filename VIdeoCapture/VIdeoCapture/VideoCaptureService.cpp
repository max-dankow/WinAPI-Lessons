#include "VideoCaptureService.h"
#include <assert.h>
#include "Utils.h"

void CVideoCaptureService::Init(HWND window)
{
    clientWindow = window;
    try {
        initCaptureGraphBuilder(graph, build);
        ThrowIfError(L"IMediaControl", graph.object->QueryInterface(IID_IMediaControl, reinterpret_cast<void **>(&mediaControl.object)));
        ThrowIfError(L"IMediaEvent", graph.object->QueryInterface(IID_IMediaEvent, reinterpret_cast<void **>(&mediaEvent.object)));
        availableDevices = obtainAvailableVideoDevices();
        SelectVideoDevice(0);
    }
    catch (std::wstring e) {
        ShowError(L"Error during initialization of VideoCaptureService: " + e);
    }
}

void CVideoCaptureService::initCaptureGraphBuilder(CComHolder<IGraphBuilder> &graph, CComHolder<ICaptureGraphBuilder2> &build)
{
    if (graph.object != NULL || build.object != NULL)
    {
        return;
    }

    // Capture Graph Builder.
    ThrowIfError(L"Fail to create CaptureGraphBuilder2",
        CoCreateInstance(CLSID_CaptureGraphBuilder2,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_ICaptureGraphBuilder2,
            reinterpret_cast<void**>(&build)));

    // FilterGraph
    ThrowIfError(L"Fail to create FilterGraph", 
        CoCreateInstance(CLSID_FilterGraph, 
            0, 
            CLSCTX_INPROC_SERVER, 
            IID_IGraphBuilder,
            reinterpret_cast<void**>(&graph)));

    ThrowIfError(L"SetFiltergraph", build.object->SetFiltergraph(graph.object));
}

// TODO: refactor
std::vector<VideoDevice> getDeviceInformation(const CComHolder<IEnumMoniker> &enumMoniker)
{
    std::vector<VideoDevice> devices;
    enumMoniker.object->Reset();
    IMoniker *pMoniker = NULL;

    while (enumMoniker.object->Next(1, &pMoniker, NULL) == S_OK)
    {
        CComHolder<IMoniker> newDevice(std::move(pMoniker));
        IPropertyBag *pPropBag;
        HRESULT hr = newDevice.object->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
        if (FAILED(hr)) {
            continue;
        }

        VARIANT var;
        VariantInit(&var);

        // Get description or friendly name.
        hr = pPropBag->Read(L"Description", &var, 0);
        if (FAILED(hr)) {
            hr = pPropBag->Read(L"FriendlyName", &var, 0);
        }
        if (SUCCEEDED(hr)) {
            devices.push_back({ std::wstring(var.bstrVal), std::move(newDevice) });
            VariantClear(&var);
        }

        pPropBag->Release();
    }
    return devices;
}

std::vector<VideoDevice> CVideoCaptureService::obtainAvailableVideoDevices()
{
    CComHolder<IEnumMoniker> devicesEnum;
    CComHolder<ICreateDevEnum> systemDeviceEnum;
    ThrowIfError(L"Create SystemDeviceEnum", CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&systemDeviceEnum.object)));
    ThrowIfError(L"CreateClassEnumerator", systemDeviceEnum.object->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &devicesEnum.object, 0));
    return getDeviceInformation(devicesEnum);
}

std::vector<std::wstring> CVideoCaptureService::GetAvailableVideoDevicesInfo()
{
    std::vector<std::wstring> devicesInfo;
    for (const VideoDevice& device : availableDevices) {
        devicesInfo.push_back(device.name);
    }
    return devicesInfo;
}

void CVideoCaptureService::SelectVideoDevice(size_t index)
{
    // TODO: isInitialised() method
    assert(graph.object != NULL);
    if (index > availableDevices.size()) {
        throw std::wstring(L"Device index is out of bounds");
    }
    // TODO: отпустить предыдущий
    selectedDevice = index;
    VideoDevice& device = availableDevices[selectedDevice];
    ThrowIfError(L"Fail to use video device", device.moniker.object->BindToObject(0, 0, IID_IBaseFilter, (void**)&captureFilter.object));
    ThrowIfError(L"Fail to add filter to graph", graph.object->AddFilter(captureFilter.object, L"Capture Filter"));
}

void CVideoCaptureService::StartPreview()
{
    if (captureFilter.object == NULL) {
        throw std::wstring(L"Video Device is not selected");
    }
    if (clientWindow == NULL) {
        throw std::wstring(L"No parent window");
    }
    prepareGraph();
    ThrowIfError(L"Graph Run error", mediaControl.object->Run());
    long width, height;

    ThrowIfError(L"GetNativeVideoSize", windowlessControl.object->GetNativeVideoSize(&width, &height, NULL, NULL));

    RECT rcDest = { 0, 0, width, height };

    windowlessControl.object->SetVideoPosition(NULL, &rcDest);
}

BITMAPINFOHEADER* CVideoCaptureService::ObtainCurrentImage()
{
    long size = 0;
    BYTE *lpDib = NULL;
    ThrowIfError(L"Fail to get Current Image", windowlessControl.object->GetCurrentImage(&lpDib));
    return (BITMAPINFOHEADER*)lpDib;
}

void CVideoCaptureService::prepareRenderer()
{
    assert(graph.object != NULL);

    // Создаем VideoMixingRenderer9
    ThrowIfError(L"Creating VideoMixingRenderer9", CoCreateInstance(CLSID_VideoMixingRenderer9, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&videoMixingRendered9.object));

    // Добавляем его в граф
    ThrowIfError(L"Add VideoMixingRenderer9", graph.object->AddFilter(videoMixingRendered9.object, L"VMR9"));

    ThrowIfError(L"IVMRFilterConfig9", videoMixingRendered9.object->QueryInterface(IID_IVMRFilterConfig9, (void**)&filterConfig.object));

    // Убедимся что используется windowless mode и получим WindowlessControl9
    ThrowIfError(L"SetRenderingMode", filterConfig.object->SetRenderingMode(VMR9Mode_Windowless));
    ThrowIfError(L"IVMRWindowlessControl9", videoMixingRendered9.object->QueryInterface(IID_IVMRWindowlessControl9, (void**)&windowlessControl.object));
    ThrowIfError(L"SetVideoClippingWindow", windowlessControl.object->SetVideoClippingWindow(clientWindow));
    ThrowIfError(L"IVMRMixerControl9", videoMixingRendered9.object->QueryInterface(IID_IVMRMixerControl9, (void**)&mixerControl));
}

bool CVideoCaptureService::isInitialized()
{
    return graph.Exist() && build.Exist() 
        && mediaControl.Exist() && mediaEvent.Exist() 
        && videoMixingRendered9.Exist() 
        && captureFilter.Exist() 
        && mixerControl.Exist() 
        && filterConfig.Exist() 
        && windowlessControl.Exist();
}

bool CVideoCaptureService::isRendererReady()
{
    return videoMixingRendered9.Exist();
}

bool CVideoCaptureService::isCaptureDevice()
{
    return captureFilter.Exist();
}

void CVideoCaptureService::prepareGraph()
{
    if (videoMixingRendered9.object == NULL) {
        prepareRenderer();
    }
    ThrowIfError(L"Creating Render Graph", build.object->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, captureFilter.object, NULL, videoMixingRendered9.object));
}
