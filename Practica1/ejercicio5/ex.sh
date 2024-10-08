#!/bin/bash

foo=script.sh

if test -f $foo && [ -x script.sh ] ; then

   echo "Correcto"

else

  echo "Incorrecto"

fi