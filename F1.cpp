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
    pthread_t thread;
    string carName;
    float currentSpeed;
    float originalSpeed;
    int tireType; // 0: soft, 1: medium, 2: hard
    int lapCount;
    int lapsBeforePitstop;
    float lapTime;
    float totalTime;
};

//Generate random number
int randomNumber(int minValue, int maxValue){
    return rand() % maxValue + minValue;
}

void* Race(void *car){
    Car * viewCar;
    viewCar = (Car *)car;
    cout << "Hola se creo el thread con el carro: " << viewCar -> carName << endl;
}


bool compareCars(const Car& a, const Car& b) {
    return a.totalTime < b.totalTime;
}

int getTireTypeFromUser() {
    int userInput;
    int scanResult;
    do {
        cout << "PITSTOP" << endl;
        cout << "Choose tire type (0: soft, 1: medium, 2: hard): " << endl;
        cin >> userInput;
        cin.ignore(10000, '\n');
        if (cin.fail() || userInput < 0 || userInput > 2) {
            cout << "Invalid input. Please enter a number between 0 and 2." << endl;
        }
    } while (scanResult == 0 || userInput < 0 || userInput > 2);
    return userInput;
}

void createCar(void *car, string carnumber, bool userCar){
    Car * newCar;
    newCar = (Car*)car;
    newCar -> carName = "Car "+(carnumber);
    newCar -> originalSpeed = randomNumber(250,300);
    newCar -> tireType = userCar ? getTireTypeFromUser() : randomNumber(0,2);
    newCar -> lapsBeforePitstop = newCar->tireType == 0 ? 7 :(newCar->tireType == 1 ? 11: 14);
    newCar -> currentSpeed = newCar -> originalSpeed * (newCar -> tireType == 0 ? 1.5 : (newCar -> tireType == 1 ? 1: 0.8));
    newCar -> lapTime = 4.7/newCar -> currentSpeed;
    newCar -> lapCount = 0;
    newCar -> totalTime = 0.0;
}


int main(int argc, char const *argv[])
{
    //Initialize seed
    srand(time(0));

    //Initialize arrray cars
    Car cars[8];

    //Create Car
    for (int i = 0; i < 8; i++)
    {
        if(i != 7){
            createCar((void *)&cars[i], to_string(i+1), false);
        }else{
            createCar((void *)&cars[i], "Player", true);
        }
    }
    
    //Create threads
    for (int i = 0; i < 8; i++)
    {
        if(pthread_create(&cars[i].thread, NULL, Race, (void *)&cars[i])){
            cout << "can't create thread: car" << endl;
            return 1;
        }
    }

    //Join threads
    for (int i = 0; i < 8; i++)
    {
        if(pthread_join(cars[i].thread, NULL)){
            cout << "can't join thread: car" << endl;
            return 1;
        }
    }
    
    
    //Sort cars by Total Time
    /*vector<Car> cars = inicializa tu vector de coches aquí;
    
    sort(cars.begin(), cars.end(), compareCars);

    for (const auto& car : cars) {
        std::cout << car.name << ", Total Time: " << car.totalTime << endl;
    }*/

    return 0;
}


