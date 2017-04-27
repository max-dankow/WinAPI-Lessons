#include "VideoCaptureService.h"
#include "Utils.h"

void CVideoCaptureService::Init(HWND window)
{
    this->window = window;
    ThrowIfError(L"Fail to initialize Capture Graph Builder", initCaptureGraphBuilder(graph, build));
    availableDevices = obtainAvailableVideoDevices();
    SelectVideoDevice(0);
}

// The caller must release both interfaces.
HRESULT CVideoCaptureService::initCaptureGraphBuilder(CComHolder<IGraphBuilder> &graph, CComHolder<ICaptureGraphBuilder2> &build)
{
    if (graph.object != NULL || build.object != NULL)
    {
        return E_POINTER;
    }

    // Create the Capture Graph Builder.
    HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&build);
    if (SUCCEEDED(hr))
    {
        // Create the Filter Graph Manager.
        hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&graph);
        if (SUCCEEDED(hr)) {
            // Initialize the Capture Graph Builder.
            build.object->SetFiltergraph(graph.object);
            return S_OK;
        }
        else {
            build.object->Release();
            build = NULL;
        }
    }
    return hr; // Failed
}

HRESULT EnumerateDevices(REFGUID category, CComHolder<IEnumMoniker> &enumMoniker)
{
    // Create the System Device Enumerator.
    ICreateDevEnum *pDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

    if (SUCCEEDED(hr))
    {
        // Create an enumerator for the category.
        hr = pDevEnum->CreateClassEnumerator(category, &enumMoniker.object, 0);
        if (hr == S_FALSE)
        {
            hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
        }
        pDevEnum->Release();
    }
    return hr;
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
    HRESULT hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, devicesEnum);
    ThrowIfError(L"Fail to get available video devices", hr);
    if (SUCCEEDED(hr)) {
        return getDeviceInformation(devicesEnum);
    } else {
        return std::vector<VideoDevice>();
    }
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
    if (index > availableDevices.size()) {
        throw std::wstring(L"Device index in out of bounds");
    }
    selectedDevice = index;
    VideoDevice& device = availableDevices[selectedDevice];
    HRESULT hr = device.moniker.object->BindToObject(0, 0, IID_IBaseFilter, (void**)&pCap.object);
    if (SUCCEEDED(hr))
    {
        hr = graph.object->AddFilter(pCap.object, L"Capture Filter");
    }
}

void CVideoCaptureService::StartPreview()
{
    if (pCap.object == NULL) {
        throw std::wstring(L"Video Device is not selected");
    }
    if (window == NULL) {
        throw std::wstring(L"No parent window");
    }
    prepareGraph();
    //setupVideoWindow();
    ThrowIfError(L"Graph Run error", pControl.object->Run());
    long width, height;

    ThrowIfError(L"GetNativeVideoSize", pWC.object->GetNativeVideoSize(&width, &height, NULL, NULL));

    // explicitly convert System::Drawing::Rectangle type to RECT type
    // TODO: this
    RECT rcDest;
    rcDest.top = 0;
    rcDest.bottom = height;
    rcDest.left = 0;
    rcDest.right = width;

    // set destination rectangle for the video
    pWC.object->SetVideoPosition(NULL, &rcDest);
    long evCode = 0;
}

BITMAPINFOHEADER* CVideoCaptureService::ObtainCurrentImage()
{
    long size = 0;
    //ThrowIfError(L"Fail to get Current Image size", basicVideo.object->GetCurrentImage(&size, NULL));

    //ThrowIfError(L"Fail to Pause", pControl.object->Pause());
    BYTE *lpDib = NULL;

    ThrowIfError(L"Fail to get Current Image", pWC.object->GetCurrentImage(&lpDib));

    //ThrowIfError(L"Fail to Resume(as Run())", pControl.object->Run());

    return (BITMAPINFOHEADER*)lpDib; // TODO: leak  -> CoTaskMemFree(lpDib);
}

void CVideoCaptureService::prepareGraph()
{
    ThrowIfError(L"Creating VideoMixingRenderer9", CoCreateInstance(CLSID_VideoMixingRenderer9, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pVmr.object));
    ThrowIfError(L"Add VideoMixingRenderer9", graph.object->AddFilter(pVmr.object, L"VMR9"));
    ThrowIfError(L"IVMRFilterConfig9", pVmr.object->QueryInterface(IID_IVMRFilterConfig9, (void**)&pConfig.object));
    // make sure VMR9 is in windowless mode
    ThrowIfError(L"SetRenderingMode", pConfig.object->SetRenderingMode(VMR9Mode_Windowless));

    // get a pointer to the IVMRWindowlessControl9 interface
    ThrowIfError(L"IVMRWindowlessControl9", pVmr.object->QueryInterface(IID_IVMRWindowlessControl9, (void**)&pWC.object));

    // specify the container window that the video should be clipped to    
    pWC.object->SetVideoClippingWindow(window);
    // IVMRMixerControl manipulates video streams
    pVmr.object->QueryInterface(IID_IVMRMixerControl9, (void**)&pMix);


    ThrowIfError(L"Creating Render Graph", build.object->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pCap.object, NULL, pVmr.object));
    ThrowIfError(L"IMediaControl", graph.object->QueryInterface(IID_IMediaControl, (void **)&pControl.object));
    ThrowIfError(L"IMediaEvent", graph.object->QueryInterface(IID_IMediaEvent, (void **)&pEvent.object));
}
