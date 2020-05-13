#ifndef LISTS_H_INCLUDED
#define LISTS_H_INCLUDED
#include <stdbool.h>
#include <stdlib.h>

typedef struct Utilizador
{
    char email[256];
    char password[20];
    bool admin;
    struct Utilizador *proximo;
} Utilizador_t;

struct Date
{
    int dia;
    int mes;
    int ano;
};

typedef struct Mensagem
{
    struct Date data;
    char email_r[256]; //codigo_user remetente
    char email_d[256]; //codigo_user destinatario
    int msgid;         //ID da mensagem
    char text[256];
    bool lida;
    struct Mensagem *proximo;
} Mensagem_t;

typedef struct Lista
{
    struct Mensagem *cabeca_m;
    struct Utilizador *cabeca_u;
} Lista_t;

void inicia_lista(Lista_t *const list);
void printUtilizadors(Utilizador_t *const node);
void printMensagens(Mensagem_t *node);
void insereUser(int socket_fd, Lista_t *list);
bool verificaUser(Lista_t *list, char *email);
bool validLogin(Lista_t *list, char *email, char *pass);
void deleteUser(Lista_t *list, char *email);
void insereMensagem(Lista_t *list, char *email_r, char *email_d, int id, char *text, bool lida);
void readMessage(int socket_fd, Lista_t *list, int id);        // Read a message from a given message id
void printMessages(int socket_fd, Lista_t *list, char *email); //Prints all messages from a user
void deleteMessagesRead(Lista_t *list, char *email);           //Deletes all the messages read from a user
void deleteMessagesUser(Lista_t *list, char *email);           //Deletes all messages from a user, read and not read

#endif