#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <string>
#include <iostream>

//#include "bradleybinarization.h"
#include "ImageReader.h"

using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

class GdiService {
public:
    GdiService() {
        Gdiplus::GdiplusStartupInput gdiplusStartupInput;
        Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    }

    ~GdiService() {
        Gdiplus::GdiplusShutdown(gdiplusToken);
    }
private:
    ULONG_PTR gdiplusToken;
};

void saveToPng(Bitmap& image, const std::wstring& outputPath) {
    CLSID pngClsid;
    CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &pngClsid);
    image.Save(outputPath.c_str(), &pngClsid, NULL);
}


void processImage(const std::wstring& filePath, const std::wstring& outputFile) {
    CMatrix<CColor> inputMatrix(CImageReader::read(filePath));
    //Bitmap result = filter.apply(inputImage);
    //saveToPng(result, outputFile);
}

int wmain(int argc, wchar_t* argv[]) {
    GdiService gdiPlus;

    for (int i = 1; i < argc; ++i) {
        std::cout << "processing " << i << " of " << argc - 1 << std::endl;
        try {
            processImage(argv[i], std::to_wstring(i) + L".png");
        } catch (const std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    return 0;
}