#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(void)
{
    int fd1,fd2,i,pos;
    char c;
    
    //Crear el fichero si no existe, borrar contenido si existe
    fd1 = open("output.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    close(fd1);
    //lo cerramos para que los procesos hijo  no hereden el fichero
    for (i=1; i < 10; i++) {
        pos = (i*5)+(i-1)*5;
        if (fork() == 0) {
            /* Child */
            char buffer[6];
            //Cada hijo abre el fichero para que no haya problemas con el puntero de posicion en el fichero y cada escriba donde le toque.
            fd1 = open("output.txt", O_RDWR);
            sprintf(buffer, "%d", i*11111);//escribimos en el buffer la cadena xxxxx que vamos a escribir luego en el output.txt
            lseek(fd1, pos, SEEK_SET); //nos  posicionamos en el lugar donde nos toca escribir
            write(fd1, buffer, 5); //escribimos
            close(fd1);//cerramos
            exit(EXIT_SUCCESS);//salimos
        } else {
            /* Parent */
            fd2 = open("output.txt", O_RDWR); //abrimos
            int posPadre=(i-1)*5+(i-1)*5; //calculamos nuestra posicion
            lseek(fd2, posPadre, SEEK_SET); //nos posicionamos
            write(fd2, "00000", 5);//escribimos
            close(fd2);//cerramos
          
        }
    }

	//wait for all childs to finish
    while (wait(NULL) != -1);
    //Mostramos el contenido del fichero por consola
    fd1 = open("output.txt", O_RDWR);
    lseek(fd1, 0, SEEK_SET);
    printf("File contents are:\n");
    while (read(fd1, &c, 1) > 0)
        printf("%c", (char) c);
    printf("\n");
    close(fd1);
    exit(EXIT_SUCCESS);
}
