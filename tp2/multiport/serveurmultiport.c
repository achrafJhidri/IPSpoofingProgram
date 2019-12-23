/**

 * serveurmultiport.c
 * @autor achraf jhidri

 */



// On importe des fichiers.h n?c?ssaires ? l'application

#include <stdio.h>      // Fichier contenant les en-t?tes des fonctions standard d'entr?es/sorties

#include <stdlib.h>     // Fichier contenant les en-t?tes de fonctions standard telles que malloc()

#include <string.h>     // Fichier contenant les en-t?tes de fonctions standard de gestion de cha?nes de caract?res

#include <unistd.h>     // Fichier d'en-t?tes de fonctions de la norme POSIX (dont gestion des fichiers : write(), close(), ...)

#include <sys/types.h>  // Fichier d'en-t?tes contenant la d?finition de plusieurs types et de structures primitifs (syst?me)

#include <sys/socket.h> // Fichier d'en-t?tes des fonctions de gestion de sockets

#include <netinet/in.h> // Fichier contenant diff?rentes macros et constantes facilitant l'utilisation du protocole IP

#include <netdb.h>      // Fichier d'en-t?tes contenant la d?finition de fonctions et de structures permettant d'obtenir des informations sur le r?seau (gethostbyname(), struct hostent, ...)

#include <memory.h>     // Contient l'inclusion de string.h (s'il n'est pas d?j? inclus) et de features.h

#include <errno.h>      // Fichier d'en-t?tes pour la gestion des erreurs (notamment perror())

#include <sys/select.h>

#define P 12332



 

void usage(const char *progname)

     {

        printf("usage:\n");

        printf("%s <portTcp> <portUdp>\n\n",progname);

        printf("\t<portTcp>   : port of tcp in which the server will listen\n");

        printf("\t<portUdp>   : port of udp in which the server will listen\n");

        printf("\n");

     }



int createSocket(u_short port, int mode, struct sockaddr_in *adresse)

{

  // On cr?e deux variables enti?res

  int sock, retour;



  /*

  La ligne suivante d?crit la cr?ation de la socket en tant que telle.

  La fonction socket prend 3 param?tres : 

  - la famille du socket : la plupart du temps, les d?veloppeurs utilisent AF_INET pour l'Internet (TCP/IP, adresses IP sur 4 octets)

    Il existe aussi la famille AF_UNIX, dans ce mode, on ne passe pas des num?ros de port mais des noms de fichiers.

  - le protocole de niveau 4 (couche transport) utilis? : SOCK_STREAM pour TCP, SOCK_DGRAM pour UDP, ou enfin SOCK_RAW pour g?n?rer

    des trames directement g?r?es par les couches inf?rieures.

  - un num?ro d?signant le protocole qui fournit le service d?sir?. Dans le cas de socket TCP/IP, on place toujours ce param?tre a 0 si on utilise le protocole par d?faut.

  */



  // mode SOCK_DGRAM / SOCK_STREAM

  sock = socket(AF_INET, mode, 0);



  // Si le code retourn? n'est pas un identifiant valide (la cr?ation s'est mal pass?e), on affiche un message sur la sortie d'erreur, et on renvoie -1

  if (sock < 0)

  {

    perror("ERREUR OUVERTURE");

    return (-1);

  }



  // On compl?te les champs de la structure sockaddr_in :

  // La famille du socket, AF_INET, comme cit? pr?c?dement

  adresse->sin_family = AF_INET;



  /* Le port auquel va se lier la socket afin d'attendre les connexions clientes. La fonction htonl()  

  convertit  un  entier  long  "htons" signifie "host to network long" conversion depuis  l'ordre des bits de l'h?te vers celui du r?seau.

  */

  adresse->sin_port = htons(port);



  /* Ce champ d?signe l'adresseTcp locale auquel un client pourra se connecter. Dans le cas d'une socket utilis?e 

  par un serveur, ce champ est initialis? avec la valeur INADDR_ANY. La constante INADDR_ANY utilis?e comme 

  adresseTcp pour le serveur a pour valeur 0.0.0.0 ce qui correspond ? une ?coute sur toutes les interfaces locales disponibles.

    

  */

  adresse->sin_addr.s_addr = INADDR_ANY;

  int opval = 1;

  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opval, sizeof(opval));



  /*

  bind est utilis? pour lier la socket : on va attacher la socket cr?e au d?but avec les informations rentr?es dans

  la structure sockaddr_in (donc une adresseTcp et un num?ro de port).

   Ce bind affecte une identit? ? la socket, la socket repr?sent?e par le descripteur pass? en premier argument est associ?e 

    ? l'adresseTcp pass?e en seconde position. Le dernier argument repr?sente la longueur de l'adresseTcp. 

    Ce qui a pour but de  rendre la socket accessible de l'ext?rieur (par getsockbyaddr)

  */

  retour = bind(sock, (struct sockaddr *)adresse, sizeof(*adresse));



  // En cas d'erreur lors du bind, on affiche un message d'erreur et on renvoie -1

  if (retour < 0)

  {

    perror("IMPOSSIBLE DE NOMMER LA SOCKET");

    return (-1);

  }



  // Au final, on renvoie sock, qui contient l'identifiant ? la socket cr?e et attach?e.

  return (sock);

}

int main(int argc, char *argv[])

{

  // On cr?e une variable adresseTcp selon la structure sockaddr_in (la structure est d?crite dans sys/socket.h)

  struct sockaddr_in adresseTcp; //

  struct sockaddr_in adresseUdp; //

  // On d?finit les variables n?c?ssaires

  int newsockfd, tcpSock, udpSock;

  int s; // contien les codes d'erreurs

  u_short portTcp, portUdp;

  char msg[BUFSIZ];



  fd_set readfds;



   if (argc != 3) {

           usage(argv[0]);

           exit(0);

        }



  portTcp = atoi(argv[1]);

  portUdp = atoi(argv[2]);



  // On cr?e la socket

  tcpSock = createSocket(portTcp, SOCK_STREAM, &adresseTcp);

  udpSock = createSocket(portUdp, SOCK_DGRAM, &adresseUdp);



  /*

  listen

	permet de dimensionner la taille de la file d'attente.

   On passe en param?tre la socket qui va ?couter, et un entier qui d?signe le nombre de connexions simultan?es autoris?es (backlog)

  */

  listen(tcpSock, 5);



  /* La fonction accept permet d'accepter une connexion ? notre socket par un client. On passe en param?tres la socket serveur d'�coute � demi d�finie.

  newsockfd contiendra l'identifiant de la socket de communication. newsockfd est la valeur de retour de la primitive accept. 

 C'est une socket d'?change de messages : elle est enti?rement d?finie.

 On peut pr?ciser aussi la structure et la taille de sockaddr associ?e 

  mais ce n'est pas obligatoire et ici on a mis le pointeur NULL

  */



  while (1)

  {



    FD_ZERO(&readfds);

    FD_SET(tcpSock, &readfds);

    FD_SET(udpSock, &readfds);



    select(udpSock + 1, &readfds, 0, 0, 0);



    socklen_t taille = sizeof(adresseTcp);

    if (FD_ISSET(udpSock, &readfds))

    {

      s = recvfrom(udpSock, msg, BUFSIZ, 0, (struct sockaddr *)&adresseTcp, &taille);



      if (s == -1)

        perror("Problemes");

      else

      {



        // Si le code d'erreur est bon, on affiche le message.

        msg[s] = 0;

        printf("Msg: %s ", msg);

        printf("Recept reussie, emission msg: ");



        // On demande ? l'utilisateur de rentrer un message qui va ?tre exp?di? sur le r?seau

        scanf(" %[^\n]", msg);



        // On va ?crire sur la socket, en testant le code d'erreur de la fonction write.

        s = sendto(udpSock, (void *)msg, strlen(msg), 0, (struct sockaddr *)&adresseTcp, taille);

        if (s == -1)

        {

          perror("Erreur write");

          return (-1);

        }

        else

          printf("Ecriture reussie, msg: %s\n", msg);

      }

    }

    else if (FD_ISSET(tcpSock, &readfds))

    {

      newsockfd = accept(tcpSock, (struct sockaddr *)0, (unsigned int *)0);



      // Si l'accept se passe mal, on quitte le programme en affichant un message d'erreur.

      if (newsockfd == -1)

      {

        perror("Erreur accept");

        return (-1);

      }

      else

        printf("Accept reussi");



      if (fork() == 0)

      {



        close(tcpSock);

        // On lit le message envoy? par la socket de communication.

        //  msg contiendra la chaine de caract?res envoy?e par le r?seau,

        // s le code d'erreur de la fonction. -1 si pb et sinon c'est le nombre de caract?res lus

        while (1)

        {

          s = read(newsockfd, msg, 1024);



          if (s == -1)

            perror("Problemes");

          else

          {



            // Si le code d'erreur est bon, on affiche le message.

            msg[s] = 0;

            printf("Msg: %s", msg);

            printf("\nRecept reussie, emission msg: \n");



            // On demande ? l'utilisateur de rentrer un message qui va ?tre exp?di? sur le r?seau

            scanf(" %[^\n]", msg);



            // On va ?crire sur la socket, en testant le code d'erreur de la fonction write.

            s = write(newsockfd, msg, strlen(msg));

            if (s == -1)

            {

              perror("Erreur write");

              return (-1);

            }

            else

              printf("Ecriture reussie, msg: %s\n", msg);

          }

        }

        close(newsockfd);

        exit(0);

      }

    }

  }

  // On referme la socket de communication

  close(tcpSock);

  close(udpSock);



  // On referme la socket d'?coute.



  return 0;

}

