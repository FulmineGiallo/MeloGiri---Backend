#!/bin/bash

echo "Script di avvio in esecuzione!"
# Eseguo server.c
/backend/server &

# Comando per mantenere il container attivo
tail -f /dev/null 

