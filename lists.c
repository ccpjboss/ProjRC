#include "lists.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

/*
 *  Inicia a lista ligada com as cabecas NULL 
 *
 *  list: ponteiro a apontar para a nova lista 
 */
void inicia_lista(Lista_t *list)
{
	list->cabeca_m = NULL;
	list->cabeca_u = NULL;
}

/*
 *  Mostra todos os utilizadores. Ultilizada em DEBUG apenas 
 */
void printUtilizadors()
{
	Lista_t *list = (Lista_t *)malloc(sizeof(Lista_t));
	inicia_lista(list);
	leUserFile(list);
	Utilizador_t *atual = list->cabeca_u;
	while (atual != NULL)
	{
		printf("%s  %s \n", atual->email, atual->password);
		atual = atual->proximo;
	}
	free(list);
}

/*
 *  Mostrar todas as mensagens. DEBUG
 */
void printMensagens()
{
	Lista_t *list = (Lista_t *)malloc(sizeof(Lista_t));
	inicia_lista(list);
	leMessagesFile(list);

	Mensagem_t *atual = list->cabeca_m;
	while (atual != NULL)
	{
		printf("%d %s %s\n", atual->msgid, atual->email_d, atual->text);
		atual = atual->proximo;
	}
	free(list);
}

/*
 *  Verifica se o email dado ja existe
 * 	
 * 	email: char *email a verificar se ja existe
 * 
 *  returns: true se duplicado / false se nao duplicado
 */
bool verificaUser(char *email)
{
	Lista_t *list = (Lista_t *)malloc(sizeof(Lista_t));
	inicia_lista(list);
	leUserFile(list);

	bool dup = false;
	Utilizador_t *atual = list->cabeca_u;
	while (atual != NULL)
	{
		if (strcmp(email, atual->email) == 0)
			dup = true;
		atual = atual->proximo;
	}
	free(list);
	return dup;
}

/*
 * 	Insere um novo utilizador
 * 
 *  socket_fd: fd do socket para escrever mensagens para o cliente
 */
void insereUser(int socket_fd)
{
	Utilizador_t *new = (Utilizador_t *)malloc(sizeof(Utilizador_t));

	char msg_send1[] = "Insira o email: ";
	char msg_send2[] = "Insira a palavra passe: ";
	char buff[256];
	char buff2[20];

	do
	{
		write(socket_fd, msg_send1, sizeof(msg_send1)); //Envia mensagem de input para o cliente
		bzero(buff, 256);								//Coloca o buffer a 0
		read(socket_fd, buff, sizeof(buff));			//Le o input do cliente
		buff[strcspn(buff, "\r\n")] = 0;
		strcpy(new->email, buff); //Copia para a variavel da struct
	} while (verificaUser(new->email));

	write(socket_fd, msg_send2, sizeof(msg_send2));
	bzero(buff2, 20);
	read(socket_fd, buff2, sizeof(buff2));
	buff2[strcspn(buff2, "\r\n")] = 0;
	strcpy(new->password, buff2);

	new->admin = false;
	new->proximo = NULL;

	Lista_t *list = (Lista_t *)malloc(sizeof(Lista_t));
	inicia_lista(list);
	leUserFile(list);

	Utilizador_t *atual = list->cabeca_u;

	if (list->cabeca_u == NULL) //Se a cabeça for igual a NULL então new vai ser o primeiro nó
	{
		list->cabeca_u = new;
		guardaUsersFile(list);
		free(list);
		return;
	}
	else
	{
		while (atual->proximo != NULL)
		{
			atual = atual->proximo;
		}
		atual->proximo = new;
		guardaUsersFile(list);
		free(list);
		return;
	}
}

/**
 *  Apaga um utilizador.
 * 
 *  email: email do utilizador a apagar
 */
void deleteUser(char *email)
{
	/* Load records */
	Lista_t *list = (Lista_t *)malloc(sizeof(Lista_t));
	inicia_lista(list);
	leUserFile(list);

	Utilizador_t *atual = list->cabeca_u;
	Utilizador_t *anterior = NULL;

	if ((atual != NULL) && (strcmp(email, atual->email) == 0)) //Se a cabeca for diferente de NULL e for o email procurado
	{
		list->cabeca_u = atual->proximo; //A cabeca passa a ser o no a seguir
		free(atual);					 //Apaga o atual
		guardaUsersFile(list);			 //Atualiza o save file
		free(list);
		return;
	}

	while (atual != NULL) //Enquanto a cabeca for diferente de NULL
	{
		if (strcmp(email, atual->email) == 0) //Se a condicao se verificar, sai do while e temos atual == ao no que queremos apagar
			break;

		anterior = atual;		//O anterior vai ser igual ao atual
		atual = atual->proximo; //e o atual vai ser igual ao atual proximo
	}

	if (atual == NULL)
	{
		free(list);
		return; //Se não encontrou nenhum no
	}
	anterior->proximo = atual->proximo;
	free(atual);
	guardaUsersFile(list); //Atualiza o save file
	free(list);
	return;
}

/**
 * Verifica se a palavra passe corresponde ao email inserido.
 * 
 * email: email inserido.
 * pass: palavra passe inserida.
 * 
 * returns: true se corresponde / false se nao corresponde
 */
bool validLogin(char *email, char *pass)
{
	Lista_t *list = (Lista_t *)malloc(sizeof(Lista_t));
	inicia_lista(list);
	leUserFile(list);

	Utilizador_t *atual = list->cabeca_u;

	while (atual != NULL)
	{
		if (strcmp(atual->email, email) == 0)
			break;
		atual = atual->proximo;
	}
	if (atual == NULL) //Se o atual for igual a NULL significa que percorreu a lista toda e não encontrou nenhum email
	{
		free(list);
		return false;
	}
	if (strcmp(atual->password, pass) == 0) //Atual aqui esta igual ao no do email passado
	{
		free(list);
		return true;
	}
	else
	{
		free(list);
		return false;
	}
}

/**
 * Insere uma nova mensagem.
 * 
 * fd: file descriptor do socket.
 * email_r: email do remetente da mensagem.
 * email_d: email do destinatario da mensagem.
 * id: id da mensagem.
 * text: texto da mensagem.
 * lida: mensagem lida ou nao lida, valor inicial false.
 */
void insereMensagem(int fd, char *email_r, char *email_d, int id, char *text, bool lida)
{
	Lista_t *list = (Lista_t *)malloc(sizeof(Lista_t));
	inicia_lista(list);
	leMessagesFile(list);

	Mensagem_t *new = (Mensagem_t *)malloc(sizeof(Mensagem_t));
	Mensagem_t *atual = list->cabeca_m;

	strcpy(new->email_r, email_r);
	if (verificaUser(email_d) == false)
	{
		char msg[] = "Email invalido!\n";
		write(fd, msg, sizeof(msg));
		return;
	}

	strcpy(new->email_d, email_d);
	new->msgid = id;
	strcpy(new->text, text);
	new->lida = false;
	new->proximo = NULL;

	if (list->cabeca_m == NULL)
	{
		list->cabeca_m = new;
		guardaMensagensFile(list);
		free(list);
		return;
	}
	else
	{
		while (atual->proximo != NULL)
		{
			atual = atual->proximo;
		}
		atual->proximo = new;
		guardaMensagensFile(list);
		free(list);
		return;
	}
}

/**
 * Vai buscar o ultimo id das mensagens presentes.
 * 
 * returns: retorna o valor do ultimo id das mensagens.
 */ 
int getLastID()
{
	int id;
	Lista_t *list = (Lista_t *)malloc(sizeof(Lista_t));
	inicia_lista(list);
	leMessagesFile(list);

	Mensagem_t *atual = list->cabeca_m;
	if (atual == NULL)
	{
		return 0;
	}

	while (atual->proximo != NULL)
	{
		atual = atual->proximo;
	}

	id = atual->msgid;
	free(list);
	return id;
}

/**
 * Le a mensagem, muda a flag lida para true.
 * 
 * socket_fd: file descriptor do socket.
 * id: id da mensagem a ler.
 * email: email do destinatario.
 */
void readMessage(int socket_fd, int id, char *email) // Read a message from a given message id
{
	Lista_t *list = (Lista_t *)malloc(sizeof(Lista_t));
	inicia_lista(list);
	leMessagesFile(list);

	Mensagem_t *atual = list->cabeca_m;
	char buff[256];
	bzero(buff, 256);

	while (atual != NULL)
	{
		if (atual->msgid == id && strcmp(atual->email_d, email) == 0)
			break;
		atual = atual->proximo;
	}

	if (atual == NULL)
	{
		free(list);
		return;
	}
	strcpy(buff, atual->text);
	write(socket_fd, buff, sizeof(buff));
	atual->lida = true;
	guardaMensagensFile(list); //Temos de guardar o ficheiro novamente pois atualizamos o campo lida
	free(list);
}

/**
 * Mostra as mensagens na caixa de entrada.
 * 
 * socket_fd: file descriptor do socket.
 * email: email do destinatario das mensagens
 */
void printMessages(int socket_fd, char *email) //Prints all messages from a user
{
	Lista_t *list = (Lista_t *)malloc(sizeof(Lista_t));
	inicia_lista(list);
	leMessagesFile(list);

	Mensagem_t *atual = list->cabeca_m;
	char msg_to_send[296];
	bzero(msg_to_send, 296); //Resets the buffer

	while (atual != NULL)
	{
		if (strcmp(email, atual->email_d) == 0)
		{
			if (atual->lida == true)
				sprintf(msg_to_send, "id:%d  email remetente:%s [Lida]\n", atual->msgid, atual->email_r);
			else
				sprintf(msg_to_send, "id:%d  email remetente:%s [N Lida]\n", atual->msgid, atual->email_r);

			write(socket_fd, msg_to_send, sizeof(msg_to_send));
		}
		atual = atual->proximo;
	}

	if (atual == NULL)
	{
		free(list);
		return;
	}
}

/**
 * Apaga as mensagens de um utlizador. Usada quando é apagado um user.
 * 
 * email: email do utilizador apagado.
 */ 
void deleteMessagesUser(char *email) //Deletes all the messages from a user
{
	Lista_t *list = (Lista_t *)malloc(sizeof(Lista_t));
	inicia_lista(list);
	leMessagesFile(list);
	Mensagem_t *atual = list->cabeca_m;
	Mensagem_t *anterior = NULL;

	while ((atual != NULL) && (strcmp(atual->email_d, email) == 0))
	{
		list->cabeca_m = atual->proximo;
		free(atual);
		atual = list->cabeca_m;
	}

	while (atual != NULL)
	{
		while ((atual != NULL) && (strcmp(atual->email_d, email) != 0))
		{
			anterior = atual;
			atual = atual->proximo;
		}

		if (atual == NULL)
		{
			guardaMensagensFile(list);
			free(list);
			return;
		}

		anterior->proximo = atual->proximo;
		free(atual);

		atual = anterior->proximo;
	}
	guardaMensagensFile(list);
	free(list);
}

/**
 * Apaga as mensagens lidas de um utilizador.
 * 
 * email: email do utilizador.
 */
void deleteMessagesRead(char *email) //Deletes all messages from a user, read
{
	Lista_t *list = (Lista_t *)malloc(sizeof(Lista_t));
	inicia_lista(list);
	leMessagesFile(list);

	Mensagem_t *atual = list->cabeca_m;
	Mensagem_t *anterior = NULL;

	if (list->cabeca_m == NULL)
	{
		free(list);
		return;
	}

	while ((list->cabeca_m != NULL) && (strcmp(list->cabeca_m->email_d, email) == 0) && (list->cabeca_m->lida == true))
	{
		anterior = list->cabeca_m;
		list->cabeca_m = list->cabeca_m->proximo;
		free(anterior);
	}

	atual = list->cabeca_m;
	anterior = NULL;

	while (atual != NULL)
	{
		while ((atual != NULL) && (atual->lida == false))
		{

			anterior = atual;
			atual = atual->proximo;
		}

		if (atual == NULL)
		{
			guardaMensagensFile(list);
			free(list);
			return;
		}

		if (strcmp(atual->email_d, email) == 0)
		{
			anterior->proximo = atual->proximo;
			free(atual);
			atual = anterior->proximo;
		}
		else
		{
			anterior = atual;
			atual = atual->proximo;
		}
	}
	guardaMensagensFile(list);
	free(list);
	return;
}

/**
 * Le o ficheiro das mensagens para carregar a lista ligada.
 * 
 * list: lista ligada para guardar os dados lidos
 **/ 
void leMessagesFile(Lista_t *list)
{
	Mensagem_t *atual = list->cabeca_m;
	/* Opens the file for reading in binary mode */
	FILE *infile;
	infile = fopen("mensagem.dat", "r");

	if (infile == NULL) // Checks for error
	{
		perror("fopen mensagem.dat");
		exit(1);
	}

	/* Checks for empty file */
	fseek(infile, 0, SEEK_END);
	int size = ftell(infile);

	if (0 == size)
	{
		printf("File is empty\n");
		return;
	}
	rewind(infile);

	while (1)
	{
		/* READ */
		Mensagem_t *new = (Mensagem_t *)malloc(sizeof(Mensagem_t));
		int check = fread(new, sizeof(Mensagem_t), 1, infile);

		/* CHECK */
		if (check == 0)
			break;

		/* USE */
		if (list->cabeca_m == NULL)
		{
			list->cabeca_m = new;
			new->proximo = NULL;
			atual = list->cabeca_m;
		}
		else
		{
			atual->proximo = new;
			new->proximo = NULL;
			atual = atual->proximo;
		}
	}
	fclose(infile);
	return;
}

/**
 * Le o ficheiro dos utilizadores para carregar a lista ligada.
 * 
 * list: lista ligada para guardar os dados lidos
 **/ 
void leUserFile(Lista_t *list)
{
	Utilizador_t *atual = list->cabeca_u;
	FILE *infile;
	infile = fopen("utilizador.dat", "r");

	if (infile == NULL)
	{
		perror("fopen utilizador.dat");
		exit(1);
	}

	fseek(infile, 0, SEEK_END);
	int size = ftell(infile);

	if (0 == size)
	{
		printf("File is empty\n");
		return;
	}
	rewind(infile);

	while (1)
	{
		/* READ */
		Utilizador_t *new = (Utilizador_t *)malloc(sizeof(Utilizador_t));
		int check = fread(new, sizeof(Utilizador_t), 1, infile);
		/* CHECK */
		if (check == 0)
			break;
		/* USE */
		if (list->cabeca_u == NULL)
		{
			list->cabeca_u = new;
			new->proximo = NULL;
			atual = list->cabeca_u;
		}
		else
		{
			atual->proximo = new;
			new->proximo = NULL;
			atual = atual->proximo;
		}
	}
	fclose(infile);
	return;
}

/**
 * Escreve a lista ligada no ficheiro
 * 
 * list: lista a escrever
 */
void guardaMensagensFile(Lista_t *list) //Escreve a lista das mensagens no ficheiro
{
	Mensagem_t *atual = list->cabeca_m;
	FILE *outfile;
	outfile = fopen("mensagem.dat", "w");

	if (outfile == NULL)
	{
		perror("fopen mensgem.dat");
		exit(1);
	}

	if (list->cabeca_m == NULL)
	{
		printf("Nothing to write to file...");
		return;
	}

	while (atual != NULL)
	{
		fwrite(atual, sizeof(Mensagem_t), 1, outfile);
		atual = atual->proximo;
	}

	fclose(outfile);
	return;
}

/**
 * Escreve a lista ligada no ficheiro
 * 
 * list: lista a escrever
 */
void guardaUsersFile(Lista_t *list) //Escreve a lista dos users no ficheiro
{
	Utilizador_t *atual = list->cabeca_u;
	FILE *outfile;
	outfile = fopen("utilizador.dat", "w");

	if (outfile == NULL)
	{
		perror("fopen utilizador.dat");
		exit(1);
	}

	if (list->cabeca_u == NULL)
	{
		printf("Nothing to write to file...");
		return;
	}

	while (atual != NULL)
	{
		fwrite(atual, sizeof(Utilizador_t), 1, outfile);
		atual = atual->proximo;
	}

	fclose(outfile);
	return;
}