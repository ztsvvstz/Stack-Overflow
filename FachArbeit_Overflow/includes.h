
#pragma once

//Visual Studio (welches ich zum Kompilieren verwende besitzt eingebaute Sicherheits�berpr�fungen (welche f�r uns 
//in diesem Moment allerdings unrelevant sind und nur zu Kompilierungsfehlern f�hren)
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>
#include <cstdio>
#include <stdlib.h>
#include <process.h>

#pragma comment(lib,"ws2_32.lib")