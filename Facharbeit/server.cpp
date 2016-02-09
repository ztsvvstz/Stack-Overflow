
#include "includes.h"

//Struktur zur Speicherung von verbundenen Clienten (Addresse und Zeitstempel)
//Hier könnte man in einer professionellern Anwendung 
//selbstverständlich mehr Daten speichern
//aber für uns ist dies momentan irrelevant
struct SClientData
{
	sockaddr_in address;
	DWORD lastPing;
};

//Diese Funktion empfängt Nachrichten von allen Clienten und sendet diese an alle
//anderen Clienten.
void BroadcastThread()
{
	//Wir brauchen einen Array, welcher die Clienten Daten speichert
	//Dies könnte man selbstverständlich auch einfacher (und besser)
	//lösen wie zum Beispiel dem verwenden von std::vector
	//Allerdings beschränken wir uns in diesem Beispiel auf C Funktionen.
	//Ein statischer Buffer, welcher nur eine bestimmte Anzahl an verbundenen Clienten
	//speichern kann wäre auch möglich, allerdings nicht so dynamisch.
	SClientData** ClientPointerTable = (SClientData**)calloc(1, sizeof(SClientData*));

	//Anzahl verbundener Clienten
	int ClientCount = 0;

	//Addresse des Absenders einer Nachricht
	sockaddr_in Addr;

	//Größe der struct, wird als Parameter von recvfrom benötigt.
	int AddrLen = sizeof(sockaddr_in);

	//Nachrichtenpuffer
	//Diesen könnte man natürlich auch überlaufen lassen
	//aber in diesem Beispiel gehen wir davon aus das
	//der Entwickler hier die Puffergröße angepasst hat.
	char Buffer[1024];

	//Helfer, dient nur zum finden (und nicht weiterleiten!) der Nachricht an den Absender
	bool FoundClient = false;

	while (true)
	{
		//Alles wird mit 0 initialisiert
		memset(Buffer, 0, sizeof(Buffer));
		FoundClient = false;

		//Empfange Nachrichten von Clienten
		recvfrom(global_socket, Buffer, 1024, 0, (sockaddr*)&Addr, &AddrLen);

		//Wegen strlen. Andererseits könnte man den Server mit nicht "genullten" Paketen zum Absturz bringen.
		Buffer[1023] = 0;

		int i = 0;

		while (true)
		{
			if (i >= ClientCount - 1)
				break;

			//Überprüfe ob der momentane Array Eintrag mit der momentanen Absender Addresse übereinstimmt
			if (ClientPointerTable[i]->address.sin_addr.s_addr == Addr.sin_addr.s_addr &&
				ClientPointerTable[i]->address.sin_port == Addr.sin_port)
			{
				//Client gefunden (ist bereits verbunden)
				//Zeitstempel updaten und fortfahren.
				ClientPointerTable[i]->lastPing = GetTickCount();
				FoundClient = true;
				++i;
				continue;
			}

			//Ist ein Client länger als 60 Sekunden nicht aktiv, wird er gelöscht
			if (GetTickCount() - ClientPointerTable[i]->lastPing > 60 * 1000)
			{
				//Der Speicher der Struktur wird nun nicht mehr benötigt,
				//und kann für andere Dinge freigegeben werden.
				free(ClientPointerTable[i]);

				//Alle nachfolgenden Einträge werden eins nach vorne verschoben
				for (int j = i; j < ClientCount-1; ++j)
					ClientPointerTable[j] = ClientPointerTable[j + 1];

				ClientCount -= 1;

				//Der Pointer zu der (bereits gelöschten) Struktur kann nun auch freigegeben werden.
				ClientPointerTable = (SClientData**)_recalloc(ClientPointerTable, ClientCount, sizeof(SClientData*));

				continue;
			}

			//Ist die Nachricht kein Heartbeat (erster byte ist 0xFF) wird diese an jeden Clienten weitergeleitet.
			//Hier sollte man auch darauf achten, nicht strlen zu verwenden.
			if (Buffer[0] != 0xFF && Buffer[0] != 0) //HeartBeat
				sendto(global_socket, Buffer, strlen(Buffer), 0, (sockaddr*)&ClientPointerTable[i]->address, sizeof(sockaddr_in));

			++i;
		}


		//Wurde der Client noch nicht gefunden, wird ein Eintrag für ihn angelegt.
		if (!FoundClient)
		{

			ClientCount += 1;

			//Ein neuer Pointer zu einer Client Struktur wird benötigt
			ClientPointerTable = (SClientData**)_recalloc(ClientPointerTable, ClientCount, sizeof(SClientData*));
			//Alloziiere die Client Struktur
			ClientPointerTable[ClientCount - 1] = (SClientData*)calloc(1, sizeof(SClientData));
			//Und Kopiere die erforderlichen Daten in die neu alloziierte Struktur
			memcpy(&ClientPointerTable[ClientCount - 1]->address, &Addr, sizeof(sockaddr_in));
			//Der Zeitstempel muss noch gesetzt werden.
			ClientPointerTable[ClientCount - 1]->lastPing = GetTickCount();
		}

	}

}

void StartServer(WORD port)
{
	//Winsock muss erst initialisiert werden (gleich dem Client code - siehe client.cpp)
	WSADATA data;

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
	{
		printf("WSAStartup nicht erfolgreich.\n");
		return;
	}

	global_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sockaddr_in serverdata;

	serverdata.sin_family = AF_INET;
	serverdata.sin_addr.s_addr = INADDR_ANY;
	serverdata.sin_port = htons(port);

	//Im Gegensatz zum Clienten muss der Serversocket allerdings noch an einen bestimmten
	//Port gebunden werden
	if (bind(global_socket, (sockaddr*)&serverdata, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		printf("Fehler in bind().\nLäuft der server bereits?\nIst der Zielport bereits belegt?\n");
		return;
	}

	printf("Server erfolgreich gestartet (port = %hu)\n", port);

	//Diese Funktion könnte man auch als thread starten,
	//Da der server danach aber sowieso nichts mehr zu tun hätte
	//lassen wir ihn einfach in der Endlosschleife laufen.
	BroadcastThread();

}
