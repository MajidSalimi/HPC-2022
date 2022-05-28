#ifndef UTILS_H
#define	UTILS_H

#include <string>
#include <iostream>
#include <fstream>
#include <mpi.h>

double getTime() {
  return MPI_Wtime();
}

void addToFile(std::string logLine) {
  std::ofstream outputFile;
  outputFile.open("o.txt", std::ios_base::app);
  outputFile << logLine;
  outputFile.close();
}

void printDiffTime(std::string name, double startTime) {
  auto now = getTime();
  auto duration = now - startTime;
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank != 0) return;
  std::string s = ">> " + name + " takes " + std::to_string(duration) + " seconds | start: " + std::to_string(startTime) + " end: " + std::to_string(now) + "\n";
  addToFile(s);
}

void log(std::string logLine) {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank != 0) return;
  std::cout << logLine;
  addToFile(logLine);
}

// class Timer {
//     double startTime;
//     std::string name;

//     public:
//         Timer(std::string name) {
//             this.name = name;
//             startTime = MPI_Wtime();
//         }

//         void printExecutionTime() {
//             auto duration = startTime - MPI_Wtime();
//             std::cout << name << " takes " << duration << " seconds" << std::endl;
//         }
// }

#endif // UTILS_H