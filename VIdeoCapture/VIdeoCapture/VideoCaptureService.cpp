#include "VideoCaptureService.h"
#include <assert.h>
#include "Utils.h"

void CVideoCaptureService::Init(HWND window)
{
    clientWindow = window;
    initCaptureGraphBuilder();

    // Получение IMediaControl
    IMediaControl* pMediaControl = NULL;
    ThrowIfError(L"IMediaControl", graph.Object()->QueryInterface(IID_IMediaControl, reinterpret_cast<void **>(&pMediaControl)));
    mediaControl.Set(std::move(pMediaControl));

    // Получение IMediaEvent
    IMediaEvent* pMediaEvent = NULL;
    ThrowIfError(L"IMediaEvent", graph.Object()->QueryInterface(IID_IMediaEvent, reinterpret_cast<void **>(&pMediaEvent)));
    mediaEvent.Set(std::move(pMediaEvent));

    // Получение доступных устройств захвата и выбор первого из них
    availableDevices = obtainAvailableVideoDevices();
    SelectVideoDevice(0);
    initRenderer();
}

void CVideoCaptureService::initCaptureGraphBuilder()
{
    assert(!builder.Exist() && !graph.Exist());
    // CaptureGraphBuilder2
    ICaptureGraphBuilder2* pBuilder = NULL;
    ThrowIfError(L"Fail to create CaptureGraphBuilder2",
        CoCreateInstance(CLSID_CaptureGraphBuilder2,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_ICaptureGraphBuilder2,
            reinterpret_cast<void**>(&pBuilder)));
    builder.Set(std::move(pBuilder));

    // GraphBuilder
    IGraphBuilder* pGraph = NULL;
    ThrowIfError(L"Fail to create IGraphBuilder", 
        CoCreateInstance(CLSID_FilterGraph, 
            0,
            CLSCTX_INPROC_SERVER, 
            IID_IGraphBuilder,
            reinterpret_cast<void**>(&pGraph)));
    graph.Set(std::move(pGraph));

    ThrowIfError(L"SetFiltergraph", builder.Object()->SetFiltergraph(graph.Object()));
}

// TODO: static
std::vector<VideoDevice> getDeviceInformation(CComHolder<IEnumMoniker> &enumMoniker)
{
    std::vector<VideoDevice> devices;
    enumMoniker.Object()->Reset();
    IMoniker* pMoniker = NULL;

    while (enumMoniker.Object()->Next(1, &pMoniker, NULL) == S_OK)
    {
        CComHolder<IMoniker> newDevice(std::move(pMoniker));

        // Получение сведений об устройстве
        IPropertyBag* pPropertyBag = NULL;
        HRESULT hr = newDevice.Object()->BindToStorage(0, 0, IID_PPV_ARGS(&pPropertyBag));
        CComHolder<IPropertyBag> propertyBag(std::move(pPropertyBag));
        if (FAILED(hr)) {
            continue;
        }

        VARIANT var;
        VariantInit(&var);
        hr = propertyBag.Object()->Read(L"Description", &var, 0);
        if (FAILED(hr)) {
            hr = propertyBag.Object()->Read(L"FriendlyName", &var, 0);
        }
        if (SUCCEEDED(hr)) {
            devices.push_back({ std::wstring(var.bstrVal), std::move(newDevice) });
            VariantClear(&var);
        }
    }
    return devices;
}

std::vector<VideoDevice> CVideoCaptureService::obtainAvailableVideoDevices()
{ 
    ICreateDevEnum* pSystemDeviceEnum = NULL;
    ThrowIfError(L"Create SystemDeviceEnum", CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSystemDeviceEnum)));
    CComHolder<ICreateDevEnum> systemDeviceEnum(std::move(pSystemDeviceEnum));

    IEnumMoniker* pDevicesEnum = NULL;
    ThrowIfError(L"CreateClassEnumerator", systemDeviceEnum.Object()->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pDevicesEnum, 0));
    CComHolder<IEnumMoniker> devicesEnum(std::move(pDevicesEnum));

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
    assert(graph.Exist());
    if (index > availableDevices.size()) {
        throw std::wstring(L"Device index is out of bounds");
    }
    // TODO: отпустить предыдущий
    selectedDevice = index;
    VideoDevice& device = availableDevices[selectedDevice];
    // Удаляем предыдущий из графа
    if (captureFilter.Exist()) {
        // TODO: stop
        ThrowIfError(L"Fail to remove filter from graph", graph.Object()->RemoveFilter(captureFilter.Object()));
        // TODO: if was running resume
    }

    IBaseFilter* pCaptureFilter = NULL;
    ThrowIfError(L"Fail to use video device", device.moniker.Object()->BindToObject(0, 0, IID_IBaseFilter, (void**)&pCaptureFilter));
    captureFilter.Set(std::move(pCaptureFilter));

    ThrowIfError(L"Fail to add filter to graph", graph.Object()->AddFilter(captureFilter.Object(), L"Capture Filter"));
}

void CVideoCaptureService::StartPreview()
{
    assert(clientWindow != NULL);
    if (!isInitialized()) {
        throw std::wstring(L"Not ready to preview");
    }
    buildGraph();
    ThrowIfError(L"Graph Run error", mediaControl.Object()->Run());

    // Устанавливаем оптимальный размер превью
    long width, height;
    ThrowIfError(L"GetNativeVideoSize", windowlessControl.Object()->GetNativeVideoSize(&width, &height, NULL, NULL));
    RECT rcDest = { 0, 0, width, height };
    windowlessControl.Object()->SetVideoPosition(NULL, &rcDest);
}

// TODO: smart_ptr
void CVideoCaptureService::ObtainCurrentImage(BITMAPINFOHEADER*& currentImage)
{
    ThrowIfError(L"Fail to get Current Image", windowlessControl.Object()->GetCurrentImage(reinterpret_cast<BYTE**>(&currentImage)));
}

void CVideoCaptureService::initRenderer()
{
    // Создаем VideoMixingRenderer9
    IBaseFilter* pRenderer = NULL;
    ThrowIfError(L"Creating VideoMixingRenderer9", CoCreateInstance(CLSID_VideoMixingRenderer9, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, 
        reinterpret_cast<void**>(&pRenderer)));
    videoMixingRenderer9.Set(std::move(pRenderer));

    // Добавляем его в граф
    ThrowIfError(L"Add VideoMixingRenderer9", graph.Object()->AddFilter(videoMixingRenderer9.Object(), L"VMR9"));

    // Убедимся что используется windowless mode и получим WindowlessControl9
    IVMRFilterConfig9* pFilterConfig = NULL;
    ThrowIfError(L"IVMRFilterConfig9", videoMixingRenderer9.Object()->QueryInterface(IID_IVMRFilterConfig9, 
        reinterpret_cast<void**>(&pFilterConfig)));
    CComHolder<IVMRFilterConfig9> filterConfig(std::move(pFilterConfig));

    ThrowIfError(L"SetRenderingMode", filterConfig.Object()->SetRenderingMode(VMR9Mode_Windowless));

    IVMRWindowlessControl9* pWindowlessControl = NULL;
    ThrowIfError(L"IVMRWindowlessControl9", videoMixingRenderer9.Object()->QueryInterface(IID_IVMRWindowlessControl9, 
        reinterpret_cast<void**>(&pWindowlessControl)));
    windowlessControl.Set(std::move(pWindowlessControl));

    ThrowIfError(L"SetVideoClippingWindow", windowlessControl.Object()->SetVideoClippingWindow(clientWindow));
    //ThrowIfError(L"IVMRMixerControl9", videoMixingRenderer9.Object()->QueryInterface(IID_IVMRMixerControl9, (void**)&mixerControl.o));
}

bool CVideoCaptureService::isInitialized()
{
    return graph.Exist() && builder.Exist() 
        && mediaControl.Exist() 
        && mediaEvent.Exist() 
        && videoMixingRenderer9.Exist() 
        && captureFilter.Exist() 
        //&& mixerControl.Exist() 
        //&& filterConfig.Exist() 
        && windowlessControl.Exist();
}

void CVideoCaptureService::buildGraph()
{
    assert(isInitialized());
    ThrowIfError(L"Creating Render Graph", 
        builder.Object()->RenderStream(&PIN_CATEGORY_PREVIEW, 
            &MEDIATYPE_Video, 
            captureFilter.Object(), 
            NULL, 
            videoMixingRenderer9.Object()));
}
