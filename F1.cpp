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
pthread_barrier_t lap_barrier;

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
    float pitstopTime = 0.0;

    while (viewCar-> lapCount < 21) {      
        

        // Simulate pitstop
        if (viewCar -> lapsBeforePitstop == 0)
        {
            // Pitstop for users car
            if (viewCar -> carName == "Car Player")
            {
                // Lock to ensure only user's car accesses this section
                pthread_mutex_lock(&console_mutex);
                // Request tire type from user
                viewCar->tireType = getTireTypeFromUser();
                pitstopTime = (randomNumber(5, 16) / 3600.0);
                cout << viewCar->carName << " had a pitstop of " << pitstopTime * 3600 << " seconds, at Lap: " << viewCar -> lapCount << endl;  // Print pitstop time
                // Unlock to allow other cars to proceed
                pthread_mutex_unlock(&console_mutex);
            }
            // Pitstops for the ohter cars
            else
            {
                viewCar -> tireType = randomNumber(0, 2);
                pitstopTime = (randomNumber(5, 16) / 3600.0);
                pthread_mutex_lock(&console_mutex);
                cout << viewCar->carName << " had a pitstop of " << pitstopTime * 3600 << " seconds, at Lap: " << viewCar -> lapCount << endl;  // Print pitstop time
                pthread_mutex_unlock(&console_mutex);
            }

            viewCar -> lapsBeforePitstop = viewCar->tireType == 0 ? 7 : (viewCar->tireType == 1 ? 11 : 14);
            viewCar->currentSpeed = viewCar->originalSpeed * (viewCar->tireType == 0 ? 1.5 : (viewCar->tireType == 1 ? 1 : 0.8));
        }
        // Simulte pitstop time penalty
        
        viewCar->totalTime += pitstopTime;

        // Simulate lap time
        viewCar->lapTime = 4.7 / viewCar->currentSpeed;
        viewCar->totalTime += viewCar->lapTime;
        viewCar -> lapCount++;
        pthread_barrier_wait(&lap_barrier);
        
        // Introduce delay using Car Player as the "master" thread
        if (viewCar->carName == "Car Player") {
            sleep(1);  // Delay of 1 second
        }

        pthread_mutex_lock(&console_mutex);
        cout << viewCar->carName << ", Lap: " << viewCar -> lapCount << ", Lap Time: " << viewCar->lapTime * 3600 << " seconds" << endl;
        pthread_mutex_unlock(&console_mutex);

        // Decrement laps before pitstop and check if pitstop is required
        viewCar -> lapsBeforePitstop--;
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
        cout << i + 1 << ". " << cars[i].carName << ", Total Time: " << cars[i].totalTime * 3600 << " seconds" << endl;
    }
}

int main(int argc, char const *argv[])
{
    //Initialize seed
    srand(time(0));

    //Initialize arrray cars
    Car cars[8];

    //Initialize mutex
    pthread_mutex_init(&console_mutex, NULL);

    //Initialize barrier
    pthread_barrier_init(&lap_barrier, NULL, 8);

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
    pthread_mutex_destroy(&console_mutex);
    pthread_barrier_destroy(&lap_barrier);

    return 0;
}