#!/bin/bash
a=" | "
cadena=Mundo
while IFS=':' read LOGIN_NAME_IDX ENCRYPTED_PASS_IDX UID_IDX GID_IDX USER_NAME_IDX USER_HOME_IDX USER_SHELL_IDX NR_FIELDS_PASSWD
do
    #if [[ "$USER_HOME_IDX" == */home* ]]; then
    salida=$LOGIN_NAME_IDX$a$ENCRYPTED_PASS_IDX$a$UID_IDX$a$GID_IDX$a$USER_NAME_IDX$a$USER_HOME_IDX$a$USER_SHELL_IDX$a$NR_FIELDS_PASSWD
    echo $salida
    #fi
    # cualquier cosa con $var1, $var2
done < $1

#para usar pipes $ ./ej5.sh|grep /home