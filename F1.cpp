#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <string>
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
    int tireType; // 0: suave, 1: medio, 2: duro
    int lapCount;
    int lapsBeforePitstop;
    float lapTime;
    float totalTime;
};

// Generar numero aleatorio
int randomNumber(int minValue, int maxValue){
    return rand() % maxValue + minValue;
}

// Obtener el tipo de llanta que el usuario desea usar en la pitstop
int getTireTypeFromUser() {
    int userInput;
    do {
        cout << "PITSTOP" << endl;
        cout << "Elige tipo de llanta (0: suave, 1: medio, 2: duro): " << endl;
        cin >> userInput;
        cin.ignore(10000, '\n');
        if (cin.fail() || userInput < 0 || userInput > 2) {
            cout << "Entrada no valida. Ingresa un numero entre 0 y 2." << endl;
            cin.clear(); // Limpiar el error
        }
    } while (userInput < 0 || userInput > 2);
    return userInput;
}

// Simulacion de carrera de F1
void* Race(void *car){
    Car * viewCar = (Car *)car;
    float pitstopTime = 0.0;

    while (viewCar->lapCount < 21) {
        // Simulacion de una pitstop      
        if (viewCar->lapsBeforePitstop == 0) {
            // Pitstop para el usuario si el carro es Car player, sino carro IA
            if (viewCar->carName == "Carro Jugador") {
                pthread_mutex_lock(&console_mutex);
                viewCar->tireType = getTireTypeFromUser();
                pthread_mutex_unlock(&console_mutex);
            } else {
                viewCar->tireType = randomNumber(0, 2);
            }
            // Tiempo que se toma la pitstop
            pitstopTime = (randomNumber(5, 16) / 3600.0);
            viewCar -> lapsBeforePitstop = viewCar->tireType == 0 ? 7 : (viewCar->tireType == 1 ? 11 : 14);
            viewCar -> currentSpeed = viewCar->originalSpeed * (viewCar->tireType == 0 ? 1.5 : (viewCar->tireType == 1 ? 1 : 0.8));
            pthread_mutex_lock(&console_mutex);
            cout << viewCar->carName << " hizo un pitstop de " << pitstopTime * 3600 << " segundos, en la vuelta: " << viewCar->lapCount << endl;
            pthread_mutex_unlock(&console_mutex);
        }

        viewCar->totalTime += pitstopTime;
        viewCar->lapTime = 4.7 / viewCar->currentSpeed;
        viewCar->totalTime += viewCar->lapTime;
        viewCar->lapCount++;

        pthread_barrier_wait(&lap_barrier);

        // Simula un delay entre cada vuelta
        if (viewCar->carName == "Carro Jugador") {
            sleep(1); // espera de 1 segundo
        }
        // Impresion final de resultados para la vuelta "n"
        pthread_mutex_lock(&console_mutex);
        cout << viewCar->carName << ", Vuelta: " << viewCar->lapCount << ", Tiempo de Vuelta: " << viewCar->lapTime * 3600 << " segundos" << endl;
        pthread_mutex_unlock(&console_mutex);

        viewCar->lapsBeforePitstop--; // decremento en la cantidad de vueltas antes de la siguiente pitstop
    }
}

// Creacion de los carros en la carrera
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

// Ordenamiento e impresion de resultados finales basados en tiempos totales
void displayFinalPositions(Car cars[]) {
    // Ordenamiento de carros basado en su tiempo total
    sort(cars, cars + 8, [](const Car& a, const Car& b) {
        return a.totalTime < b.totalTime;
    });
    // Mostrar en pantalla de forma secuencial
    cout << "\nPosiciones finales:" << endl;
    for (int i = 0; i < 8; i++) {
        cout << i + 1 << ". " << cars[i].carName << ", Tiempo Total: " << cars[i].totalTime * 3600 << " segundos" << endl;
    }
    cout << "" << endl;
}

int main(int argc, char const *argv[])
{
    // Inicializacion de semilla random
    srand(time(0));
    // Inicializacion de carros
    Car cars[8];
    // Inicializacion de mutex y barrera
    pthread_mutex_init(&console_mutex, NULL);
    pthread_barrier_init(&lap_barrier, NULL, 8);

    // Creacion de carros
    for (int i = 0; i < 8; i++) {
        if(i != 7){
            createCar((void *)&cars[i], to_string(i+1), false);
        } else {
            createCar((void *)&cars[i], "Jugador", true);
        }
    }
    
    // Creacion de hilos por carro
    for (int i = 0; i < 8; i++) {
        if(pthread_create(&cars[i].thread, NULL, Race, (void *)&cars[i])){
            cout << "No se puede crear el hilo: carro" << endl;
            return 1;
        }
    }

    // Union de hilos para resultados finales
    for (int i = 0; i < 8; i++) {
        if(pthread_join(cars[i].thread, NULL)){
            cout << "No se puede unir el hilo: carro" << endl;
            return 1;
        }
    }

    // resultados finales
    displayFinalPositions(cars);

    // limpieza de mutex y barrera
    pthread_mutex_destroy(&console_mutex);
    pthread_barrier_destroy(&lap_barrier);

    return 0;
}