using namespace std;

// **************************************************************************************
// * Echo Strings (echo_s.cc)
// * -- Accepts TCP connections and then echos back each string sent.
// **************************************************************************************
#include "echo_s.h"


// **************************************************************************************
// * processConnection()
// * - Handles reading the line from the network and sending it back to the client.
// * - Returns true if the client sends "QUIT" command, false if the client sends "CLOSE".
// **************************************************************************************
int processConnection(int inputSkt) {

  bool quitProgram = false;
  bool keepGoing = true;
  char buffer[1024];

  while (keepGoing) {

    //
    // Call read() call to get a buffer/line from the client.
    // Hint - don't forget to zero out the buffer each time you use it.
    //
    memset(buffer, 0, sizeof(buffer));
    ssize_t dataRead = read(inputSkt, buffer, sizeof(buffer)-1);
    if (LOG_LEVEL >= 4) cout << "DEBUG: Calling read(" << inputSkt<< ", " << &buffer << ", " << sizeof(buffer) << ")" << " (echo_s.cpp:28)" << endl;
    string stringCont = "";
    for (char toPush : buffer) {
      if (toPush != 0 && toPush != '\n') stringCont.push_back(toPush);
    }
    if (LOG_LEVEL >= 5) cout << "DEBUG: Received " << dataRead << " bytes, containing the string \"" << stringCont << "\" (echo_s.cpp:28)" << endl;

    //
    // Check for the commands
    //
    char quit[5];
    char close[6];
    memcpy(quit, &buffer[0], 4);
    memcpy(close, &buffer[0], 5);

    if(strcmp(quit, "QUIT") == 0) {
      if (LOG_LEVEL >= 0) cout << "DEBUG: QUIT command found, closing server (echo_s.cpp:44)" << endl;
      keepGoing = false;
      quitProgram = true;
      continue;
    }

    else if(strcmp(close, "CLOSE") == 0){
      if(LOG_LEVEL >= 0) cout << "DEBUG: CLOSE command found: Closing Connection... (echo_s.cpp)" <<endl;
      keepGoing = false;
      return 2;
    }

    //
    // Call write() to send line back to the client.
    //
    write(inputSkt, buffer, dataRead);
    if (LOG_LEVEL >= 4) cout << "DEBUG: Calling write(" << inputSkt << ", " << &buffer << ", " << dataRead << ") (echo_s.cpp:54)" << endl;
    if (LOG_LEVEL >= 5) cout << "DEBUG: Wrote " << dataRead << " bytes back to client (echo_s.cpp:54)" << endl;
  }

  return quitProgram;
}
    


// **************************************************************************************
// * main()
// * - Sets up the sockets and accepts new connection until processConnection() returns 1
// **************************************************************************************

int main (int argc, char *argv[]) {

  // ********************************************************************
  // * Process the command line arguments
  // ********************************************************************
  int opt = 0;
  LOG_LEVEL = -1;
  while ((opt = getopt(argc,argv,"d:")) != -1) {
    switch (opt) {
    case 'd':
      LOG_LEVEL = stoi(optarg);;
      break;
    case ':': 
    case '?':
    default:
      cout << "useage: " << argv[0] << " -d <num>" << endl;
      exit(-1);
    }
  }

  // *******************************************************************
  // * Creating the inital socket is the same as in a client.
  // ********************************************************************
  // Call socket() to create the socket you will use for lisening.
  int listenSckt = socket(AF_INET, SOCK_STREAM, 0);
  if (LOG_LEVEL >= 0) cout << "DEBUG: Calling Socket() assigned file descriptor " << listenSckt << " (echo_s.cpp:90)" << endl;
  
  // ********************************************************************
  // * The bind() and calls take a structure that specifies the
  // * address to be used for the connection. On the cient it contains
  // * the address of the server to connect to. On the server it specifies
  // * which IP address and port to lisen for connections.
  // ********************************************************************
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));

  // *** assign 3 fields in the servadd struct sin_family, sin_addr.s_addr and sin_port
  // *** the value your port can be any value > 1024.
  int portNum = 1025;
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY; 
  servaddr.sin_port = htons(portNum);       

  // ********************************************************************
  // * Binding configures the socket with the parameters we have
  // * specified in the servaddr structure.  This step is implicit in
  // * the connect() call, but must be explicitly listed for servers.
  // ********************************************************************
  bool bindSuccesful = false;
  while (!bindSuccesful) {
    // ** Call bind()
    // You may have to call bind multiple times if another process is already using the port
    // your program selects.
    if (LOG_LEVEL >= 1) cout << "DEBUG: Attempting bind on Port " << portNum << "...." <<endl;
    if (bind(listenSckt, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            cout << "Port in use, changing ports..." << endl;
            sleep(1);
            portNum++;
            servaddr.sin_port = htons(portNum); 
        } else {
            if (LOG_LEVEL >= 1) cout << "DEBUG: binding successful on port " << portNum << " (echo_s.cpp:124)" <<endl;
            bindSuccesful = true;
        }
  }
  // *** DON'T FORGET TO PRINT OUT WHAT PORT YOUR SERVER PICKED SO YOU KNOW HOW TO CONNECT.
  cout << "Using port: " << portNum << endl;

  // ********************************************************************
  // * Setting the socket to the listening state is the second step
  // * needed to being accepting connections.  This creates a queue for
  // * connections and starts the kernel listening for connections.
  // ********************************************************************
  int listenQueueLength = 1;
  // ** Cal listen()
  listen(listenSckt, listenQueueLength);

  // ********************************************************************
  // * The accept call will sleep, waiting for a connection.  When 
  // * a connection request comes in the accept() call creates a NEW
  // * socket with a new fd that will be used for the communication.
  // ********************************************************************
  int quitProgram = 0;
  int conSckt = 0;
  while (quitProgram == false) {
    // Call the accept() call to check the listening queue for connection requests.
    // If a client has already tried to connect accept() will complete the
    // connection and return a file descriptor that you can read from and
    // write to. If there is no connection waiting accept() will block and
    // not return until there is a connection.
    // ** call accept()
    conSckt = accept(listenSckt, nullptr, nullptr);
    if(LOG_LEVEL >= 2) cout << "DEBUG: Calling accept(" << listenSckt<< ", NULL, NULL) " << " (echo_s.cpp:154)" <<endl;
    if(LOG_LEVEL >= 3) cout << "DEBUG: We have received a connection on " << conSckt << " (echo_s.cpp:154)" << endl;

    // Now we have a connection, so you can call processConnection() to do the work.
    quitProgram = processConnection(conSckt);
    if (quitProgram == 2){
      close(conSckt);
      if(LOG_LEVEL == 6) cout << "Connection closed, waiting for new connection... (echo_s.cpp:171)" <<endl;
      quitProgram = 0;
    }
  }

  close(listenSckt);
  return 0;

}
