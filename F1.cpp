#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <pthread.h>
#include <unistd.h>

using namespace std;

// Declaración de mutex y barreras
pthread_mutex_t console_mutex;
pthread_barrier_t lap_barrier;
pthread_barrier_t display_barrier;

// Estructura para representar un carro F1
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


// Función para generar un número aleatorio en un rango dado
int randomNumber(int minValue, int maxValue) {
    return rand() % maxValue + minValue;
}


// Función para obtener el tipo de llanta que el usuario desea usar en la pitstop
int getTireTypeFromUser() {
    int userInput;

    while (true) {
        cout << "PITSTOP JUGADOR" << endl;
        cout << "Elige tipo de llanta (0: suave, 1: medio, 2: duro): " << endl;
        cin >> userInput;

       // Si el tipo de llanta es válido se acepta
        if (cin.good() && userInput >= 0 && userInput <= 2) {
            cin.ignore(INT_MAX, '\n');
            return userInput;
        } else {
            // Caso contrario, se vuelve a solicitar el tipo de llanta
            cout << "Entrada no valida. Ingresa un numero entre 0 y 2." << endl;
            cin.clear();
            cin.ignore(INT_MAX, '\n');
        }
    }
}


// Función para inicializar un carro
void createCar(void *car, string carnumber, bool userCar){
    Car * newCar = (Car*)car;
    newCar->carName = "Carro "+(carnumber);
    newCar->originalSpeed = randomNumber(250,300); // velocidad aleatoria de 250km/h a 300km/h
    newCar->tireType = userCar ? getTireTypeFromUser() : randomNumber(0,2);
    newCar->lapsBeforePitstop = newCar->tireType == 0 ? 7 : (newCar->tireType == 1 ? 11 : 14);
    newCar->currentSpeed = newCar->originalSpeed * (newCar->tireType == 0 ? 1.5 : (newCar->tireType == 1 ? 1 : 0.8));
    newCar->lapTime = 4.7/newCar->currentSpeed; // recorrido de 4.7 km por vuelta
    newCar->lapCount = 0;
    newCar->totalTime = 0.0;
}


// Función que simula una carrera para un carro
void* Race(void *car){
    Car * viewCar = (Car *)car;
    float pitstopTime = 0.0;

    // Bucle principal de la carrera
    while (viewCar->lapCount < 21) {

        // Verificar si se necesita un pitstop
        if (viewCar->lapsBeforePitstop == 0) {
            
            // Si es el carro del jugador, pedir al usuario el tipo de llanta
            if (viewCar->carName == "Carro Jugador") {
                pthread_mutex_lock(&console_mutex);
                viewCar->tireType = getTireTypeFromUser();
                pthread_mutex_unlock(&console_mutex);
            } else {
                // Para otros carros, asignar un tipo de llanta aleatoriamente
                viewCar->tireType = randomNumber(0, 2);
            }

            // Calcular el tiempo que toma el pitstop
            pitstopTime = (randomNumber(5, 16) / 3600.0);
            // Resetear las vueltas antes del próximo pitstop según el tipo de llanta nuevo
            viewCar->lapsBeforePitstop = viewCar->tireType == 0 ? 7 : (viewCar->tireType == 1 ? 11 : 14);
            // Ajustar la velocidad actual según el tipo de llanta
            viewCar->currentSpeed = viewCar->originalSpeed * (viewCar->tireType == 0 ? 1.5 : (viewCar->tireType == 1 ? 1 : 0.8));

            // Mostrar el tiempo de pitstop
            pthread_mutex_lock(&console_mutex);
            cout << viewCar->carName << " hizo un pitstop de " << pitstopTime * 3600 << " segundos, en la vuelta: " << viewCar->lapCount << endl;
            pthread_mutex_unlock(&console_mutex);
        }

        // Actualizar tiempos y vueltas
        viewCar->totalTime += pitstopTime;
        viewCar->lapTime = 4.7 / viewCar->currentSpeed;
        viewCar->totalTime += viewCar->lapTime;
        viewCar->lapCount++;
        viewCar->lapsBeforePitstop--;

        // Sincronización de hilos para mostrar el rendimiento de la vuelta
        pthread_barrier_wait(&lap_barrier);
        pthread_barrier_wait(&display_barrier);
    }
    return NULL;
}


// Función que muestra el rendimiento de la vuelta para todos los carros
void* displayLapPerformance(void *arg) {
    int currentLapNumber = 0;
    Car* cars = (Car*)arg;

    // Bucle para mostrar rendimiento después de cada vuelta
    while(currentLapNumber < 21){
        
        // Sincronizar con los hilos de los carros
        pthread_barrier_wait(&lap_barrier);

        if (currentLapNumber < 21) {
            // Ordenar los carros por tiempo de vuelta
            sort(cars, cars + 8, [](const Car& a, const Car& b) {
                return a.lapTime < b.lapTime;
            });

            // Mostrar el rendimiento de la vuelta de los carros
            pthread_mutex_lock(&console_mutex);
            cout << "\nRendimiento de la Vuelta " << currentLapNumber + 1 << ":" << endl;
            for (int i = 0; i < 8; i++) {
                cout << i + 1 << ". " << cars[i].carName << ", Tiempo de Vuelta: " << cars[i].lapTime * 3600 << " segundos" << endl;
            }
            cout << "" << endl;
            pthread_mutex_unlock(&console_mutex);
        }

        currentLapNumber++;

        // Sincronizar nuevamente con los hilos de los carros
        pthread_barrier_wait(&display_barrier);
    }
    return NULL;
}


// Función que muestra las posiciones finales
void displayFinalPositions(Car cars[]) {
    // Ordenar los carros por tiempo de vuelta
    sort(cars, cars + 8, [](const Car& a, const Car& b) {
        return a.totalTime < b.totalTime;
    });

    // Mostrar en pantalla las posiciones finales
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

    // Mensajes de inicio de la carrera
    pthread_mutex_lock(&console_mutex);
    cout << "Empezando la carrera" << endl;
    createCar(&cars[0], "Jugador", true);
    cout << "En sus marcas" << endl;
    sleep(1);
    cout << "Listos" << endl;
    sleep(1);
    cout << "¡FUERA!" << endl;
    pthread_mutex_unlock(&console_mutex);

    // Creación de carros
    for (int i = 1; i < 8; i++) {
        createCar(&cars[i], to_string(i+1), false);
    }

    // Creación de hilo para rendimiento de carros
    pthread_t displayThread;
    pthread_create(&displayThread, NULL, displayLapPerformance, cars);

    // Creación de hilos para los carros
    for (int i = 0; i < 8; i++) {
        pthread_create(&cars[i].thread, NULL, Race, &cars[i]);
    }

    // Unión de hilos para resultados finales
    for (int i = 0; i < 8; i++) {
        pthread_join(cars[i].thread, NULL);
    }
    pthread_join(displayThread, NULL);

    // Mostrar las posiciones finales
    displayFinalPositions(cars);

    // Limpieza de recursos
    pthread_mutex_destroy(&console_mutex);
    pthread_barrier_destroy(&lap_barrier);
    pthread_barrier_destroy(&display_barrier);

    return 0;
}