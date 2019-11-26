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
    char* message;
};
int readcount = 0;

using namespace Sync;
// This thread handles each client connection
class SocketThreadReader : public Thread
{
private:
    // Reference to our connected socket
    Socket& socket;
    // The data we are receiving
    ByteArray data;
    char* temp;
    int readcount;
    std::string roomID;
public:
    bool flag;
    SocketThreadReader(Socket& socket, std::string roomID)
    : socket(socket), flag(false), readcount(0), roomID(roomID)
    {}

    ~SocketThreadReader()
    {}

    Socket& GetSocket()
    {
        return socket;
    }

    ByteArray encode(std::string message){
        ByteArray n;
        for(int i = 0;i<message.length;i++){
            n.v[i] = message[i];
        }
        return n;
    }

    virtual long ThreadMain()
    {
        while(1)
        {
            try
            {
                // Wait for data
                Semaphore guard(roomID+'g');
                Semaphore write(roomID+'w');
                Semaphore buffer(roomID+'b');
                Shared<ChatLog> room(roomID);
                guard.Wait();
                readcount++;
                if(readcount == 1)
                    write.Wait();
                guard.Signal();    

                temp = room->message;
                buffer.Wait();
                socket.Write(encode(temp));
                buffer.Signal();


                guard.Wait();
                readcount--;
                if(readcount == 0)
                    write.Signal();
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

class SocketThreadWriter : public Thread
{
private:
    // Reference to our connected socket
    Socket& socket;
    // The data we are receiving
    ByteArray data;
    char* temp;
    std::string roomID, temp;
public:
    bool flag;
    SocketThreadWriter(Socket& socket, std::string roomID)
    : socket(socket), flag(false), roomID(roomID)
    {}

    ~SocketThreadWriter()
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
                Semaphore guard(roomID+'g');
                Semaphore write(roomID+'w');
                Semaphore buffer(roomID+'b');
                Shared<ChatLog> room(roomID);
                guard.Wait();
                readcount++;
                if(readcount == 1)
                    write.Wait();
                guard.Signal();    

                temp = room->message;
                buffer.Wait();
                socket.Write(encode(temp));
                buffer.Signal();


                guard.Wait();
                readcount--;
                if(readcount == 0)
                    write.Signal();




                
                // Perform operations on the data
                /*temp = new char[data.v.size()];
                for(int i = 0; i<data.v.size(); i++) {
                    temp[i] = data.v[i];
                    data.v[i] = toupper(temp[i]); 
                }*/
                std::string a = temp;
                std::cout<<a<<std::endl;
                
                // Send it back
                //socket.Write(data);
                //delete temp;
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
    std::vector<SocketThreadReader*> readers;
    std::vector<SocketThreadWriter*> writers;
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
        for(int i = readers.size()-1; i>0;i--){
            readers[i]->flag = true;
            delete readers[i];
            readers.pop_back();
        }
        for(int i = writers.size()-1; i>0;i--){
            writers[i]->flag = true;
            delete writers[i];
            writers.pop_back();
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
                Semaphore guard(id+'g',1,true);
                Semaphore wsem(id+'w',1,true);
                Semaphore buffer(id+'b',0,true);
                roomID.push_back(id);
                chatrooms.push_back(a);
            }
            else
                found = false;

            // Pass a reference to this pointer into a new socket thread
            Socket& socketReference = *newConnection;
            readers.push_back(new SocketThreadReader(socketReference, id));
            writers.push_back(new SocketThreadWriter(socketReference, id));
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
