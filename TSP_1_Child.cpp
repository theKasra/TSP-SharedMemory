#include <iostream>
#include <Windows.h>
#include <vector>
#include <time.h>
#include <thread>


struct PathInfo {
    std::vector<int> path;
    int cost;
};

void TSP_RandomPaths(int*, int*, int*, int*);
int FindCost(int*, int, int, int);
PathInfo FindBestAnswer(std::vector<PathInfo>);

int main()
{
    TCHAR InputParamsName[] = TEXT("params");
    TCHAR FileInputName[] = TEXT("input");
    TCHAR PathsName[] = TEXT("paths");
    TCHAR CostsName[] = TEXT("costs");
    TCHAR StartSwitchName[] = TEXT("start");

    HANDLE hInputParams = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, InputParamsName);
    HANDLE hFileInput = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, FileInputName);
    HANDLE hPaths = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, PathsName);
    HANDLE hCosts = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, CostsName);
    HANDLE hStartSwitch = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, StartSwitchName);

    int* InputParamsBuf = (int*)MapViewOfFile(
        hInputParams,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        3 * sizeof(int));
    int* FileInputBuf = (int*)MapViewOfFile(
        hFileInput,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        (InputParamsBuf[2] * InputParamsBuf[2]) * sizeof(int));
    int* PathsBuf = (int*)MapViewOfFile(
        hPaths,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        InputParamsBuf[1] * (InputParamsBuf[2] + 1) * sizeof(int));
    int* CostsBuf = (int*)MapViewOfFile(
        hCosts,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        InputParamsBuf[1] * sizeof(int));
    bool* StartSwitchBuf = (bool*)MapViewOfFile(
        hStartSwitch,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        sizeof(bool));
    
    while (!StartSwitchBuf[0]);
    TSP_RandomPaths(InputParamsBuf, FileInputBuf, PathsBuf, CostsBuf);

    UnmapViewOfFile(InputParamsBuf);
    UnmapViewOfFile(FileInputBuf);
    UnmapViewOfFile(PathsBuf);
    UnmapViewOfFile(CostsBuf);

    CloseHandle(hInputParams);
    CloseHandle(hFileInput);
    CloseHandle(hPaths);
    CloseHandle(hCosts);

    return 0;
}

void TSP_RandomPaths(int* InputParamsBuf, int* FileInputBuf, int* PathsBuf, int* CostsBuf)
{
    int process_id = GetCurrentProcessId();

    int cities = InputParamsBuf[2];

    int curr_city = 0;
    int curr_cost = 0;
    int travel_to;

    int rand_min = 1;
    int rand_max = cities - 1;

    std::vector<int> visited;
    std::vector<PathInfo> temp_answers;

    visited.push_back(curr_city);

    srand(time(NULL) + process_id);
    int Time = time(NULL);

    while (time(NULL) - Time != InputParamsBuf[0])
    {
        PathInfo answer;
        
        // continue making cycles or stop
        if (visited.size() < cities)
            travel_to = ((rand() % rand_max - rand_min + 1) + rand_min);
        else
        {
            travel_to = 0;
            curr_cost += FindCost(FileInputBuf, curr_city, travel_to, cities);
            visited.push_back(travel_to);
            curr_city = travel_to;
        }

        // avoid revisiting cities
        if (std::find(visited.begin(), visited.end(), travel_to) != visited.end() && visited.size() < cities)
            continue;

        // found a hamiltonian cycle
        if (curr_cost != 0 && visited.size() == cities + 1)
        {
            answer.path = visited;
            answer.cost = curr_cost;
            temp_answers.push_back(answer);

            // prepare for finding another path
            visited.clear();
            visited.push_back(0);
            curr_cost = 0;
            continue;
        }

        curr_cost += FindCost(FileInputBuf, curr_city, travel_to, cities);
        visited.push_back(travel_to);
        curr_city = travel_to;
    }

    PathInfo best_temp = FindBestAnswer(temp_answers);

    for (int i = 0; i < InputParamsBuf[1] * (cities + 1); i++)
    {
        if (PathsBuf[i] == process_id)
        {
            for (int j = 0; j < best_temp.path.size(); j++)
            {
                PathsBuf[i + j] = best_temp.path[j];
            }
        }

        if (CostsBuf[i / (cities + 1)] == process_id)
        {
            CostsBuf[i / (cities + 1)] = best_temp.cost;
        }
    }
}
int FindCost(int* FileInputBuf, int curr_city, int travel_to, int cities)
{
    int target = (curr_city * cities) + travel_to;
    return FileInputBuf[target];
}
PathInfo FindBestAnswer(std::vector<PathInfo> answers)
{
    PathInfo best_answer = answers[0];
    for (int i = 0; i < answers.size(); i++)
    {
        if (answers[i].cost < best_answer.cost)
        {
            best_answer = answers[i];
        }
    }
    return best_answer;
}