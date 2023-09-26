#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include <windows.h>
#include <cstdlib>
#include <ctime>

using namespace std;
struct Car {
    char name[10];
    float currentSpeed;
    float originalSpeed;
    int tireType; // 0: soft, 1: medium, 2: hard
    int lapCount;
    int lapsBeforePitstop;
    float lapTime;
    float totalTime;
};

int main(int argc, char const *argv[])
{
    std::srand(std::time(0));
    std::cout << "Numero aleatorio: " << randomNumber() << std::endl;
    
    std::vector<Car> cars = /* inicializa tu vector de coches aquí */;
    
    std::sort(cars.begin(), cars.end(), compareCars);

    for (const auto& car : cars) {
        std::cout << car.name << ", Total Time: " << car.totalTime << std::endl;
    }

    return 0;
}
int randomNumber(){
    return std::rand() % 100 + 1;
}
bool compareCars(const Car& a, const Car& b) {
    return a.totalTime < b.totalTime;
}

