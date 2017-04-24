#include "VideoCaptureService.h"
#include "Utils.h"


CVideoCaptureService::CVideoCaptureService() : pGraph(NULL), pBuild(NULL) { }

CVideoCaptureService::~CVideoCaptureService()
{
    if (pBuild != NULL) {
        pBuild->Release();
        pBuild = NULL;
    }
    if (pGraph != NULL) {
        pGraph->Release();
    }
}

void CVideoCaptureService::Init()
{
    ThrowIfError(L"Fail to initialize Capture Graph Builder", initCaptureGraphBuilder(pGraph, pBuild));
}

// The caller must release both interfaces.
HRESULT CVideoCaptureService::initCaptureGraphBuilder(IGraphBuilder *&pGraph, ICaptureGraphBuilder2 *&pBuild)
{
    if (pGraph != NULL || pBuild != NULL)
    {
        return E_POINTER;
    }

    // Create the Capture Graph Builder.
    HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&pBuild);
    if (SUCCEEDED(hr))
    {
        // Create the Filter Graph Manager.
        hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);
        if (SUCCEEDED(hr)) {
            // Initialize the Capture Graph Builder.
            pBuild->SetFiltergraph(pGraph);
            return S_OK;
        }
        else {
            pBuild->Release();
            pBuild = NULL;
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

std::vector<VideoDevice> CVideoCaptureService::GetPossibleVideoSources()
{
    CComHolder<IEnumMoniker> devicesEnum;
    HRESULT hr = EnumerateDevices(CLSID_VideoInputDeviceCategory, devicesEnum);
    ThrowIfError(L"Fail to enumerate video devices", hr);
    if (SUCCEEDED(hr)) {
        return getDeviceInformation(devicesEnum);
    }
    else {
        return std::vector<VideoDevice>();
    }
}