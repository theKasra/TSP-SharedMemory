# Traveling Salesperson Problem... with an awkward collectivist approach
This piece of C++ code uses Windows API (oh boy) to find a solution for TSP problem by creating multiple processes.  
Each process communicates through... "Shared Memory".  
Each child process generates the best answer within a limited amount of time, then stores it in the... "Shared Memory".  
In the end parent process finds the best solution by accessing... yes.
