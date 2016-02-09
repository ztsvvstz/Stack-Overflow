
#include "includes.h"

SOCKET global_socket; //Global definierter socket, wird vom client und server benutzt

int main(int argc,char *argv[])
{

	//Parameter �bergabe

	for (int i = 0; i < argc; ++i)
	{
		if (!strcmp(argv[i], "server"))
		{
			if ((i + 1) >= argc)
			{
				printf("Falscher Parameter.\nserver <port>\n");
				system("pause");
				return 0;
			}

			StartServer(atoi(argv[i + 1]));

		}
		if (!strcmp(argv[i], "client"))
		{
			if ((i + 2) >= argc)
			{
				printf("Falscher Parameter.\client <ip> <port>\n");
				system("pause");
				return 0;
			}

			StartClient(inet_addr(argv[i + 1]), atoi(argv[i + 2]));
		}
	}

	system("pause"); //Sorgt daf�r das das Programm nicht einfach beendet wird (zeigt an Dr�cken sie eine beliebige Taste...)

	return 1; //Kein Fehler
}