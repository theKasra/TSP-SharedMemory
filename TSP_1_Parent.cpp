#include <iostream>
#include <Windows.h>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <string>
#include <thread>

void PrintAnswer(int*, int*, int, int);
int GetTotalNumberOfCities(unsigned int);
void ReadInputFile(std::string, int, char, int*);

int main()
{
    const int cores = std::thread::hardware_concurrency();

    STARTUPINFO si;
    PROCESS_INFORMATION* pi = new PROCESS_INFORMATION[cores];

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    for (int i = 0; i < cores; i++)
    {
        ZeroMemory(&pi[i], sizeof(pi[i]));
    }

    unsigned int input_number;
    unsigned int runtime;
    int cities;

    std::cout << "Which input would you like to choose?\n" << "1. input_1.txt - 5 cities\n" << "2. input_2.txt - 100 cities\n\n" << "Enter the number: ";
    std::cin >> input_number;
    std::cout << "\nPlease set the runtime value: ";
    std::cin >> runtime;

    if (input_number == 1 || input_number == 2)
    {
        cities = GetTotalNumberOfCities(input_number);

        TCHAR InputParamsName[] = TEXT("params");
        HANDLE hInputParams = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            3 * sizeof(int),    // 0 = runtime value, 1 = cores, 2 = cities
            InputParamsName);
        int* InputParamsBuf = (int*)MapViewOfFile(
            hInputParams,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            3 * sizeof(int));

        TCHAR FileInputName[] = TEXT("input");
        HANDLE hFileInput = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            (cities * cities) * sizeof(int),
            FileInputName);
        int* FileInputBuf = (int*)MapViewOfFile(
            hFileInput,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            (cities * cities) * sizeof(int));

        TCHAR PathsName[] = TEXT("paths");
        HANDLE hPaths = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            cores * sizeof(int),
            PathsName);
        int* PathsBuf = (int*)MapViewOfFile(
            hPaths,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            cores * (cities + 1) * sizeof(int));

        TCHAR CostsName[] = TEXT("costs");
        HANDLE hCosts = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            cores * sizeof(int),
            CostsName);
        int* CostsBuf = (int*)MapViewOfFile(
            hCosts,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            cores * sizeof(int));

        TCHAR StartSwitchName[] = TEXT("start");
        HANDLE hStartSwitch = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            sizeof(bool),
            StartSwitchName);
        bool* StartSwitchBuf = (bool*)MapViewOfFile(
            hStartSwitch,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            sizeof(bool));

        InputParamsBuf[0] = runtime;
        InputParamsBuf[1] = cores;
        InputParamsBuf[2] = cities;
        StartSwitchBuf[0] = false;

        if (input_number == 1)
        {
            ReadInputFile("input_1.txt", cities * cities, '\t', FileInputBuf);
        }

        else if (input_number == 2)
        {
            ReadInputFile("input_2.txt", cities * cities, ' ', FileInputBuf);
        }

        // -1 => unavailable
        for (int i = 0; i < cores; i++)
            CostsBuf[i] = -1;

        int a = 0;
        int c = 0;
        for (int i = 0; i < cores; i++)
        {
            if (!CreateProcess(
                TEXT("INSERT THE CHILD'S EXE LOCATION"),
                NULL,
                NULL,
                NULL,
                FALSE,
                0,
                NULL,
                NULL,
                &si,
                &pi[i]))
            {
                std::cout << "Error occured during creating process" << std::endl;
            }

            else
            {
                int b = 0;
                for (; a < cores * (cities + 1); a++)
                {
                    if (b == cities + 1) break;

                    PathsBuf[a] = GetProcessId(pi[i].hProcess);

                    if (a % (cities + 1) == 0)
                    {
                        CostsBuf[c] = GetProcessId(pi[i].hProcess);
                        c++;
                    }
                    b++;
                }
            }
        }

        StartSwitchBuf[0] = true;
        
        for (int i = 0; i < cores; i++)
        {
            WaitForSingleObject(pi[i].hProcess, INFINITE);
            CloseHandle(pi[i].hProcess);
            CloseHandle(pi[i].hThread);
        }

        PrintAnswer(CostsBuf, PathsBuf, cores, cities);
        
        UnmapViewOfFile(InputParamsBuf);
        UnmapViewOfFile(FileInputBuf);
        UnmapViewOfFile(PathsBuf);
        UnmapViewOfFile(CostsBuf);

        CloseHandle(hInputParams);
        CloseHandle(hFileInput);
        CloseHandle(hPaths);
        CloseHandle(hCosts);
    }

    else
    {
        std::cout << "Wrong input file number!" << std::endl;
    }

    return 0;
}

int GetTotalNumberOfCities(unsigned int input_number)
{
    std::ifstream f;
    std::string temp;

    if (input_number == 1)
    {
        f.open("input_1.txt");
        if (f.is_open())
        {
            std::getline(f, temp);
            f.close();
            return std::stoi(temp);
        }
        else return -1;
    }

    else if (input_number == 2)
    {
        f.open("input_2.txt");
        if (f.is_open())
        {
            std::getline(f, temp);
            f.close();
            return std::stoi(temp);
        }
        else return -1;
    }

    else return -1;
    
}
void ReadInputFile(std::string file_name, int limit, char split_char, int* FileInputBuff)
{
    std::ifstream f;
    f.open(file_name);

    if (f.is_open())
    {
        // skip the first line
        std::string skip_first;
        getline(f, skip_first);

        std::string str;
        while (!f.eof())
        {
            for (int i = 0; i < limit; i++)
            {
                getline(f, str, split_char);
                FileInputBuff[i] = std::stoi(str);
            }
        }
        f.close();
    }

    else std::cout << "An error occured during reading the file!" << std::endl;
}
void PrintAnswer(int* CostsBuf, int* PathsBuf, int cores, int cities)
{
    int m = 0;
    int final_cost = CostsBuf[0];
    for (int i = 0; i < cores; i++)
    {
        if (CostsBuf[i] < final_cost && CostsBuf[i] != -1)
        {
            final_cost = CostsBuf[i];
            m = i;
        }
    }

    int p = m * (cities + 1);
    std::cout << "\nPath: ";
    for (int j = 0; j < cities + 1; j++)
    {
        std::cout << PathsBuf[p];
        if (j < cities) std::cout << " -> ";
        p++;
    }
    std::cout << "\nCost: " << final_cost << std::endl;
}