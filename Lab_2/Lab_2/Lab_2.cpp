#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <iostream>

//  Forward declarations:
BOOL GetProcessList();
BOOL ListProcessModules(DWORD dwPID);
BOOL ListProcessThreads(DWORD dwOwnerPID);
void printError(TCHAR* msg);
using namespace std;

int main(void)
{
    GetProcessList();
    return 0;
}

BOOL GetProcessList()
{
    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;
    // Делает снимок всех процессов в системе.

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        cout << "CreateToolhelp32Snapshot (of processes)";
        // printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
        return(FALSE);
    }

    // Устанавливаем размер конструкции перед ее использованием.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Получить информацию о первом процессе,
     // и выходим в случае неудачи
    if (!Process32First(hProcessSnap, &pe32))
    {
        cout << "Process32First";
        // printError(TEXT("Process32First")); // show cause of failure
        CloseHandle(hProcessSnap);          // очищаем объект снимка
        return(FALSE);
    }

    // Теперь пройдемся по снимку процессов и
     // выводим информацию о каждом процессе по очереди
    do
    {
        _tprintf(TEXT("\n\n====================================================="));
        _tprintf(TEXT("\nPROCESS NAME:  %s"), pe32.szExeFile);
        _tprintf(TEXT("\n-------------------------------------------------------"));

        // Получить класс приоритета.
        dwPriorityClass = 0;
        hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
        if (hProcess == NULL)
            cout << "OpenProcess";
        //  printError(TEXT("OpenProcess"));
        else
        {
            dwPriorityClass = GetPriorityClass(hProcess);
            if (!dwPriorityClass)
                cout << "GetPriorityClass";
            //printError(TEXT("GetPriorityClass"));
            CloseHandle(hProcess);
        }

        _tprintf(TEXT("\n  Process ID        = 0x%08X"), pe32.th32ProcessID); 
        _tprintf(TEXT("\n  Thread count      = %d"), pe32.cntThreads);
        _tprintf(TEXT("\n  Parent process ID = 0x%08X"), pe32.th32ParentProcessID);
        _tprintf(TEXT("\n  Priority base     = %d"), pe32.pcPriClassBase);
        if (dwPriorityClass)
            _tprintf(TEXT("\n  Priority class    = %d"), dwPriorityClass);

        // Список модулей и потоков, связанных с этим процессом
        ListProcessModules(pe32.th32ProcessID);
        ListProcessThreads(pe32.th32ProcessID);

    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return(TRUE);
}


BOOL ListProcessModules(DWORD dwPID)
{
    HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
    MODULEENTRY32 me32;

    // Сделаем снимок всех модулей в указанном процессе.
    hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
    if (hModuleSnap == INVALID_HANDLE_VALUE)
    {
        cout << "CreateToolhelp32Snapshot (of modules)";
        // printError(TEXT("CreateToolhelp32Snapshot (of modules)"));
        return(FALSE);
    }

    // Устанавливаем размер конструкции перед ее использованием.
    me32.dwSize = sizeof(MODULEENTRY32);

    // Получить информацию о первом модуле,
     // и выходим в случае неудачи
    if (!Module32First(hModuleSnap, &me32))
    {
        cout << "Module32First";
        //   printError(TEXT("Module32First"));  // show cause of failure
        CloseHandle(hModuleSnap);         // очищаем объект снимка
        return(FALSE);
    }

    // Теперь пройдемся по списку модулей процесса,
     // и отображаем информацию о каждом модуле
    do
    {
        _tprintf(TEXT("\n\n     MODULE NAME:     %s"), me32.szModule);
        _tprintf(TEXT("\n     Executable     = %s"), me32.szExePath);
        _tprintf(TEXT("\n     Process ID     = 0x%08X"), me32.th32ProcessID);
        _tprintf(TEXT("\n     Ref count (g)  = 0x%04X"), me32.GlblcntUsage);
        _tprintf(TEXT("\n     Ref count (p)  = 0x%04X"), me32.ProccntUsage);
        _tprintf(TEXT("\n     Base address   = 0x%08X"), (DWORD)me32.modBaseAddr);
        _tprintf(TEXT("\n     Base size      = %d"), me32.modBaseSize);

    } while (Module32Next(hModuleSnap, &me32));

    CloseHandle(hModuleSnap);
    return(TRUE);
}

BOOL ListProcessThreads(DWORD dwOwnerPID)
{
    HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
    THREADENTRY32 te32;

    // Сделаем снимок всех запущенных потоков
    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE)
        return(FALSE);

    // Заполняем размер структуры перед ее использованием.
    te32.dwSize = sizeof(THREADENTRY32);

    // Получить информацию о первом потоке,
       // и выходим в случае неудачи
    if (!Thread32First(hThreadSnap, &te32))
    {
        cout << "Thread32First";
        //    printError(TEXT("Thread32First")); // show cause of failure
        CloseHandle(hThreadSnap);          // clean the snapshot object
        return(FALSE);
    }

    // Теперь пройдемся по списку потоков системы,
     // и отображаем информацию о каждом потоке
     // связан с указанным процессом
    do
    {
        if (te32.th32OwnerProcessID == dwOwnerPID)
        {
            _tprintf(TEXT("\n\n     THREAD ID      = 0x%08X"), te32.th32ThreadID);
            _tprintf(TEXT("\n     Base priority  = %d"), te32.tpBasePri);
            _tprintf(TEXT("\n     Delta priority = %d"), te32.tpDeltaPri);
            _tprintf(TEXT("\n"));
        }
    } while (Thread32Next(hThreadSnap, &te32));

    CloseHandle(hThreadSnap);
    return(TRUE);
}

void printError(TCHAR* msg)
{
    DWORD eNum;
    TCHAR sysMsg[256];
    TCHAR* p;

    eNum = GetLastError();
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, eNum,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        sysMsg, 256, NULL);

    // Обрезать конец строки и завершить его нулем
    p = sysMsg;
    while ((*p > 31) || (*p == 9))
        ++p;
    do { *p-- = 0; } while ((p >= sysMsg) &&
        ((*p == '.') || (*p < 33)));

    // Display the message
    _tprintf(TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg);
}