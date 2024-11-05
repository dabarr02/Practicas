#!/bin/bash
if [ -d "$1" ]; then
  echo "El directorio $1 existe."
  rm -rf $1
else
  echo "El directorio $1 no existe."
fi
mkdir $1
cd $1
mkdir subdir
touch fichero1
echo "1234567890">fichero2
ln -s fichero2 enlaceS
ln fichero2 enlaceH

for item in *; #para recorrer el dir actual
do 
  stat "$item"
done