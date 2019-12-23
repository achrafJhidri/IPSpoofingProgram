/**

 * client.c

 */



// On importe les fichiers.h nc�ssaires.

#include <stdio.h>      // Fichier contenant les en-t�tes des fonctions standard d'entr�es/sorties

#include <stdlib.h>     // Fichier contenant les en-t�tes de fonctions standard telles que malloc()

#include <string.h>     // Fichier contenant les en-t�tes de fonctions standard de gestion de cha�nes de caract�res

#include <unistd.h>     // Fichier d'en-t�tes de fonctions de la norme POSIX (dont gestion des fichiers : write(), close(), ...)

#include <sys/types.h>  // Fichier d'en-t�tes contenant la d�finition de plusieurs types et de structures primitifs (syst�me)

#include <sys/socket.h> // Fichier d'en-t�tes des fonctions de gestion de sockets

#include <netinet/in.h> // Fichier contenant diff�rentes macros et constantes facilitant l'utilisation du protocole IP

#include <netdb.h>      // Fichier d'en-t�tes contenant la d�finition de fonctions et de structures permettant d'obtenir des informations sur le r�seau (gethostbyname(), struct hostent, ...)

#include <memory.h>     // Contient l'inclusion de string.h (s'il n'est pas d�j� inclus) et de features.h

#include <errno.h>      // Fichier d'en-t�tes pour la gestion des erreurs (notamment perror())



#define P 12332

#define LGMES 200



int main(int argc, char *argv[])

{

    if (argc != 3)

    {

        printf("adresse du serveur  + numero de port manquant\n");

        exit(1);

    }



    // On d�finit les variables utilis�es dans le programme client.

    int s, sock;

    char msg[BUFSIZ];



    // On cr�e une variable de type sokaddr_in qui sera utilis�e pour la cr�ation de la socket cliente.

    struct sockaddr_in adresse;



    // Ce pointeur sur la structure hostent sera utilis� pour la recherche d'une adresse IP connaissant le nom du serveur (r�solution de noms)

    struct hostent *recup;



    // On cr�e la socket cliente, de type AF_INET, qui utilisera UDP comme protocole de transport.

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)

    {

        perror("Erreur ouverture");

        return (-1);

    }



    // On tente de r�soudre le nom donn� par l'utilisateur, afin de r�cup�rer l'adresse qui lui correspond

    // et remplir ainsi la structure sockaddr_in

    recup = gethostbyname(argv[1]);

    if (recup == NULL)

    {

        perror("Erreur obtention adresse");

        return (-1);

    }



    /*

    On copie la zone m�moire qui contient la valeur de reecup (l'adresse IP du serveur) directement dans la zone m�moire 

    qui doit contenir cette valeur dans la structure sockaddr_in. 

    En param�tres :  la zone m�moire destination, la zone m�moire source,

    la longueur des donn�es a copier.

    */

    memcpy((char *)&adresse.sin_addr, (char *)recup->h_addr, recup->h_length);



    // On utilise toujours une socket de type AF_INET

    adresse.sin_family = AF_INET;



    // On place le num�ro de port

    adresse.sin_port = htons(atoi(argv[2]));

    socklen_t taille = sizeof(adresse);



    printf("\nMessage: ");

    scanf(" %[^\n]", msg);



    // On va ?crire sur la socket, en testant le code d'erreur de la fonction write.

    s = sendto(sock, (void *)msg, strlen(msg), 0, (struct sockaddr *)&adresse, taille);



    if (s == -1)

    {

        perror("Erreur write");

        return (-1);

    }

    else

        printf("Ecriture reussie, msg: %s\n", msg);



    // On lit la r�ponse du serveur.

    printf("Maintenant lecture\n");

    memset(msg, 0, sizeof msg);



    s = recvfrom(sock, msg, BUFSIZ, 0, (struct sockaddr *)&adresse, &taille);

    //s = read(sock, msg, 1024);



    // Message d'erreur si la lecture se passe mal.

    if (s == -1)

    {

        perror("Erreur read");

        return (-1);

    }

    else

        printf("Lecture reussie, msg: %d %s\n", s, msg);



    // On referme la socket cliente. Cette op�ration ferme la socket cliente pour le programme client, le serveur fait de m�me de son cot�,

    // en plus de refermer sa propre socket.

    close(sock);



    return 0;

}

