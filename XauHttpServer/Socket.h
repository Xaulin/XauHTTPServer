#pragma once
#include <string>
#include <vector>
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")
#define WM_SOCKET 104

namespace x{
	int msgLoop();
	class Socket{
	public:
		typedef bool(*NewConnectionEvent)(x::Socket&);
		typedef void(*DisconnectionEvent)(x::Socket&);
		typedef void(*ReadEvenet)(x::Socket&, const std::string&);

		static std::vector<x::Socket> sockets;
		static void startup();
		static unsigned long hostName(const char* name);
		static unsigned long hostAddr(const char* addr);
		Socket();
		Socket(unsigned long hostAddr, int port);
		Socket(int port);
		x::Socket& operator<<(const std::string& str);
		x::Socket& operator>>(std::string& str);

		void async(NewConnectionEvent event);
		void async(DisconnectionEvent event);
		void async(ReadEvenet event);

		void connect(unsigned long hostAddr, int port);
		void listen(int port);
		x::Socket accept();

		char* getIp();
		void close();

		SOCKET s;
		long asyncParam;

	private:
		static LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		static ::HWND hwnd;
		static NewConnectionEvent newConnectionEvent;
		static DisconnectionEvent disconnectionEvent;
		static ReadEvenet readEvenet;

		struct{
			sockaddr_in addr;
		};
	};
}