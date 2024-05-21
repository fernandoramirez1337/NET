#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <thread>
#include <fstream>
#include <cmath>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
using namespace std;

bool iniciar = true;
bool esperar = false;
bool ejecutado = false;
bool empezar_juego = false;
int contador = 0;
string mensaje = "";

void esperar_hilo(int socket_cliente, bool *aceptado, bool *turno, bool *juego) {
    sleep(1);
    while (!ejecutado) {
        char buffer[1000];
        memset(buffer, 0, sizeof(buffer));
        int n = read(socket_cliente, buffer, 2);
        buffer[n] = '\0';
        cout << buffer << endl;
        if (buffer[1] == '1') {
            n = read(socket_cliente, buffer, 5);
            buffer[n] = '\0';
            char *p = buffer;
            int tam = atoi(p);
            n = read(socket_cliente, buffer, tam);
            buffer[n] = '\0';
            contador++;
            cout << "Mensaje recibido" << endl;
            mensaje = buffer;
            memset(buffer, 0, sizeof(buffer));
        }
        else if (buffer[1] == '2') {
            n = read(socket_cliente, buffer, 5);
            buffer[n] = '\0';
            char *p = buffer;
            int tam = atoi(p);
            n = read(socket_cliente, buffer, tam);
            cout << "Otro mensaje recibido" << endl;
            buffer[n] = '\0';
            memset(buffer, 0, sizeof(buffer));
        }
        else if (buffer[1] == '3') {
            esperar = true;
            string msg = "02";
            if (contador < 10) {
                msg += "0000";
            }
            else if (contador < 99 && contador > 10) {
                msg += "000";
            }
            else if (contador < 999 && contador > 100) {
                msg += "00";
            }
            else if (contador < 9999 && contador > 1000) {
                msg += "0";
            }
            msg += to_string(contador) + mensaje;
            write(socket_cliente, msg.c_str(), 12);
            sleep(1);
            string waza = "03";
            write(socket_cliente, waza.c_str(), 2);
            ejecutado = true;
        }
        memset(buffer, 0, sizeof(buffer));
    }
}

int main(void) {
    struct sockaddr_in st_sock_addr;
    int socket_cliente = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    memset(&st_sock_addr, 0, sizeof(struct sockaddr_in));
    st_sock_addr.sin_family = AF_INET;
    st_sock_addr.sin_port = htons(5000);
    inet_pton(AF_INET, "localhost", &st_sock_addr.sin_addr);

    if (connect(socket_cliente, (const struct sockaddr *)&st_sock_addr, sizeof(struct sockaddr_in)) < 0) {
        cout << "Error al conectar al servidor." << endl;
        return -1;
    }

    string entrada;
    thread hilo_esperar(esperar_hilo, socket_cliente, &iniciar, &esperar, &empezar_juego);

    while (!ejecutado) {
        char buffer[1000];
        memset(buffer, 0, sizeof(buffer));
        if (!empezar_juego) {
            string mensaje_inicial = "01000102111051834";
            write(socket_cliente, mensaje_inicial.c_str(), 17);
            empezar_juego = true;
        }
        memset(buffer, 0, sizeof(buffer));
    }

    close(socket_cliente);
    return 0;
}