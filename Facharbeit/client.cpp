
#include "includes.h"


//Endlosschleife, überprüft ob in dem momentanen Empfangspuffer daten vorhanden sind
//(und gibt diese in der Konsole aus)

void ReceiveThread(sockaddr_in* param)
{
	//Folgendes Szenario:
	//In dem Programm war eine ursprüngliche, maximale Pufferlänge von 512 bytes festgelegt.
	//diese wurde allerdings nachträglich geändert (auf 1024 Bytes) und der Entwickler hat 
	//vergessen die Größe dieses Puffers anzupassen.

	char Buffer[512]; //Overflow buffer

	int serverdatalen = sizeof(sockaddr_in);

	//Hier steckt der Fehler
	int err = recvfrom(global_socket, Buffer, 1024, 0, (sockaddr*)&param, &serverdatalen);  

	if (err < 0)
	{
		printf("Server wurde ausgeschaltet. (Err = %i)\n", WSAGetLastError());
		return;
	}

	printf("%s\n", Buffer);

}

void StartReceiveThread(void* param)
{
	sockaddr_in serverdata;

	memcpy(&serverdata, param, sizeof(sockaddr_in));

	while (true)
		ReceiveThread(&serverdata);
}

//Der server muss wissen wann ein client disconnected ist.
//Dafür gibt er jedem clienten exakt 60 sekunden Zeit eine Nachricht zu senden
//Deswegen sendet der client alle 25 Sekunden einen "heartbeat"
//also einfach ein Paket, welches nicht an andere Clients weitergeleitet wird.
void HeartbeatThread(void* param)
{
	sockaddr_in* serverdata_cpy = (sockaddr_in*)param;

	sockaddr_in serverdata;

	memcpy(&serverdata, serverdata_cpy, sizeof(sockaddr_in));

	serverdata.sin_family = AF_INET;

	//Heartbeat Pakete müssen als "start"-byte nur 0xFF (255 in Dezimal) beinhalten.
	static char Buffer[1024] = "\xFF\x00";

	while (true)
	{
		int err = sendto(global_socket, Buffer, 1024, 0, (sockaddr*)&serverdata, sizeof(sockaddr_in));
		if (err <= 0)
		{
			printf("Fehler in HeartbeatThread() (err = %i)\n", WSAGetLastError());
			return;
		}

		//Der Grund warum hier nur 25 Sekunden gewartet wird ist, dass UDP Nachrichten 
		//manchmal im Internet einfach "verloren" gehen und damit der Server nicht denkt 
		//wir hätten einen Timeout, hat der Client die Chance zwei mal einen Heartbeat zu senden

		Sleep(25 * 1000);
	}
}

void StartClient(DWORD ip,WORD port)
{
	//Winsock muss erst initialisiert werden
	WSADATA data;
	char Username[512];
	char Buffer[1024];
	char SendBuffer[1024];

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
	printf("Bitte einen Benutzernamen eingeben:\n");

	//Bitte den Benutzer einen Benutzernamen einzugeben
	//dieser wird mit jeder nachricht mitgesendet und bei anderen Clienten angezeigt
	scanf("%s", &Username);

	printf("Hallo, %s!\nBitte gib eine Nachricht ein:\n\n",Username);

	//Da HeartbeatThread und ReceiveThread einen unendlichen loop beinhalten, 
	//müssen diese als eigener Thread gestartet werden
	_beginthread(HeartbeatThread, 0, &serverdata);
	_beginthread(StartReceiveThread, 0, &serverdata);

	//Warte auf Benutzer Eingabe und sende (wenn enter gedrückt wird)
	while (true)
	{
		scanf("%s", &Buffer);

		memset(SendBuffer, 0, 1024);

		_snprintf(SendBuffer, 512, "%s: %s\n", Username, Buffer);

		sendto(global_socket, SendBuffer, 512, 0, (sockaddr*)&serverdata, sizeof(sockaddr_in));
	}
}
