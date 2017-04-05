#include <windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <assert.h>

#include "../Utils/Utils.h"

static const int NumberOfThreads = 1;

class CWorker {
public:
    CWorker(const PROCESS_INFORMATION processInfo, const HANDLE terminationEvent) :
        processInfo(processInfo),
        terminationEvent(terminationEvent) {
        dataIsReadyEvent = GetDataIsReadyEvent(processInfo.dwProcessId);
        workIsDoneEvent = GetWorkIsDoneEvent(processInfo.dwProcessId);
    }

    HANDLE GetTerminationEvent() const {
        return terminationEvent;
    }

    void Notify() {
        SetEvent(dataIsReadyEvent);
    }

    HANDLE OnWorkIsDoneEvent() const {
        return workIsDoneEvent;
    }

    void Join() {
        if (processInfo.hProcess != INVALID_HANDLE_VALUE) {
            WaitForSingleObject(processInfo.hProcess, INFINITE);
            CloseHandle(processInfo.hProcess);
            processInfo.hProcess = INVALID_HANDLE_VALUE;
        } else {
            throw std::runtime_error("Trying to join already joined or not created worker");
        }
    }

private:
    PROCESS_INFORMATION processInfo;
    HANDLE terminationEvent, dataIsReadyEvent, workIsDoneEvent;
};

struct CMappedFile {
    HANDLE fileHandle, mappingHandle;
    PVOID mappedFilePtr;
    size_t size;
};

HANDLE CreateTerminationEvent()
{
    SECURITY_ATTRIBUTES attributes;
    attributes.nLength = sizeof(attributes);
    attributes.lpSecurityDescriptor = NULL;
    attributes.bInheritHandle = TRUE;
    HANDLE terminationEvent = CreateEvent(&attributes, TRUE, FALSE, 0);
    if (terminationEvent == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to create event");
    }
    return terminationEvent;
}

std::vector<CWorker> InitWorkers(size_t numberOfWorkers, const std::wstring &dictionaryPath, const CMappedFile &source) 
{
    std::vector<CWorker> workers;
    if (numberOfWorkers < 1) {
        return workers;
    }

    std::wstring workerPath(L"Worker.exe");
    HANDLE terminationEvent = CreateTerminationEvent();
    
    for (size_t i = 0; i < numberOfWorkers; ++i) {
        std::wstring arguments = workerPath.append(L" ")
            .append(dictionaryPath).append(L" ")
            .append(std::to_wstring(reinterpret_cast<int> (terminationEvent))).append(L" ")
            .append(std::to_wstring(reinterpret_cast<int> (source.mappingHandle))).append(L" ")
            .append(std::to_wstring(0)).append(L" ") // начало области файла, назначенной процессу
            .append(std::to_wstring(source.size));

        auto cArguments = std::make_unique<wchar_t[]>(arguments.length() + 1);
        wcscpy(cArguments.get(), arguments.c_str());

        PROCESS_INFORMATION processInfo;
        STARTUPINFO startUpInfo = { sizeof(startUpInfo) };
        if (CreateProcess(0, cArguments.get(), 0, 0, TRUE, 0, 0, 0, &startUpInfo, &processInfo) != 0) {
            std::cerr << "Process created " << processInfo.dwProcessId << std::endl;
            CloseHandle(processInfo.hThread);
            CWorker worker(processInfo, terminationEvent);
            workers.push_back(worker);
        } else {
            DWORD lerr = GetLastError();
            throw std::runtime_error("Failed to create a process");
        }
    }
    return workers;
}

void JoinWorkers(std::vector<CWorker> &workers) 
{
    for (CWorker& worker : workers) {
        worker.Join();
    }
}

void WaitForResults(const std::vector<CWorker> &workers)
{
    auto awaitedEvents = std::make_unique<HANDLE[]>((workers.size()));
    for (size_t i = 0; i < workers.size(); ++i) {
        awaitedEvents.get()[i] = workers[i].OnWorkIsDoneEvent();
    }
    DWORD waitResult = WaitForMultipleObjects(workers.size(), awaitedEvents.get(), TRUE, INFINITE);
    assert(waitResult != WAIT_FAILED && waitResult != WAIT_TIMEOUT);
    std::cerr << "Accepted" << std::endl;
}

CMappedFile OpenAndMapFile(const std::wstring &filePath)
{
    HANDLE fileHandle = CreateFileW(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Can't open file. Error code");
    }

    SECURITY_ATTRIBUTES attributes;
    attributes.nLength = sizeof(attributes);
    attributes.lpSecurityDescriptor = NULL;
    attributes.bInheritHandle = TRUE;
    HANDLE mappingHandle = CreateFileMapping(fileHandle, &attributes, PAGE_READWRITE, 0, 0, 0);
    if (mappingHandle == 0) {
        throw std::runtime_error("Fail to create file mapping");
    }

    PVOID mappedFilePtr = MapViewOfFile(mappingHandle, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if (mappedFilePtr == 0) {
        throw std::runtime_error("Fail to map file");
    }
    DWORD fileSize = GetFileSize(fileHandle, &fileSize);
    return {fileHandle, mappingHandle, mappedFilePtr, fileSize};
}

void OnTerminate(CMappedFile &mappedFile)
{
    CloseHandle(mappedFile.fileHandle);
    CloseHandle(mappedFile.mappingHandle);
    UnmapViewOfFile(mappedFile.mappedFilePtr);
}

int main(int argc, char* argv[]) {
    try {
        std::wstring sourceFilePath;
        auto mappedSource = OpenAndMapFile(GetWStringFromArguments(2, "Source file path"));
        auto str = reinterpret_cast<wchar_t*>(mappedSource.mappedFilePtr);
        auto workers = InitWorkers(NumberOfThreads, GetArgument(1, "Dictionary file path"), mappedSource);
        std::cin.ignore();
        for (CWorker &worker : workers) {
            worker.Notify();
        }
        std::cin.ignore();
        WaitForResults(workers);
        SetEvent(workers[0].GetTerminationEvent());
        JoinWorkers(workers);
        OnTerminate(mappedSource);
        std::cin.ignore();
        return 0;
    } catch (std::exception e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}