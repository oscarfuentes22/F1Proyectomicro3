#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <pthread.h>
#include <unistd.h>

using namespace std;

pthread_mutex_t console_mutex;
pthread_barrier_t lap_barrier;
pthread_barrier_t display_barrier;

struct Car {
    pthread_t thread;
    string carName;
    float currentSpeed;
    float originalSpeed;
    int tireType;
    int lapCount;
    int lapsBeforePitstop;
    float lapTime;
    float totalTime;
};

int randomNumber(int minValue, int maxValue) {
    return rand() % maxValue + minValue;
}

int getTireTypeFromUser() {
    int userInput;
    while (true) {
        cout << "PITSTOP JUGADOR" << endl;
        cout << "Elige tipo de llanta (0: suave, 1: medio, 2: duro): " << endl;
        cin >> userInput;
        if (cin.good() && userInput >= 0 && userInput <= 2) {
            cin.ignore(INT_MAX, '\n');
            return userInput;
        } else {
            cout << "Entrada no valida. Ingresa un numero entre 0 y 2." << endl;
            cin.clear();
            cin.ignore(INT_MAX, '\n');
        }
    }
}

void createCar(void *car, string carnumber, bool userCar){
    Car * newCar = (Car*)car;
    newCar->carName = "Carro "+(carnumber);
    newCar->originalSpeed = randomNumber(250,300);
    newCar->tireType = userCar ? getTireTypeFromUser() : randomNumber(0,2);
    newCar->lapsBeforePitstop = newCar->tireType == 0 ? 7 : (newCar->tireType == 1 ? 11 : 14);
    newCar->currentSpeed = newCar->originalSpeed * (newCar->tireType == 0 ? 1.5 : (newCar->tireType == 1 ? 1 : 0.8));
    newCar->lapTime = 4.7/newCar->currentSpeed;
    newCar->lapCount = 0;
    newCar->totalTime = 0.0;
}

void* Race(void *car){
    Car * viewCar = (Car *)car;
    float pitstopTime = 0.0;
    while (viewCar->lapCount < 21) {
        if (viewCar->lapsBeforePitstop == 0) {
            if (viewCar->carName == "Carro Jugador") {
                pthread_mutex_lock(&console_mutex);
                viewCar->tireType = getTireTypeFromUser();
                pthread_mutex_unlock(&console_mutex);
            } else {
                viewCar->tireType = randomNumber(0, 2);
            }
            pitstopTime = (randomNumber(5, 16) / 3600.0);
            viewCar->lapsBeforePitstop = viewCar->tireType == 0 ? 7 : (viewCar->tireType == 1 ? 11 : 14);
            viewCar->currentSpeed = viewCar->originalSpeed * (viewCar->tireType == 0 ? 1.5 : (viewCar->tireType == 1 ? 1 : 0.8));
            pthread_mutex_lock(&console_mutex);
            cout << viewCar->carName << " hizo un pitstop de " << pitstopTime * 3600 << " segundos, en la vuelta: " << viewCar->lapCount << endl;
            pthread_mutex_unlock(&console_mutex);
        }
        viewCar->totalTime += pitstopTime;
        viewCar->lapTime = 4.7 / viewCar->currentSpeed;
        viewCar->totalTime += viewCar->lapTime;
        viewCar->lapCount++;
        viewCar->lapsBeforePitstop--;
        pthread_barrier_wait(&lap_barrier);
        pthread_barrier_wait(&display_barrier);
    }
}

void* displayLapPerformance(void *arg) {
    int currentLapNumber = 0;
    Car* cars = (Car*)arg;
    while(currentLapNumber < 21){
        pthread_barrier_wait(&lap_barrier);
        if (currentLapNumber < 21) {
            sort(cars, cars + 8, [](const Car& a, const Car& b) {
                return a.lapTime < b.lapTime;
            });
            pthread_mutex_lock(&console_mutex);
            cout << "\nRendimiento de la Vuelta " << currentLapNumber+1 << ":" << endl;
            for (int i = 0; i < 8; i++) {
                cout << i + 1 << ". " << cars[i].carName << ", Tiempo de Vuelta: " << cars[i].lapTime * 3600 << " segundos" << endl;
            }
            cout << "" << endl;
            pthread_mutex_unlock(&console_mutex);
        }
        currentLapNumber++;
        pthread_barrier_wait(&display_barrier);
    }
    return NULL;
}

void displayFinalPositions(Car cars[]) {
    sort(cars, cars + 8, [](const Car& a, const Car& b) {
        return a.totalTime < b.totalTime;
    });
    cout << "\nPosiciones finales:" << endl;
    for (int i = 0; i < 8; i++) {
        cout << i + 1 << ". " << cars[i].carName << ", Tiempo Total: " << cars[i].totalTime * 3600 << " segundos" << endl;
    }
    cout << "" << endl;
}

int main(int argc, char const *argv[])
{
    srand(time(0));
    pthread_mutex_init(&console_mutex, NULL);
    pthread_barrier_init(&lap_barrier, NULL, 9);
    pthread_barrier_init(&display_barrier, NULL, 9);
    Car cars[8];
    pthread_mutex_lock(&console_mutex);
    cout << "Empezando la carrera" << endl;
    createCar(&cars[0], "Jugador", true);
    cout << "En sus marcas" << endl;
    sleep(1);
    cout << "Listos" << endl;
    sleep(1);
    cout << "¡FUERA!" << endl;
    pthread_mutex_unlock(&console_mutex);
    for (int i = 1; i < 8; i++) {
        createCar(&cars[i], to_string(i), false);
    }
    for (int i = 0; i < 8; i++) {
        pthread_create(&cars[i].thread, NULL, Race, &cars[i]);
    }
    pthread_t display_thread;
    pthread_create(&display_thread, NULL, displayLapPerformance, cars);
    for (int i = 0; i < 8; i++) {
        pthread_join(cars[i].thread, NULL);
    }
    pthread_join(display_thread, NULL);
    displayFinalPositions(cars);
    pthread_mutex_destroy(&console_mutex);
    pthread_barrier_destroy(&lap_barrier);
    pthread_barrier_destroy(&display_barrier);
    return 0;
}