#include <windows.h>
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <assert.h>

#include "../Utils/Utils.h"

static const int NumberOfProcesses = 4;

class CWorker {
public:
    CWorker(const PROCESS_INFORMATION processInfo, const HANDLE terminationEvent, CMappedFile &&mappedFile) :
        processInfo(processInfo),
        terminationEvent(terminationEvent),
        mappedFile(std::move(mappedFile)) {
        dataIsReadyEvent = GetDataIsReadyEvent(processInfo.dwProcessId);
        workIsDoneEvent = GetWorkIsDoneEvent(processInfo.dwProcessId);
    }

    CWorker(const CWorker&) = delete;
    void operator=(const CWorker&) = delete;

    CWorker(CWorker&& other) :
        processInfo(other.processInfo),
        terminationEvent(other.terminationEvent),
        dataIsReadyEvent(other.dataIsReadyEvent),
        workIsDoneEvent(other.workIsDoneEvent),
        mappedFile(std::move(other.mappedFile)) {
        other.invalidate();
    }

    void operator=(CWorker &&other) {
        invalidate();
        processInfo = other.processInfo;
        terminationEvent = other.terminationEvent;
        dataIsReadyEvent = other.dataIsReadyEvent;
        workIsDoneEvent = other.workIsDoneEvent;
        mappedFile = std::move(other.mappedFile);
    }

    ~CWorker() {
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        CloseHandle(dataIsReadyEvent);
        CloseHandle(workIsDoneEvent);
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

    std::wstring GetResult() const {
        return std::wstring(reinterpret_cast<wchar_t*>(mappedFile.GetAddr()));
    }

private:
    PROCESS_INFORMATION processInfo;
    HANDLE terminationEvent, dataIsReadyEvent, workIsDoneEvent;
    CMappedFile mappedFile;

    void invalidate() {
        processInfo.hProcess = INVALID_HANDLE_VALUE;
        processInfo.hThread = INVALID_HANDLE_VALUE;
        terminationEvent = INVALID_HANDLE_VALUE;
        dataIsReadyEvent = INVALID_HANDLE_VALUE;
        workIsDoneEvent = INVALID_HANDLE_VALUE;
    }
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

size_t GetTaskSize(int processIndex, const CMappedFile &source, size_t &offset)
{
    size_t taskSize = ((source.GetSize()) / 2 / NumberOfProcesses) * 2;
    if (processIndex + 1 == NumberOfProcesses || source.GetSize() - offset < taskSize) {
        taskSize = source.GetSize() - offset;
        return taskSize;
    }
    while (reinterpret_cast<wchar_t*>(source.GetAddr())[(taskSize + offset) / 2] != L' ' && taskSize + offset < source.GetSize()) {
        taskSize += sizeof(wchar_t);
    }
    
    return taskSize;
}

CMappedFile PrepareSharedMem(DWORD processId, int processIndex, const CMappedFile &source, size_t &offset) {
    size_t taskSize = GetTaskSize(processIndex, source, offset);
    HANDLE mappingHandle = GetSourceMapping(processId, taskSize + sizeof(L'\0'));
    PVOID mappedFilePtr = MapViewOfFile(mappingHandle, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if (mappedFilePtr == 0) {
        throw std::runtime_error("Fail to map file");
    }
    //todo: ну тут вообще плохо
    std::wcsncpy(reinterpret_cast<wchar_t*>(mappedFilePtr), reinterpret_cast<wchar_t*>(source.GetAddr()) + offset / 2, taskSize / 2);
    reinterpret_cast<wchar_t*>(mappedFilePtr)[taskSize / 2] = L'\0';
    offset += taskSize;
    return { 0, mappingHandle, mappedFilePtr, taskSize + 2 };
}

std::vector<CWorker> InitWorkers(size_t numberOfWorkers, const std::wstring &dictionaryPath, const CMappedFile &source) 
{
    std::vector<CWorker> workers;
    if (numberOfWorkers < 1) {
        return workers;
    }

    std::wstring workerPath(L"Worker.exe");
    HANDLE terminationEvent = CreateTerminationEvent();
    // Смещение в исхожном файле, изменяется в процессе раздачи заданий
    size_t offset = 0;
    
    for (size_t i = 0; i < numberOfWorkers; ++i) {
        auto taskSize = GetTaskSize(i, source, offset);
        std::wstring arguments = workerPath.append(L" ")
            .append(dictionaryPath).append(L" ")
            .append(std::to_wstring(reinterpret_cast<int> (terminationEvent))).append(L" ");

        auto cArguments = std::make_unique<wchar_t[]>(arguments.length() + 1);
        wcscpy(cArguments.get(), arguments.c_str());
        
        PROCESS_INFORMATION processInfo;
        STARTUPINFO startUpInfo = { sizeof(startUpInfo) };
        if (CreateProcess(0, cArguments.get(), 0, 0, TRUE, 0, 0, 0, &startUpInfo, &processInfo) != 0) {
            CloseHandle(processInfo.hThread);
            auto sharedPiece = PrepareSharedMem(processInfo.dwProcessId, i, source, offset);
            workers.push_back(CWorker(processInfo, terminationEvent, std::move(sharedPiece)));
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

void WaitForResults(const std::vector<CWorker> &workers, CMappedFile &output)
{
    auto awaitedEvents = std::make_unique<HANDLE[]>(workers.size());
    size_t offset = 0;
    for (size_t i = 0; i < workers.size(); ++i) {
        awaitedEvents.get()[i] = workers[i].OnWorkIsDoneEvent();
    }
    DWORD waitResult = WaitForMultipleObjects(workers.size(), awaitedEvents.get(), TRUE, INFINITE);
    assert(waitResult != WAIT_FAILED && waitResult != WAIT_TIMEOUT);
    for (size_t i = 0; i < workers.size(); ++i) {
        auto result = workers[i].GetResult();
        std::wcscpy(reinterpret_cast<wchar_t*>(output.GetAddr()) + offset, result.c_str());
        offset += result.length();
    }
}

CMappedFile OpenAndMapFile(const std::wstring &filePath)
{
    HANDLE fileHandle = CreateFileW(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Can't open file. Error code");
    }
    HANDLE mappingHandle = CreateFileMapping(fileHandle, 0, PAGE_READONLY, 0, 0, 0);
    if (mappingHandle == 0) {
        throw std::runtime_error("Fail to create file mapping");
    }

    PVOID mappedFilePtr = MapViewOfFile(mappingHandle, FILE_MAP_READ, 0, 0, 0);
    if (mappedFilePtr == 0) {
        throw std::runtime_error("Fail to map file");
    }
    DWORD fileSize = GetFileSize(fileHandle, &fileSize);
    return {fileHandle, mappingHandle, mappedFilePtr, fileSize};
}

// da prostit me DRY(
CMappedFile OpenAndMapOutputFile(const std::wstring &filePath, size_t size)
{
    HANDLE fileHandle = CreateFileW(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (fileHandle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Can't open file");
    }
    HANDLE mappingHandle = CreateFileMapping(fileHandle, 0, PAGE_READWRITE, 0, size, 0);
    if (mappingHandle == 0) {
        throw std::runtime_error("Fail to create file mapping");
    }

    PVOID mappedFilePtr = MapViewOfFile(mappingHandle, FILE_MAP_WRITE, 0, 0, 0);
    if (mappedFilePtr == 0) {
        throw std::runtime_error("Fail to map file");
    }
    DWORD fileSize = GetFileSize(fileHandle, &fileSize);
    return { fileHandle, mappingHandle, mappedFilePtr, fileSize };
}

int main(int argc, char* argv[]) {
    try {
        std::wstring sourceFilePath;
        auto mappedSource = OpenAndMapFile(GetWStringFromArguments(2, "Source file path"));
        auto workers = InitWorkers(NumberOfProcesses, GetArgument(1, "Dictionary file path"), mappedSource);
        for (CWorker &worker : workers) {
            worker.Notify();
        }
        auto mappedOutput = OpenAndMapOutputFile(GetWStringFromArguments(3, "Target file path"), mappedSource.GetSize());
        WaitForResults(workers, mappedOutput);
        SetEvent(workers[0].GetTerminationEvent());
        JoinWorkers(workers);
        return 0;
    } catch (std::exception e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}