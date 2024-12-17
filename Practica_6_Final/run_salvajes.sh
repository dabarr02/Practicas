#!/bin/bash

# Número de salvajes a ejecutar
N=$1

# Verificar si se proporcionó el número de salvajes
if [ -z "$N" ]; then
    echo "Uso: $0 <número_de_salvajes>"
    exit 1
fi

# Ejecutar el cocinero
./cocinero &


# Esperar un momento para asegurarse de que el cocinero haya creado la memoria compartida
sleep 2

# Ejecutar N salvajes (clientes) concurrentemente
for ((i=1; i<=N; i++))
do
    echo "Ejecutando salvaje $i..."
    ./salvajes &
done

wait