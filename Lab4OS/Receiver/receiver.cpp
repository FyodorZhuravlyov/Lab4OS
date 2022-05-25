#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include<conio.h>
#include <Windows.h>

HANDLE* readyEvents;
const int MESSAGE_SIZE = 20;
int CreateEv(int sendersCount, char filename[80]) 
{
	readyEvents = new HANDLE[sendersCount];
	char buff[10];
	for (int i = 0; i < sendersCount; i++) {
		//creating event
		char eventName[30] = "sender";
		strcat(eventName, itoa(i, buff, 10));
		readyEvents[i] = CreateEvent(NULL, TRUE, FALSE, LPWSTR(eventName));
		if (NULL == readyEvents[i]) {
			printf("Creation event failed.");
			return GetLastError();
		}

	}
}

int CreateProc(int sendersCount, char filename[80])
{
	for (int i = 0; i < sendersCount; i++)
	{
		const char* args = "/*path to sender.exe*/";
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		if (!CreateProcess(NULL, LPWSTR(args), NULL, NULL, FALSE,
			CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
			printf("Error in process creating\n");
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}
}

int receiveMessage(char* filename) {
	std::fstream in(filename, std::ios::binary | std::ios::in);
	if (!in.is_open()) {
		std::cout<< "Opening file failed.\n";
		return -1;
	}
	char res[MESSAGE_SIZE];
	in.read(res, MESSAGE_SIZE);
	in.seekg(0, std::ios::end);
	int n = in.tellg();
	in.seekg(0, std::ios::beg);
	char* temp = new char[n];
	in.read(temp, n);
	in.close();
	in.open(filename, std::ios::binary | std::ios::out);
	in.write(temp + MESSAGE_SIZE, n - MESSAGE_SIZE);
	in.close();
	delete[] temp;
	std::cout << res;
	return 0;
}

int main() {
	std::cout << "Input filename" << std::endl;
	char filename[100];
	std::cin >> filename;
	std::cout << "Input number of sender processes" << std::endl;
	int countOfSender;
	std::cin >> countOfSender;

	//HANDLE* senders = new HANDLE[countOfSender];
	//HANDLE* events = new HANDLE[countOfSender];

	HANDLE startALL = CreateEvent(NULL, TRUE, FALSE, L"START_ALL");
	HANDLE fileMutex = CreateMutex(NULL, FALSE, L"FILE_ACCESS");
	HANDLE senderSemaphore = CreateSemaphore(NULL, 0, countOfSender, L"MESSAGES_COUNT_SEM");
	HANDLE mesReadEvent = CreateEvent(NULL, FALSE, FALSE, L"MESSAGE_READ");
	if (NULL == senderSemaphore || NULL == mesReadEvent || NULL == fileMutex)
	{
		std::cout << "Error";
		return GetLastError();
	}
	CreateProc(countOfSender, filename);
	CreateEv(countOfSender, filename);
	printf("Created %d senders.\n", countOfSender);
	WaitForMultipleObjects(countOfSender, readyEvents, TRUE, INFINITE);
	SetEvent(startALL);
	char tmp[MESSAGE_SIZE];
	char message[MESSAGE_SIZE];
	while (true) {
		std::cout << ">";
		std::cin >> tmp;
		if (std::cin.eof())
			break;
		WaitForSingleObject(senderSemaphore, INFINITE);
		WaitForSingleObject(fileMutex, INFINITE);
		printf("%s\n", receiveMessage(filename));
		ReleaseMutex(fileMutex);
		SetEvent(mesReadEvent);
	}
}