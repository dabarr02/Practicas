#include <stdio.h>     // Librería estándar para entrada y salida
#include <stdlib.h>    // Librería estándar general
#include <unistd.h>    // Librería para funciones POSIX (sleep, etc.)
#include <pthread.h>   // Librería para manejar hilos POSIX

// Definición de la capacidad máxima de la discoteca
#define CAPACITY 2

// Macro para convertir el estado VIP a cadena de texto
#define VIPSTR(vip) ((vip) ? "  vip  " : "not vip")

// Estructura para pasar argumentos a cada hilo (cliente)
typedef struct {
    int thread_num;  // Identificador del hilo (cliente)
    int priority;    // Prioridad del cliente (1 para VIP, 0 para normal)
} thread_arg;

// Variables globales para sincronización
pthread_mutex_t cerrojo;        // Mutex para proteger secciones críticas
pthread_cond_t vip_cond;        // Variable de condición para VIPs
pthread_cond_t normal_cond;     // Variable de condición para normales
int ocupacion = 0;                  // Ocupación actual de la discoteca

// Contadores para mantener el orden de llegada y turno de entrada
int vip_dispensador = 0;            // Siguiente turno que se va a asignar vip
int vip_next = 0;               // Siguiente VIP que puede entrar
int normal_dispensador = 0;         // Siguiente turno que se va a asignar normal
int normal_next = 0;            // Siguiente normal que puede entrar

// Función para que los clientes VIP entren a la discoteca
void enter_vip_client(int id)
{
    pthread_mutex_lock(&cerrojo);  // Adquirir el mutex para entrar en sección crítica

    int my_turn = vip_dispensador;   // Asignar el turno al cliente VIP
	vip_dispensador++;
    // Esperar hasta que sea su turno y haya espacio en la discoteca
    while (ocupacion >= CAPACITY || vip_next != my_turn) {
        pthread_cond_wait(&vip_cond, &cerrojo);
    }
    //Entramos en discoteca
    ocupacion++;    // Incrementar la ocupacion
    printf("Cliente %2d (%s) entra a la discoteca\n", id, VIPSTR(1));
    //Como hemos entrado, tenemos que actualizar quien es el siguiente que puede entrar
    vip_next++; // Incrementar el siguiente VIP que puede entrar
	if(vip_next<vip_dispensador){//Si hay vips esperando despertamos a los vips
      pthread_cond_broadcast(&vip_cond);
	} else if (normal_next < normal_dispensador) {//Si no hay vips pero si normales, avisamos a los normales
        pthread_cond_broadcast(&normal_cond); // Despertar a los normales
    }
    pthread_mutex_unlock(&cerrojo); // Liberar el mutex
}

// Función para que los clientes normales entren a la discoteca
void enter_normal_client(int id)
{
    pthread_mutex_lock(&cerrojo);  // Adquirir el mutex para entrar en sección crítica

    int my_turn = normal_dispensador;  // Asignar el turno al cliente normal
	normal_dispensador++;
    // Esperar hasta que sea su turno, no haya VIPs esperando y haya espacio
    while (ocupacion >= CAPACITY || vip_next < vip_dispensador || normal_next != my_turn) {
        pthread_cond_wait(&normal_cond, &cerrojo);
    }

    ocupacion++;    // Incrementar la ocupacion
    printf("Cliente %2d (%s) entra a la discoteca\n", id, VIPSTR(0));

    normal_next++; // Incrementar el siguiente normal que puede entrar
	if (normal_next < normal_dispensador) {
        pthread_cond_broadcast(&normal_cond); // Despertar a los normales
    }
    pthread_mutex_unlock(&cerrojo); // Liberar el mutex
}

// Función para simular que el cliente está bailando en la discoteca
void dance(int id, int isvip)
{
    printf("Cliente %2d (%s) está bailando en la discoteca\n", id, VIPSTR(isvip));
    sleep((rand() % 3) + 1); // Dormir entre 1 y 3 segundos para simular el baile
}

// Función para que los clientes salgan de la discoteca
void disco_exit(int id, int isvip)
{
    pthread_mutex_lock(&cerrojo);  // Adquirir el mutex para entrar en sección crítica

    ocupacion--;  // Decrementar la ocupacion ya que el cliente sale
    printf("Cliente %2d (%s) sale de la discoteca\n", id, VIPSTR(isvip));

    // Despertar al siguiente cliente que pueda entrar
    if (vip_next < vip_dispensador) {
        pthread_cond_broadcast(&vip_cond);   // Despertar a los VIPs
    } else if (normal_next < normal_dispensador) {
        pthread_cond_broadcast(&normal_cond); // Despertar a los normales
    }

    pthread_mutex_unlock(&cerrojo);   // Liberar el mutex
}

// Función que ejecutará cada hilo (representa a un cliente)
void *client(void *arg)
{
    thread_arg* t_arg = (thread_arg*)arg;  // Convertir el argumento al tipo 'thread_arg'
    int id = t_arg->thread_num;            // Obtener el identificador del cliente
    int prio = t_arg->priority;            // Obtener la prioridad (VIP o normal)
    free(t_arg);                           // Liberar la memoria asignada para los argumentos

    printf("Cliente %2d (%s) espera para entrar a la discoteca\n", id, VIPSTR(prio));

    if (prio == 1) {
        enter_vip_client(id);  // Intentar entrar como VIP
    } else {
        enter_normal_client(id);  // Intentar entrar como cliente normal
    }

    dance(id, prio);      // Simular que el cliente está bailando
    disco_exit(id, prio); // El cliente sale de la discoteca

    pthread_exit(EXIT_SUCCESS);      // Terminar el hilo
}

int main(int argc, char *argv[])
{
    int n;        // Número de clientes a crear
    int p;        // Prioridad del cliente (VIP o normal)

    pthread_t *threads;   // Puntero para almacenar los identificadores de los hilos

    // Aceptar el nombre del archivo de entrada desde los argumentos de línea de comandos
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <nombre_archivo_entrada>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Abrir el archivo de entrada especificado en el argumento de línea de comandos
    FILE* file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error al abrir el archivo de entrada");
        exit(EXIT_FAILURE);
    }

    // Inicializar el mutex y las variables de condición
    if (pthread_mutex_init(&cerrojo, NULL) != 0) {
        printf("Error al inicializar el mutex\n");
        fclose(file);
        return -1;
    }
    if (pthread_cond_init(&vip_cond, NULL) != 0) {
        printf("Error al inicializar la variable de condición 'vip_cond'\n");
        pthread_mutex_destroy(&cerrojo);
        fclose(file);
        return -1;
    }
    if (pthread_cond_init(&normal_cond, NULL) != 0) {
        printf("Error al inicializar la variable de condición 'normal_cond'\n");
        pthread_cond_destroy(&vip_cond);
        pthread_mutex_destroy(&cerrojo);
        fclose(file);
        return -1;
    }

    // Leer el número de clientes desde el archivo
    if (fscanf(file, "%d\n", &n) != 1) {
        perror("Error al leer el número de clientes");
        pthread_cond_destroy(&normal_cond);
        pthread_cond_destroy(&vip_cond);
        pthread_mutex_destroy(&cerrojo);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Asignar memoria para el array de hilos
    threads = (pthread_t*)malloc(n * sizeof(pthread_t));
    if (threads == NULL) {
        perror("Error al asignar memoria para los hilos");
        pthread_cond_destroy(&normal_cond);
        pthread_cond_destroy(&vip_cond);
        pthread_mutex_destroy(&cerrojo);
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Crear los hilos (clientes)
    for (int i = 0; i < n; i++) {
        // Asignar memoria para los argumentos del hilo
        thread_arg* argumento_hilo = (thread_arg*)malloc(sizeof(thread_arg));
        if (argumento_hilo == NULL) {
            perror("Error al asignar memoria para los argumentos del hilo");
            pthread_cond_destroy(&normal_cond);
            pthread_cond_destroy(&vip_cond);
            pthread_mutex_destroy(&cerrojo);
            fclose(file);
            free(threads);
            exit(EXIT_FAILURE);
        }
        // Leer la prioridad (VIP o normal) desde el archivo
        if (fscanf(file, "%d\n", &p) != 1) {
            perror("Error al leer la prioridad del cliente");
            free(argumento_hilo);
            pthread_cond_destroy(&normal_cond);
            pthread_cond_destroy(&vip_cond);
            pthread_mutex_destroy(&cerrojo);
            fclose(file);
            free(threads);
            exit(EXIT_FAILURE);
        }
        // Establecer los argumentos del hilo
        argumento_hilo->priority = p;
        argumento_hilo->thread_num = i;
        // Crear el hilo del cliente
        if (pthread_create(&threads[i], NULL, client, argumento_hilo) != 0) {
            perror("Error al crear el hilo");
            free(argumento_hilo);
            pthread_cond_destroy(&normal_cond);
            pthread_cond_destroy(&vip_cond);
            pthread_mutex_destroy(&cerrojo);
            fclose(file);
            free(threads);
            exit(EXIT_FAILURE);
        }
    }
    fclose(file); // Cerrar el archivo de entrada

    // Esperar a que todos los hilos terminen
    for (int j = 0; j < n; j++) {
        pthread_join(threads[j], NULL);
    }

    // Liberar la memoria del array de hilos
    free(threads);

    // Destruir el mutex y las variables de condición después de usarlas
    pthread_mutex_destroy(&cerrojo);
    pthread_cond_destroy(&vip_cond);
    pthread_cond_destroy(&normal_cond);

    return 0;
}