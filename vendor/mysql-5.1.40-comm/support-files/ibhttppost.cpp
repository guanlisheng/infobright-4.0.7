#include <iostream>
#include <string>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include <arpa/inet.h>

#if !defined(__WIN__) && !defined(HAVE_BROKEN_NETINET_INCLUDES) 
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#if !defined(alpha_linux_port)
#include <netinet/tcp.h>
#endif
#endif

#if defined(__WIN__)
#define O_NONBLOCK 1    /* For emulation of fcntl() */

/*
  SHUT_RDWR is called SD_BOTH in windows and
  is defined to 2 in winsock2.h
  #define SD_BOTH 0x02
*/
#define SHUT_RDWR 0x02

#endif

/*
  On OSes which don't have the in_addr_t, we guess that using uint32 is the best
  possible choice. We guess this from the fact that on HP-UX64bit & FreeBSD64bit
  & Solaris64bit, in_addr_t is equivalent to uint32. And on Linux32bit too.
*/

/* On some operating systems (e.g. Solaris) INADDR_NONE is not defined */
#ifndef INADDR_NONE
#define INADDR_NONE -1                          /* Error value from inet_addr */
#endif

#ifdef __GNUC__
#include <netdb.h>
#endif
#ifndef __GNUC__
#include <winsock2.h>
#endif

#ifndef __GNUC__

/**
 * Include files needed
 */
#include <winsock2.h>
#include <ws2tcpip.h>

#define InetErrno WSAGetLastError()
#define EWOULDBLOCK WSAEWOULDBLOCK
#define NDB_SOCKET_TYPE SOCKET
#define NDB_INVALID_SOCKET INVALID_SOCKET
#define _NDB_CLOSE_SOCKET(x) closesocket(x)

#else

/**
 * Include files needed
 */
#include <netdb.h>

#define NDB_NONBLOCK O_NONBLOCK
#define NDB_SOCKET_TYPE int
#define NDB_INVALID_SOCKET -1
#define _NDB_CLOSE_SOCKET(x) ::close(x)

#define InetErrno errno

#endif

using namespace std;

#define NODETAILS 0
#define VERBOSE 1

int mode = NODETAILS;

//#define _TEST

extern "C"
int
read_socket(NDB_SOCKET_TYPE socket, int timeout_millis, 
	    char * buf, int buflen){
  if(buflen < 1)
    return 0;
  
  fd_set readset;
  FD_ZERO(&readset);
  FD_SET(socket, &readset);
  
  struct timeval timeout;
  timeout.tv_sec  = (timeout_millis / 1000);
  timeout.tv_usec = (timeout_millis % 1000) * 1000;

  const int selectRes = select(socket + 1, &readset, 0, 0, &timeout);
  if(selectRes == 0)
    return 0;
  
  if(selectRes == -1){
    return -1;
  }

  return recv(socket, &buf[0], buflen, 0);
}

extern "C"
int
write_socket(NDB_SOCKET_TYPE socket, int timeout_millis, 
	     const char buf[], int len, int mode){
  if (mode == VERBOSE)
    printf("write begin\n");

  fd_set writeset;
  FD_ZERO(&writeset);
  FD_SET(socket, &writeset);
  struct timeval timeout;
  timeout.tv_sec  = (timeout_millis / 1000);
  timeout.tv_usec = (timeout_millis % 1000) * 1000;
  if (mode == VERBOSE)
    printf("w1\n");
  const int selectRes = select(socket + 1, 0, &writeset, 0, &timeout);
  if (selectRes != 1) {
    if (mode == VERBOSE)
      printf("failed on select\n");
    return -1;
  }
  if (mode == VERBOSE)
    printf("after select\n");
  const char * tmp = &buf[0];
  if (mode == VERBOSE)
    printf("w2\n");
  while(len > 0){
    if (mode == VERBOSE)
      printf("before send, len=%d, date=%s\n",len, tmp);
    const int w = send(socket, tmp, len, 0);
    if (mode == VERBOSE)
      printf("Wrote sofar =%d\n",w);

    if(w == -1){
      return -1;
    }
    len -= w;
    tmp += w;
    
    if(len == 0)
      break;
    
    FD_ZERO(&writeset);
    FD_SET(socket, &writeset);
    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;
    const int selectRes = select(socket + 1, 0, &writeset, 0, &timeout);
    if(selectRes != 1){
      return -1;
    }
  }
  if (mode == VERBOSE)
    printf("End write\n");
 
  return 0;
}

/* Define some useful general macros */
#if !defined(max)
#define max(a, b)	((a) > (b) ? (a) : (b))
#define min(a, b)	((a) < (b) ? (a) : (b))
#endif

static const unsigned int MAX_READ_TIMEOUT = 5000 ;
static const unsigned int MAX_WRITE_TIMEOUT = 100 ;

extern "C"
int 
Ndb_getInAddr(struct in_addr * dst, const char *address) {
  //  DBUG_ENTER("Ndb_getInAddr");
  {
    int tmp_errno;
    struct hostent tmp_hostent, *hp;
    char buff[1024];
    
    hp = gethostbyname(address);
    if (hp)
    {
      memcpy(dst, hp->h_addr, min(sizeof(*dst), (size_t) hp->h_length));
      return 0; //DBUG_RETURN(0);
    }
  }
  /* Try it as aaa.bbb.ccc.ddd. */
  dst->s_addr = inet_addr(address);
  if (dst->s_addr != 
#ifdef INADDR_NONE
      INADDR_NONE
#else
      -1
#endif
      )
  {
    return 0; //DBUG_RETURN(0);
  }
  //  DBUG_PRINT("error",("inet_addr(%s) - %d - %s",
  //		      address, errno, strerror(errno)));
  return -1; //DBUG_RETURN(-1);
}

#ifndef __GNUC__

class INIT_WINSOCK2
{
public:
    INIT_WINSOCK2(void);
    ~INIT_WINSOCK2(void);

private:
    bool m_bAcceptable;
};

INIT_WINSOCK2 g_init_winsock2;

INIT_WINSOCK2::INIT_WINSOCK2(void)
: m_bAcceptable(false)
{
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    
    wVersionRequested = MAKEWORD( 2, 2 );
    
    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 ) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        m_bAcceptable = false;
    }
    
    /* Confirm that the WinSock DLL supports 2.2.*/
    /* Note that if the DLL supports versions greater    */
    /* than 2.2 in addition to 2.2, it will still return */
    /* 2.2 in wVersion since that is the version we      */
    /* requested.                                        */
    
    if ( LOBYTE( wsaData.wVersion ) != 2 ||
        HIBYTE( wsaData.wVersion ) != 2 ) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        WSACleanup( );
        m_bAcceptable = false; 
    }
    
    /* The WinSock DLL is acceptable. Proceed. */
    m_bAcceptable = true;
}

INIT_WINSOCK2::~INIT_WINSOCK2(void)
{
    if(m_bAcceptable)
    {
        m_bAcceptable = false;
        WSACleanup();
    }
}
#endif

class SocketClient
{
  NDB_SOCKET_TYPE m_sockfd;
  struct sockaddr_in m_servaddr;
  unsigned short m_port;
  char *m_server_name;
public:
  SocketClient(const char *server_name, unsigned short port);
  ~SocketClient();
  bool init();
  void set_port(unsigned short port) {
    m_port = port;
    m_servaddr.sin_port = htons(m_port);
  };
  unsigned short get_port() { return m_port; };
  char *get_server_name() { return m_server_name; };
  int bind(const char* toaddress, unsigned short toport);
  NDB_SOCKET_TYPE connect(const char* toaddress = 0, unsigned short port = 0);
  bool close();
};

SocketClient::SocketClient(const char *server_name, unsigned short port)
{
  m_port= port;
  m_server_name= server_name ? strdup(server_name) : 0;
  m_sockfd= NDB_INVALID_SOCKET;
}

SocketClient::~SocketClient()
{
  if (m_server_name)
    free(m_server_name);
  if (m_sockfd != NDB_INVALID_SOCKET)
    _NDB_CLOSE_SOCKET(m_sockfd);
}

bool
SocketClient::init()
{
  if (m_sockfd != NDB_INVALID_SOCKET)
    _NDB_CLOSE_SOCKET(m_sockfd);

  if (m_server_name)
  {
    memset(&m_servaddr, 0, sizeof(m_servaddr));
    m_servaddr.sin_family = AF_INET;
    m_servaddr.sin_port = htons(m_port);
  }
  
  m_sockfd= socket(AF_INET, SOCK_STREAM, 0);
  if (m_sockfd == NDB_INVALID_SOCKET) {
    return false;
  }
  
  return true;
}

int
SocketClient::bind(const char* bindaddress, unsigned short localport)
{
  if (m_sockfd == NDB_INVALID_SOCKET)
    return -1;

  struct sockaddr_in local;
  memset(&local, 0, sizeof(local));
  local.sin_family = AF_INET;
  local.sin_port = htons(localport);

  struct hostent * host_addr = gethostbyname(bindaddress);
  if(host_addr==NULL) {
    return -103;
  }
  local.sin_addr.s_addr = *((int*)*host_addr->h_addr_list) ;
 
  const int on = 1;
  if (setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, 
		 (const char*)&on, sizeof(on)) == -1) {

    int ret = errno;
    _NDB_CLOSE_SOCKET(m_sockfd);
    m_sockfd= NDB_INVALID_SOCKET;
    return errno;
  }
  
  if (::bind(m_sockfd, (struct sockaddr*)&local, sizeof(local)) == -1) 
  {
    int ret = errno;
    _NDB_CLOSE_SOCKET(m_sockfd);
    m_sockfd= NDB_INVALID_SOCKET;
    return ret;
  }
  
  return 0;
}

NDB_SOCKET_TYPE
SocketClient::connect(const char *toaddress, unsigned short toport)
{
  if (m_sockfd == NDB_INVALID_SOCKET)
  {
    if (!init()) {
#ifdef VM_TRACE
      ndbout << "SocketClient::connect() failed " << m_server_name << " " << m_port << endl;
#endif
      return NDB_INVALID_SOCKET;
    }
  }

  if (toaddress)
  {
    if (m_server_name)
      free(m_server_name);
    m_server_name = strdup(toaddress);
    m_port = toport;
    memset(&m_servaddr, 0, sizeof(m_servaddr));
    m_servaddr.sin_family = AF_INET;
    m_servaddr.sin_port = htons(toport);
    // Convert ip address presentation format to numeric format
    if (Ndb_getInAddr(&m_servaddr.sin_addr, m_server_name))
      return NDB_INVALID_SOCKET;
  }
  
  const int r = ::connect(m_sockfd, (struct sockaddr*) &m_servaddr, sizeof(m_servaddr));
  if (r == -1) {
    _NDB_CLOSE_SOCKET(m_sockfd);
    m_sockfd= NDB_INVALID_SOCKET;
    return NDB_INVALID_SOCKET;
  }

  NDB_SOCKET_TYPE sockfd= m_sockfd;
  m_sockfd= NDB_INVALID_SOCKET;

  return sockfd;
}

int checkInternetAccess(string httpserver, int portno)
{
  NDB_SOCKET_TYPE sockfd= NDB_INVALID_SOCKET;
  SocketClient s(0, 0);
  if (!s.init())
  {
	return -1;
  }

  if ((sockfd=s.connect(httpserver.c_str(), portno)) == NDB_INVALID_SOCKET)
  {
    return -1;
  }

  return 0;
}

int userregistration(string httpserver, int portno, string httppage)
{
  string htmldata = "" , htmlpost = "";
  char htmlresponse[1024]= "";
  char fname[256], lname[256], email[256], company[256], title[256], city[256];
  char state[256], country[256], phone[256], hear[256], mysql[256];
  int rd = 0, wr = 0;

  int ii, jj;
  int nindustries;
  int cindustry = 0;
  char stemp1[256];
  string scindustry = "";

  int nproject;
  int cproject = 0;
  char stemp2[256];
  string sproject = "";

  #define MAXINDUSTRIES 26
  const char industries [MAXINDUSTRIES][256] = {
    {"Aerospace and Defense"},
    {"Automotive and Transportation"},
    {"Charity, Non-profit Organization"},
    {"Chemicals"},
    {"Computer Hardware"},
    {"Computer Software and Services"},
    {"Consumer Products"},
    {"Diversified Services (Accounting, Legal)"},
    {"Education"},
    {"Electronics"},
    {"Energy, Utilities, Oil and Gas"},
    {"Engineering, Construction, Materials"},
    {"Financial Services, Banking, Insurance"},
    {"Food and Beverages (Consumables)"},
    {"Government"},
    {"Healthcare"},
    {"Leisure, Entertainment, Restaurants"},
    {"Manufacturing"},
    {"Media"},
    {"Metals and Mining"},
    {"Online advertising and Marketing"},
    {"Pharmaceuticals and Medicine"},
    {"Retail"},
    {"Telecommunications"},
    {"Transportation"},
    {"Other"} };

  #define MAXPROJECTS   7
  const char projects [MAXPROJECTS][256] = {
    {"Enterprise Data Warehouse"},
    {"Department or Application Data Mart"},
    {"Log Analytics"},
    {"Online Analytics"},
    {"Telecom Analytics"},
    {"Embedded Analytic Database"},
    {"Other"} };

  // ice registration questions. Take answers and http post to infobright web server

  // hidden field and value
  htmldata += "formid=";
  htmldata += "85c6c36f-b706-4c14-93c8-04217c588826";

  // hidden field and value
  htmldata += "&cid=";
  htmldata += "LF_1b4a8584";

  // hidden field and value
  htmldata += "&lead_source=";
  htmldata += "ICE Registration (v2) via Command Line Tool";

first:
  cout << "First Name: ";
  cin.getline ( fname, 256, '\n' ); 
  if (!strlen(fname)) {
    cout << "First name can not be empty! Please give a valid First Name.\n";
	goto first;
  }
  htmldata += "&first_name=";
  htmldata += fname;

last:
  cout << "Last Name: ";
  cin.getline ( lname, 256, '\n' ); 
  if (!strlen(lname)) {
    cout << "Last name can not be empty. Please give a valid Last Name.\n";
	goto last;
  }

  htmldata += "&last_name=";
  htmldata += lname;

comp:
  cout << "Company: ";
  cin.getline ( company, 256, '\n' ); 
  if (!strlen(company)) {
    cout << "Company name can not be empty. Please give a valid Company Name.\n";
	goto comp;
  }
  htmldata += "&company=";
  htmldata += company;

  cout << "Job Title: ";
  cin.getline ( title, 256, '\n' ); 
  htmldata += "&title=";
  htmldata += title;

cit:
  cout << "City: ";
  cin.getline ( city, 256, '\n' ); 
  if (!strlen(city)) {
    cout << "City name can not be empty. Please give a valid City Name.\n";
	goto cit;
  }
  htmldata += "&city=";
  htmldata += city;

statp:
  cout << "State/Province: ";
  cin.getline ( state, 256, '\n' ); 
  if (!strlen(state)) {
    cout << "State/Province name can not be empty. Please give a valid State/Province Name.\n";
	goto statp;
  }
  htmldata += "&state=";
  htmldata += state;

countr:
  cout << "Country: ";
  cin.getline ( country, 256, '\n' );
  if (!strlen(country)) {
    cout << "Country name can not be empty. Please give a valid Country Name.\n";
	goto countr;
  }
  htmldata += "&country=";
  htmldata += country;

emailaddres:
  cout << "Email address: ";
  cin.getline ( email, 256, '\n' );
  if (!strlen(email)) {
    cout << "Email address can not be empty. Please give a valid Email Address.\n";
	goto emailaddres;
  }
  if (strchr( email, '@') == NULL) {
	cout << "Invalid email address. Please give a valid Email Address.\n";
    goto emailaddres;
  }
  htmldata += "&email=";
  htmldata += email;

  cout << "Phone number: ";
  cin.getline ( phone, 256, '\n' ); 
  htmldata += "&phone=";
  htmldata += phone;

  cout << "Industry:\n";
  for (ii = 0; ii< MAXINDUSTRIES-1; ii++)
    printf("%d. %s\n", ii+1, industries[ii]);
  printf("%d. %s", ii+1, industries[ii]);

  printf("\nEnter number here[%d-%d]: ",1, MAXINDUSTRIES);
indus:
  cin.getline(stemp1, 256, '\n' ); 
  if (!strlen(stemp1)) 
  {
    printf("Please give a valid number for industry[%d-%d]: ",1, MAXINDUSTRIES);
    goto indus;
  }
  if (strcmp(stemp1, ""))
    sscanf(stemp1, "%d", &cindustry);
  if (cindustry <1 || cindustry >MAXINDUSTRIES)
  {
    printf("Please give a valid number for industry[%d-%d]: ",1, MAXINDUSTRIES);
    goto indus;
  }
  htmldata += "&industry=";
  htmldata += industries[cindustry-1];

  cout << "What kind of project do you intend to use ICE for?:\n";
  for (jj = 0; jj< MAXPROJECTS-1; jj++)
    printf("%d. %s\n", jj+1, projects[jj]);
  printf("%d. %s", jj+1, projects[jj]);

  printf("\nEnter number here[%d-%d]: ", 1, MAXPROJECTS);
proj:
  cin.getline(stemp2, 256, '\n'); 
  if (!strlen(stemp2)) 
  {
    printf("Please give a valid number for project: ", 1, MAXPROJECTS);
    goto proj;
  }
  if (strcmp(stemp2, ""))
    sscanf(stemp2, "%d", &cproject);
  if (cproject <1 || cproject >MAXPROJECTS)
  {
    printf("Please give a valid number for project: ", 1, MAXPROJECTS);
    goto proj;
  }
  htmldata += "&project=";
  htmldata += projects[cproject-1];

  char slen[64];
  sprintf(slen, "%d", htmldata.size());

  htmlpost = "POST ";
  htmlpost = htmlpost + httppage;
  htmlpost = htmlpost + " HTTP/1.1\r\n";
  htmlpost = htmlpost + "Host: ";
  htmlpost = htmlpost + httpserver;
  htmlpost = htmlpost + "\r\n\
User-Agent: Mozilla/4.0\r\n\
Content-Length: ";
  htmlpost = htmlpost + slen;
  htmlpost = htmlpost + "\r\n";
  htmlpost = htmlpost + "Referer: http://www.infobright.org/Downloads/ICE/\r\n";
  htmlpost = htmlpost + "Content-Type: application/x-www-form-urlencoded\r\n";
  htmlpost = htmlpost + "\r\n";
  htmlpost = htmlpost + htmldata;
  htmlpost = htmlpost + "\r\n";

#ifdef _TEST
  printf("htmldata=%s", htmldata.c_str());
  printf("\nhtmlpost=%s", htmlpost.c_str());
  return 0;
#endif
  int retry = 0;

tryagain:

  NDB_SOCKET_TYPE sockfd = NDB_INVALID_SOCKET;
  SocketClient s(0, 0);
  if (!s.init()) {
	return -1;
  }

  if ((sockfd=s.connect(httpserver.c_str(), portno)) == NDB_INVALID_SOCKET) {
    return -1;
  }

  if (mode == VERBOSE)
    printf("http request ===>\n%s",htmlpost.c_str());

  if (mode == VERBOSE)
    printf("trying=%d\n", retry);

  if ((wr = write_socket(sockfd, MAX_WRITE_TIMEOUT, htmlpost.c_str(), htmlpost.size(),mode)) == -1) {
    if (mode == VERBOSE)
      printf("error on write = %d\n", wr);
    return -1;
  }

  if (mode == VERBOSE)
    printf("wrote = %d\n", wr);

  sleep(2);
  if ((rd = read_socket(sockfd, MAX_READ_TIMEOUT, htmlresponse, 1024)) == -1) {
    if (mode == VERBOSE)
      printf("error on read = %d\n", rd);
    return -1;
  }
  retry ++;
  if (mode == VERBOSE)
    printf("retry = %d, rd = %d\n", retry, rd);
  if (rd == 0 && retry <= 5)
    goto tryagain;

  if (rd ==0)
    return -1;

  if (mode == VERBOSE)
  	printf("http response, read=%d bytes, response=>%s\n", rd, htmlresponse);

  return 0;
}

void usage()
{
	printf ("Wrong parameters. Example parameters are: \n\t \
--checkinternet httpserver port httppage --verbose \n\t \
--register httpserver port httppage --verbose\n\t \
example command: ./ibhttppost --register loopfuse.net 80 /webrecorder/post\n");
};

int main(int argc, char* argv[])
{
  string httpserver;
  int httpport;
  string httppage;

#ifdef _TEST
userregistration(httpserver, httpport, httppage);
#endif
  if (argc > 4) {
    httpserver = argv[2];
	httpport = atoi(argv[3]);
	httppage = argv[4];
	
	if (argc > 5) {
		mode = VERBOSE;
		printf("Remote host %s, port %d, page = %s\n", httpserver.c_str(), httpport, httppage.c_str());
	}

	if (!strcmp(argv[1], "--checkinternet")) {
	  return checkInternetAccess(httpserver, httpport);
	}
    else if (!strcmp(argv[1], "--register")) {
	  return userregistration(httpserver, httpport, httppage);
	}
  }
  usage();
  return -1;
}
