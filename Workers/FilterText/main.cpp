#include <windows.h>
#include <string>
#include <vector>
#include <iostream>

#include "../Utils/Utils.h" 

class CWorker {
public:
    CWorker(const HANDLE processHandle) : processHandle(processHandle) {}

private:
    HANDLE processHandle;
};

std::vector<CWorker> InitWorkers(size_t numberOfWorkers, const std::wstring &dictionaryPath) 
{
    std::vector<CWorker> workers;
    std::wstring workerPath(L"Worker.exe");
    std::wstring arguments = workerPath.append(L" ").append(dictionaryPath);
    wchar_t *cArguments = new wchar_t[arguments.length() + 1];
    wcscpy(cArguments, arguments.c_str());
    
    for (size_t i = 0; i < numberOfWorkers; ++i) {
        PROCESS_INFORMATION processInfo;
        STARTUPINFO startUpInfo = { sizeof(startUpInfo) };
        if (CreateProcess(0, cArguments, 0, 0, TRUE, 0, 0, 0, &startUpInfo, &processInfo) != 0) {
            std::cerr << "Process created " << processInfo.hProcess << std::endl;
            CWorker worker(processInfo.hProcess);
            workers.push_back(worker);
        } else {
            DWORD lerr = GetLastError();
            throw std::runtime_error("Failed to create a process");
        }
    }
    delete[] cArguments;
    return workers;
}

int main(int argc, char* argv[]) {

    try {
        InitWorkers(1, GetDictionaryPathFromArgs());
    } catch (std::exception e) {
        std::cerr << e.what() << std::endl;
    }
    std::cin.ignore();
    return 0;

    SECURITY_ATTRIBUTES attributes;
    attributes.nLength = sizeof(attributes);
    attributes.lpSecurityDescriptor = NULL;
    attributes.bInheritHandle = TRUE;
    HANDLE eventTerminate = CreateEvent(0, TRUE, FALSE, L"ru.mipt.diht.dankovtsev.workers.terminate");


    return 0;
}