#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

pthread_mutex_t console_mutex;
pthread_barrier_t start_barrier, lap_barrier;

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

// F1 car race simulation
void* Race(void *car){
    Car * viewCar;
    viewCar = (Car *)car;

    while (viewCar-> lapCount < 21) {      
        // Decrement laps before pitstop and check if pitstop is required
        viewCar -> lapsBeforePitstop--;

        if (viewCar->carName == "Car Player" && viewCar->lapsBeforePitstop == 0) {
            // Lock to ensure only user's car accesses this section
            pthread_mutex_lock(&console_mutex);
        
            // Request tire type from user
            viewCar->tireType = getTireTypeFromUser();
            viewCar->lapsBeforePitstop = viewCar->tireType == 0 ? 7 : (viewCar->tireType == 1 ? 11 : 14);
            viewCar->currentSpeed = viewCar->originalSpeed * (viewCar->tireType == 0 ? 1.5 : (viewCar->tireType == 1 ? 1 : 0.8));
            viewCar->lapTime = 4.7 / viewCar->currentSpeed;

            // Unlock to allow other cars to proceed
            pthread_mutex_unlock(&console_mutex);
        }
        

        if (viewCar -> lapsBeforePitstop == 0 && viewCar->carName != "Car Player") {
            // Simulate pitstop
            Sleep(5000);  // 5 seconds pitstop
            viewCar -> tireType = randomNumber(0, 2);
            viewCar -> lapsBeforePitstop = viewCar->tireType == 0 ? 7 : (viewCar->tireType == 1 ? 11 : 14);
            viewCar -> currentSpeed = viewCar -> originalSpeed * (viewCar -> tireType == 0 ? 1.5 : (viewCar -> tireType == 1 ? 1 : 0.8));
            viewCar -> lapTime = 4.7 / viewCar -> currentSpeed;
            Sleep(1000);  // 1 second delay for tire change
        }
        
        // Simulate lap time
        Sleep(viewCar->lapTime * 100000);
        viewCar->totalTime += viewCar->lapTime;
        viewCar -> lapCount++;
        
        // Print lap time protected with mutex
        
    pthread_mutex_lock(&console_mutex);
    cout << viewCar->carName << ", Lap: " << viewCar -> lapCount << ", Lap Time: " << viewCar->lapTime << endl;
    pthread_mutex_unlock(&console_mutex);
    }
}

bool compareCars(const Car& a, const Car& b) {
    return a.totalTime < b.totalTime;
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

void displayFinalPositions(Car cars[]) {
    // Sort the cars based on totalTime
    sort(cars, cars + 8, compareCars);
    
    cout << "\nFinal positions:" << endl;
    for (int i = 0; i < 8; i++) {
        cout << i + 1 << ". " << cars[i].carName << ", Total Time: " << cars[i].totalTime << endl;
    }
}

int main(int argc, char const *argv[])
{
    //Initialize seed
    srand(time(0));
    
    pthread_barrier_init(&start_barrier, NULL, 8);
    pthread_barrier_init(&lap_barrier, NULL, 8);

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
    
    displayFinalPositions(cars);

    return 0;
}