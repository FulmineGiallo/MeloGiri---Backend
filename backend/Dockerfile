FROM ubuntu:latest

# Installa pacchetti necessari
# Installa gcc per compilare il server.c
RUN apt-get update && apt-get install -y gcc
# Librerie di MYSQL
RUN apt-get install libmysqlclient-dev -y
# Librerie di Java
RUN apt install default-jre -y && apt install default-jdk -y

#Porte aperte se è necessario
EXPOSE 8080/tcp
