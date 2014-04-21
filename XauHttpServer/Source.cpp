#include "Socket.h"

void read(x::Socket& socket, const std::string& str){
	printf("<%s:%d>: %s\n", socket.getIp(), socket.s, str.c_str());
}
void disCon(x::Socket& socket){
	printf("<%s:%d>: Disconnected\n", socket.getIp(), socket.s);
}
bool newCon(x::Socket& socket){
	printf("<%s:%d>: Connected\n", socket.getIp(), socket.s);
	socket.async(read);
	socket.async(disCon);
	return true;
}

int main(){
	x::Socket::startup();
	x::Socket server(666);
	server.async(newCon);

	return x::msgLoop();
}