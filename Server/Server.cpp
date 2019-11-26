#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <algorithm>
#include "SharedObject.h"
#include "Semaphore.h"

struct ChatLog{
    std::string message;
};

using namespace Sync;
// This thread handles each client connection
class SocketThread : public Thread
{
private:
    // Reference to our connected socket
    Socket& socket;
    // The data we are receiving
    ByteArray data;
    char* temp;
public:
    bool flag;
    SocketThread(Socket& socket, std::string roomID)
    : socket(socket), flag(false)
    {}

    ~SocketThread()
    {}

    Socket& GetSocket()
    {
        return socket;
    }

    virtual long ThreadMain()
    {
        while(1)
        {
            try
            {
                // Wait for data
                socket.Read(data);
                
                // Perform operations on the data
                /*temp = new char[data.v.size()];
                for(int i = 0; i<data.v.size(); i++) {
                    temp[i] = data.v[i];
                    data.v[i] = toupper(temp[i]); 
                }*/
                std::string a = temp;
                std::cout<<a<<std::endl;
                
                // Send it back
                socket.Write(data);
                delete temp;
            }
            catch (...)
            {
                // ???
                std::cout << "could not read socket data" << std::endl;
            }
            if(flag)
                break;
        }
		
	// ???

        return 0;
    }
};

// This thread handles the server operations
class ServerThread : public Thread
{
private:
    SocketServer& server;
    bool terminate = false;
    bool found;
    std::vector<SocketThread*> threads;
    std::vector<Socket*> sockets;
    std::vector<Shared<ChatLog>> chatrooms;
    std::vector<std::string> roomID;
    std::string id;
    ByteArray login;
public:
    bool flag;
    ServerThread(SocketServer& server)
    : server(server), flag(false), found(false)
    {}

    ~ServerThread()
    {
        // Cleanup
        
        for(int i = sockets.size()-1; i>0;i--){
            sockets[i]->Close();
            sockets.pop_back();
        }
        for(int i = threads.size()-1; i>0;i--){
            threads[i]->flag = true;
            delete threads[i];
            threads.pop_back();
        }
        
    }

    virtual long ThreadMain()
    {
        do{
            // Wait for a client socket connection
            Socket* newConnection = new Socket(server.Accept());
            sockets.push_back(newConnection);
            newConnection->Read(login);
            id = login.ToString();
            for(int i = 0; roomID.size();i++){
                if(roomID[i] == id){
                    found = true;
                }
            }
            std::cout << id << std::endl;
            if(!found){
                Shared<ChatLog> a(login.ToString(), true);
                Semaphore guard(id+'g',true);
                Semaphore wsem(id+'w',true);
                roomID.push_back(id);
                chatrooms.push_back(a);
            }
            else
                found = false;

            // Pass a reference to this pointer into a new socket thread
            Socket& socketReference = *newConnection;
            threads.push_back(new SocketThread(socketReference, id));
        }while(!flag);
        return 0;
    }
};


int main(void)
{
    std::cout << "I am a server." << std::endl;
	std::cout << "Press enter to terminate the server...";
    std::cout.flush();
	
    // Create our server
    SocketServer server(3000);    

    // Need a thread to perform server operations
    ServerThread serverThread(server);
	
    // This will wait for input to shutdown the server
    FlexWait cinWaiter(1, stdin);
    cinWaiter.Wait();
    std::cin.get();
    // Shut down and clean up the server
    serverThread.flag = true;
    server.Shutdown();

}
