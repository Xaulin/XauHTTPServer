#include "Socket.h"

namespace x{
	int msgLoop(){
		MSG msg;
		while (GetMessage(&msg, 0, 0, 0)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return msg.wParam;
	}
	void Socket::startup(){
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);

		WNDCLASS cls = { 0 };
		cls.lpfnWndProc = wndproc;
		cls.lpszClassName = L"cls";
		RegisterClass(&cls);
		hwnd = CreateWindow(L"cls", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	}
	unsigned long Socket::hostName(const char* name){
		return *(ULONG*)gethostbyname(name)->h_addr_list[0];
	}
	unsigned long Socket::hostAddr(const char* addr){
		return inet_addr(addr);
	}

	Socket::Socket() : s(0), asyncParam(0){
	}
	Socket::Socket(unsigned long hostAddr, int port) : s(0), asyncParam(0){
		connect(hostAddr, port);
		sockets.push_back(*this);
	}
	Socket::Socket(int port) : s(0), asyncParam(0){
		listen(port);
		sockets.push_back(*this);
	}

	x::Socket& Socket::operator<<(const std::string& str){
		send(s, str.c_str(), str.size(), 0);
		return *this;
	}
	x::Socket& Socket::operator>>(std::string& str){
		char buf[512];
		while (1){
			int len = recv(s, buf, 511, 0);
			if (len == SOCKET_ERROR || len == 0)
				break;
			buf[len] = 0;
			str += buf;
		}
		return *this;
	}

	void Socket::async(NewConnectionEvent event){
		asyncParam |= FD_ACCEPT;
		WSAAsyncSelect(s, hwnd, WM_SOCKET, asyncParam);
		newConnectionEvent = event;
	}
	void Socket::async(DisconnectionEvent event){
		asyncParam |= FD_CLOSE;
		WSAAsyncSelect(s, hwnd, WM_SOCKET, asyncParam);
		disconnectionEvent = event;
	}
	void Socket::async(ReadEvenet event){
		asyncParam |= FD_READ;
		WSAAsyncSelect(s, hwnd, WM_SOCKET, asyncParam);
		readEvenet = event;
	}

	void Socket::connect(unsigned long hostAddr, int port){
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		addr.sin_addr.s_addr = hostAddr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		::connect(s, (SOCKADDR*)&addr, sizeof(addr));
	}
	void Socket::listen(int port){
		s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		::bind(s, (SOCKADDR*)&addr, sizeof(addr));
		::listen(s, SOMAXCONN);
	}
	x::Socket Socket::accept(){
		Socket socket;
		int len = sizeof(addr);
		socket.s = ::accept(s, (SOCKADDR*)&socket.addr, &len);
		sockets.push_back(socket);
		return socket;
	}

	char* Socket::getIp(){
		return inet_ntoa(addr.sin_addr);
	}

	void Socket::close(){
		closesocket(s);
	}
	LRESULT CALLBACK Socket::wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
		switch (msg){
		case WM_SOCKET:
			if (HIWORD(lparam))
				break;
			else switch (LOWORD(lparam)){
			case FD_ACCEPT:
				if (newConnectionEvent){
					Socket socket;
					int len = sizeof(socket.addr);
					socket.s = ::accept(wparam, (SOCKADDR*)&socket.addr, &len);
					if (newConnectionEvent(socket))
						sockets.push_back(socket);
					else
						closesocket(socket.s);
				}
				break;
			case FD_CLOSE:
				if (disconnectionEvent)
					for (auto& o : sockets)
						if (o.s == wparam){
							disconnectionEvent(o);
							//o.close();
							break;
						}
				break;
			case FD_READ:
				if (readEvenet){
					char buf[1024];
					int len = recv(wparam, buf, 1023, 0);
					buf[len] = 0;
					for (auto& o : sockets)
						if (o.s == wparam){
							readEvenet(o, buf);
							break;
						}
				}
				break;
			}
			break;

		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		return 0;
	}

	std::vector<x::Socket> Socket::sockets;
	HWND Socket::hwnd;
	Socket::NewConnectionEvent Socket::newConnectionEvent = 0;
	Socket::DisconnectionEvent Socket::disconnectionEvent = 0;
	Socket::ReadEvenet Socket::readEvenet = 0;
}