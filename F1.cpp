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
        cout << R"(
                      00  00  00       _____  _  _     _____  _                     00  00  00      
                        00  00  00     |  __ \(_)| |   / ____|| |                     00  00  00    
                      00  00  00       | |__) |_ | |_ | (___  | |_  ___   _ __      00  00  00       
                        00  00  00     |  ___/| || __| \___ \ | __|/ _ \ | '_ \       00  00  00    
                      00  00  00       | |    | || |_  ____) || |_| (_) || |_) |    00  00  00      
                        00  00  00     |_|    |_| \__||_____/  \__|\___/ | .__/       00  00  00    
                      00  00  00                                       | |          00  00  00      
                        00  00  00                                     |_|            00  00  00    
        )" << '\n';   
        
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

        sleep(1);

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
    cout << R"(
             /$$$$$$$                                /$$   /$$                     /$$                     /$$ /$$
            | $$__  $$                              | $$  | $$                    | $$                    | $$| $$
            | $$  \ $$  /$$$$$$   /$$$$$$$ /$$   /$$| $$ /$$$$$$    /$$$$$$   /$$$$$$$  /$$$$$$   /$$$$$$$| $$| $$
            | $$$$$$$/ /$$__  $$ /$$_____/| $$  | $$| $$|_  $$_/   |____  $$ /$$__  $$ /$$__  $$ /$$_____/| $$| $$
            | $$__  $$| $$$$$$$$|  $$$$$$ | $$  | $$| $$  | $$      /$$$$$$$| $$  | $$| $$  \ $$|  $$$$$$ |__/|__/
            | $$  \ $$| $$_____/ \____  $$| $$  | $$| $$  | $$ /$$ /$$__  $$| $$  | $$| $$  | $$ \____  $$        
            | $$  | $$|  $$$$$$$ /$$$$$$$/|  $$$$$$/| $$  |  $$$$/|  $$$$$$$|  $$$$$$$|  $$$$$$/ /$$$$$$$/ /$$ /$$
            |__/  |__/ \_______/|_______/  \______/ |__/   \___/   \_______/ \_______/ \______/ |_______/ |__/|__/                                                                              
    )"<< '\n';

    sleep(2);

    cout << R"(                                                   
                                                            11111111111131                                                   
                                                        177133333333333252771                                                
                                                        77  77111111111127 77                                                
                                                         3  7711111111112  3                                                 
                                                          1 7711111111112 1                                                  
                                                           11 711111111323                                                   
                              711111111111357                77111111112                                                     
                           1    711111111132  1                71111131                                                      
                           1     11111111132  3                  1131                                                        
                            3    11111111132 17                  1137                                                        
                            717  1111111112217                   731                                                         
                              1  71111111152                     1137                                                        
                                 7111111121                    7111133               3888888888888888808883                  
                                   111132                  800000000000000           38  44699999999989  93                  
                                    133                    0071111111111007           93 44699999999989 26                   
                                    133                   10071111111111003           78144699999999889187                   
                                    121              777773005555555555500277777        8846999999998888                     
                                   71127            77777715444666666664451777777        16499999999801                      
                                 71111122         777777777777777777777777777777777        4699999989                        
                             2064444444444605    11111111111111111111111111111111111         2998847                         
                             20 1111111111205    11111111111111111111111111111111111          9988                           
                             20 1111111111204    11111111111111111111111111111111111          3882                           
                             2000000000000004    11111111111111111111111111111111111          2985                           
                     111111111111111111111111111111111111111111111111111111111111111    2066666666666802                     
                    111111111111111111111111111111111111111111117   1111111111111111    4069999999999804                 
                    1111111111111111111111111111111111111111111117  1111111111111111    8068888888888808                
                    1111111111111111111111111111111111111111111117  1111111111111111    8000000000000008             
                    111111111111117    71111111111111111111111117   711111111111111166666666666666666666666666661            
                    11111111111111117  711111111111111111111111111111111111111111111999999999999999999999999999995           
                    111111111111111   1111111111111111111111111111111111111111111111999999999999996699999999999995           
                    11111111111111      1111111111111111111111111111111111111111111199999999999      6999999999995           
                    111111111111111111111111111111111111111111111111111111111111111199999999999393  99999999999995           
                    111111111111111111111111111111111111111111111111111111111111111199999999999393   9599999999995           
                    111111111111111111111111111111111111111111111111111111111111111199999999999      9999999999995           
                    1111111111111111111111111111111111111111111111111111111111111111999999999999999999999999999995           
    )"<< '\n';

    sleep(1);

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
    cout << R"(
                            6000000000000000000000000000000000000000000000000000000000000000                                                            
                                 0000000000000000000000000000000000000000000000000000000000                                                             
                     0000000000000000000000000000000000000000000000000000000000000000000000                                                             
                                  8000000000000000000000000000000000000000000000000000000000000000000000000000000000000                                 
                         00000000000007                                                                           6000                                  
                            0000000007                                                                            0000                                  
                     1000000000000000         00000000                       0000                                 000                                   
                             7000000        0000  0000   0000000   00000008  000 00000000   000000009            000                                    
                        000000000004       00000000009   7   000700004  999 0000 000  0000 0000  000            9000                                    
            00000000000000000000000        00000000    000000000 0000       000 0000  000 0000  0000            000                                     
                          00000000        0000420000  000000000  000000000 0000 000  0000 000000000            0006                                     
                   555555000000000                                                          7  0007            000                                      
                  111900000000000                                                         0000009             0007                                      
                          5000000                                                                           00007                                       
                          0000000000000000000000000000000000000000000000000000000000000000000000000000000000000                                         
              0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001                                            
    )"<< '\n';

    sleep(2);

    createCar(&cars[0], "Jugador", true);

    cout << R"(
     /$$$$$$$$                                                                                                                     /$$
    | $$_____/                                                                                                                    | $$
    | $$       /$$$$$$$         /$$$$$$$ /$$   /$$  /$$$$$$$       /$$$$$$/$$$$   /$$$$$$   /$$$$$$   /$$$$$$$  /$$$$$$   /$$$$$$$| $$
    | $$$$$   | $$__  $$       /$$_____/| $$  | $$ /$$_____/      | $$_  $$_  $$ |____  $$ /$$__  $$ /$$_____/ |____  $$ /$$_____/| $$
    | $$__/   | $$  \ $$      |  $$$$$$ | $$  | $$|  $$$$$$       | $$ \ $$ \ $$  /$$$$$$$| $$  \__/| $$        /$$$$$$$|  $$$$$$ |__/
    | $$      | $$  | $$       \____  $$| $$  | $$ \____  $$      | $$ | $$ | $$ /$$__  $$| $$      | $$       /$$__  $$ \____  $$    
    | $$$$$$$$| $$  | $$       /$$$$$$$/|  $$$$$$/ /$$$$$$$/      | $$ | $$ | $$|  $$$$$$$| $$      |  $$$$$$$|  $$$$$$$ /$$$$$$$/ /$$
    |________/|__/  |__/      |_______/  \______/ |_______/       |__/ |__/ |__/ \_______/|__/       \_______/ \_______/|_______/ |__/
    )"<< '\n';

    sleep(2);

    cout << R"(
                                          /$$       /$$             /$$                        
                                         | $$      |__/            | $$                        
                                         | $$       /$$  /$$$$$$$ /$$$$$$    /$$$$$$   /$$$$$$$
                                         | $$      | $$ /$$_____/|_  $$_/   /$$__  $$ /$$_____/
                                         | $$      | $$|  $$$$$$   | $$    | $$  \ $$|  $$$$$$ 
                                         | $$      | $$ \____  $$  | $$ /$$| $$  | $$ \____  $$
                                         | $$$$$$$$| $$ /$$$$$$$/  |  $$$$/|  $$$$$$/ /$$$$$$$/
                                         |________/|__/|_______/    \___/   \______/ |_______/ 00 00 00
    )"<< '\n';

    sleep(2);

    cout <<R"(                                                                                         
                                           3113322222222222277272222222222231323222232222223123123            
                                         71732222333222223322222223322222311222233322222232222222377            
                                         71322222333222          3332222333           22323222223377            
                                         712232322223     3223    11222332             7222222322377            
                                         712233322211     3223     2332317     727     7322333223777           
                                         7132222333       3323     32223       2337      73322223377            
                                         7132222233       33222233332223      12237      13222221377            
                                         7123237323       1221      1223      11227      12231123777            
                                         7121232123       1237      1321      11327      12237322377            
                                         7322113223       1117      1323      13227      17373223327            
                                         7344552233       1221      1323      13237      73222544577            
                                         7344444445       3231      1321       322       74444444577            
                                         734444444444              7133223             3444444444577            
                                         734444444444757        211244445755        7312444444444477            
                                         73444444444444427777777754444444444417777774444444444444577            
                                           3444444444444444444444444444444554444444444444444444443      
                                           )"<< '\n';

    sleep(2);
    
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