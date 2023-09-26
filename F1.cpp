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

int randomNumber(){
    return std::rand() % 100 + 1;
}

int main(int argc, char const *argv[])
{
    std::srand(std::time(0));
    std::cout << "Numero aleatorio: " << randomNumber() << std::endl;

    return 0;
}

