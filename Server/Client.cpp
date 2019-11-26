
#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;

// This thread handles the connection to the server
class ClientThread : public Thread
{
private:
	// Reference to our connected socket
	Socket& socket;

	// Data to send to server
	ByteArray data, close;
	std::string data_str;
public:
	bool flag;
	ClientThread(Socket& socket)
	: socket(socket), flag(false)
	{}

	~ClientThread()
	{}

	virtual long ThreadMain()
	{
		int result;
		try{
			result = socket.Open();
		}
		catch(...){
			std::cout << "Could not connect to server" << std::endl;
			flag = true;
			return -1;
		}
		
		do{
			std::cout << "Please input your data (done to exit): ";
			std::cout.flush();

			// Get the data
			std::getline(std::cin, data_str);
			data = ByteArray(data_str);
						
			// Write to the server
			socket.Write(data);

			// Get the response
			socket.Read(data);
			//...
			if(data_str != "done")
				std::cout << data.ToString() << std::endl;
			else{
				std::cout << "Connection Closed" << std::endl;
				flag = true;	
			}
		}while(!flag);
		
		return 0;
	}
};

int main(void)
{
	// Welcome the user 
	std::cout << "SE3313 Lab 3 Client" << std::endl;

	// Create our socket
	Socket socket("127.0.0.1", 3000);
	ClientThread clientThread(socket);
	while(!clientThread.flag)
	{
		sleep(1);
	}
	socket.Close();

	return 0;
}
