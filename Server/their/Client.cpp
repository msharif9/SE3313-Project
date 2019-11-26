#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;


class ClienThreadReader : public Thread
{	
	private:
		// Reference to our connected socket
		Socket& socket;
	
		// Reference to boolean flag for terminating the thread
		bool& term1;
		bool& term2;
		
		// Data to read from server
		ByteArray data;
		std::string data_str;

	public:
	
		~ClienThreadReader()
		{}
		
		ClienThreadReader(Socket& socket, bool& term1, bool& term2)
		: socket(socket), term1(term1), term2(term2)
		{}
		
		virtual long ThreadMain(void) 
		{
			std::cout.flush();
			while (!term1)
			{
				socket.Read(data);
				data_str = data.ToString();
				if (data_str == "") {}
				else 
				{
					std::cout << data_str << std::endl;
				}
				
			}
			std::cout << "Closing the ClientReader Thread" << std::endl;
			term2 = true;
			return 0;
		}
		
};


// This thread handles the connection to the server
class ClientThread : public Thread
{
private:
	//socket reference
	Socket& socket;

	//boolean flag for thread termination
	bool& term1;

	//connection boolean
	bool connected = false;
	
	ByteArray data;
	std::string data_str;


public:
	ClientThread(Socket& socket, bool& term1)
	: socket(socket), term1(term1)
	{}

	~ClientThread()
	{}

	void TryConnect()
	{
		try
		{
			std::cout << "Connecting...";
			socket.Open();
			connected = true;
			std::cout << "OK" << std::endl;
			std::cout << "First, enter your chatroom number please" << std::endl; 
		}
		catch (std::string exception)
		{
			std::cout << "FAIL (" << exception << ")" << std::endl;
			return;
		}
	}

	virtual long ThreadMain()
	{
		// Initially we need a connection
		while (true)
		{
			// Attempt to connect
			TryConnect();

			// Check if we are exiting or connection was established
			if (term1 || connected)
			{
				break;
			}

			// Try again every 5 seconds
			std::cout << "Trying again in 5 seconds" << std::endl;
			sleep(5);
		}
		while (!term1)
			{

                // Get the data
                std::cout.flush();
                data_str.clear();
				std::getline(std::cin, data_str);
                
                data = ByteArray(data_str); //convert to byte array
                
                if (data_str == "done")
                {
                    socket.Write(data);
                    std::cout << "Closing client" << std::endl;
                    term1 = true;
                    break;
                }
                
                
				
            
	

			}
		
		return 0;
	}
};

int main(void)
{
	std::cout << "SE3313 Project Client" << std::endl;

	// Create our socket
	Socket socket("127.0.0.1", 3000); //local host
	bool term1 = false;
    bool term2 = false;


	{
		// Thread to perform socket operations on
		ClientThread clientThread(socket, term1);
		ClienThreadReader clientThreadReader(socket, term1, term2);

		while(!term2) //wait for done
		{
			sleep(1);
		}
		
	}
	
	// Attempt to close the socket
	try
	{
		socket.Close();
	}
	catch (...)
	{
		
	}
	
	std::cout << "You are exiting the chatroom, goodbye!" << std::endl;

	return 0;
}
