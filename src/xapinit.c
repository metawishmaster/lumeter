#include "xapdef.h"

#include <libgen.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <linux/sockios.h>
//#include <linux/if.h>
//#include <linux/in.h>
//#include <asm-generic/fcntl.h>

int g_debuglevel=0;

#ifdef WIN32
void cfmakeraw(struct termios *termios_p) {

	termios_p->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
		|INLCR|IGNCR|ICRNL|IXON);
	termios_p->c_oflag &= ~OPOST;
	termios_p->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	termios_p->c_cflag &= ~(CSIZE|PARENB);
	termios_p->c_cflag |= CS8;
}
#endif


int xap_parse_commandline(int argc, char* argv[], int base)
{
	if (argc<1+base) {
		// set default instance name
		strcpy(g_instance, XAP_DEFAULT_INSTANCE); // default instance name
	} else 
	{
		strncpy(g_instance, basename(argv[base]), sizeof(g_instance)-1);
	}

	if (argc<2+base) {
		// set default interface
		strcpy(g_interfacename, "eth0");
	} else
	{
		// get chosen interface
		strncpy(g_interfacename, argv[base+1], sizeof(g_interfacename)-1);
	}

	if (argc<3+base) {
		// set default interface port
		g_interfaceport=3639; 
	} else
	{
		// get chosen interface port
		g_interfaceport=atoi(argv[base+2]);
	}

	if (argc<4+base) {
		// set default debug level
		g_debuglevel=0; 
	} else
	{
		// get chosen interface port
		g_debuglevel=atoi(argv[base+3]);

	}

	return 1;
}


int xap_discover_broadcast_network(int* a_sender_sockfd, struct sockaddr_in* a_sender_address)
{
	int i; 
	long int i_inverted_netmask;
	struct ifreq i_interface;
	struct sockaddr_in  i_mybroadcast;
	struct sockaddr_in i_myinterface;
	struct sockaddr_in i_mynetmask;
	int i_optval, i_optlen;

	// Discover the broadcast network settings
	*a_sender_sockfd=socket(AF_INET, SOCK_DGRAM, 0);

	i_optval=1;
	i_optlen=sizeof(int);
	if (setsockopt(*a_sender_sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&i_optval, i_optlen)) {
		printf("Cannot set options on broadcast socket\n");
		return 0;
	}


	// Query the low-level capabilities of the network interface
	// we are to use. If none passed on command line, default to
	// eth0.

	memset((char*)&i_interface, 0, sizeof(i_interface));

	i_interface.ifr_addr.sa_family = AF_INET; 
	strcpy(i_interface.ifr_name,g_interfacename);

	i_interface.ifr_addr.sa_family = AF_INET; 
	i=ioctl(*a_sender_sockfd, SIOCGIFADDR, &i_interface);

	if (i!=0) {
		printf("Could not determine interface address for interface %s\n",g_interfacename);
		return 0;
	}

	i_myinterface.sin_addr.s_addr=((struct sockaddr_in*)&i_interface.ifr_broadaddr)->sin_addr.s_addr;


	printf("%s: address %s\n",i_interface.ifr_name, inet_ntoa( ((struct sockaddr_in*)&i_interface.ifr_addr)->sin_addr));

	i_interface.ifr_broadaddr.sa_family = AF_INET; 
	strcpy(i_interface.ifr_name,g_interfacename);

	i_interface.ifr_addr.sa_family = AF_INET; 
	strcpy(i_interface.ifr_name,g_interfacename);

	i=ioctl(*a_sender_sockfd, SIOCGIFNETMASK, &i_interface);

	if (i!=0) {
		printf("Unable to determine netmask for interface %s\n",g_interfacename);
		return 0;
	}

	i_mynetmask.sin_addr.s_addr=((struct sockaddr_in*)&i_interface.ifr_broadaddr)->sin_addr.s_addr;

#ifndef OSX	
	printf("%s: netmask %s\n",i_interface.ifr_name, inet_ntoa( ((struct sockaddr_in*)&i_interface.ifr_netmask)->sin_addr));
#endif

	// Calculate broadcast address and stuff a_sender_address struct
	// with all the right params

	a_sender_address->sin_family = AF_INET;

	i_inverted_netmask=~i_mynetmask.sin_addr.s_addr;
	i_mybroadcast.sin_addr.s_addr=i_inverted_netmask|i_myinterface.sin_addr.s_addr;
	//	i_mybroadcast.sin_addr.s_addr=inet_addr("127.255.255.255");
	a_sender_address->sin_addr.s_addr=i_mybroadcast.sin_addr.s_addr;
	printf("Autoconfig: xAP broadcasts on %s:%d\n",inet_ntoa(a_sender_address->sin_addr),g_interfaceport);
	printf("xAP uid=%s, source=%s.%s.%s\n",g_uid, XAP_ME, XAP_SOURCE, g_instance);
	if (g_debuglevel>0) printf("Debug level %d\n",g_debuglevel);


	a_sender_address->sin_port=htons(g_interfaceport);

	return 1;

}


int xap_discover_hub_address(int* a_receiver_sockfd, struct sockaddr_in* a_receiver_address, int a_port_range_low, int a_port_range_high) {

	// In a hub configuration, the Xap application attempts
	// to open consecutive ports on the loop back interface
	// in a known range.
	// Once found, a heartbeat message is sent regularly to the hub
	// indicating which port is in use, and all incoming xAP messages 
	// are relayed to this application on this port.

	// returns 1 on success, 0 on failure.

	int i;
	int i_hubflag=0;

	if (a_port_range_high<a_port_range_low) {
		printf("Illegal socket range (highest port is lower than lowest port!)\n");
		exit(-1); // Illegal params
	}
	if (a_port_range_low==0) {
		printf("Illegal socket range (cannot use a socket of 0)\n");
		exit(-1);				  // Illegal params
	}


	*a_receiver_sockfd = socket(AF_INET, SOCK_DGRAM, 0); // Non-blocking listener
	fcntl(*a_receiver_sockfd, F_SETFL, O_NONBLOCK);
	fcntl(*a_receiver_sockfd, F_SETFL, O_ASYNC);


	// First atttempt to open the a broadcast port
	// If this fails then we can assume that a hub is active on this host

	a_receiver_address->sin_family = AF_INET; 
#ifndef WIN32	
	a_receiver_address->sin_addr.s_addr=g_xap_mybroadcast_address.sin_addr.s_addr;
#endif
#ifdef WIN32
	a_receiver_address->sin_addr.s_addr=htonl(INADDR_ANY);
#endif	
	a_receiver_address->sin_port=htons(a_port_range_low);

	if (bind(*a_receiver_sockfd, (struct sockaddr*)a_receiver_address, sizeof(*a_receiver_address))!=0) {
		printf("Broadcast socket port %d in use \n",a_port_range_low);
		printf("Assuming a hub is active\n");
		i_hubflag=1;
	}
	else {
		printf("Acquired broadcast socket, port %d\n",a_port_range_low);
		printf("Assuming no local hub is active\n");
		i_hubflag=0;
	}

	if (i_hubflag==1) {
		for (i=a_port_range_low; i<a_port_range_high; i++) {
			a_receiver_address->sin_family = AF_INET;
			a_receiver_address->sin_addr.s_addr=inet_addr("127.0.0.1");
			a_receiver_address->sin_port=htons(i);

			if (bind(*a_receiver_sockfd, (struct sockaddr*)a_receiver_address, sizeof(struct sockaddr))!=0) {
				printf("Socket port %d in use\n",i);
			}
			else {
				printf("Discovered port %d\n",i);
				break;
			}
		}
	}


	listen(*a_receiver_sockfd, MAX_QUEUE_BACKLOG);
	return 1; 	
}

int xap_discover_hub_address2(int* a_receiver_sockfd, struct sockaddr_in* a_receiver_address, struct sockaddr_in* a_broadcast_address, int a_port_range_low, int a_port_range_high) {

	// In a hub configuration, the Xap application attempts
	// to open consecutive ports on the loop back interface
	// in a known range.
	// Once found, a heartbeat message is sent regularly to the hub
	// indicating which port is in use, and all incoming xAP messages 
	// are relayed to this application on this port.

	// returns 1 on success, 0 on failure.

	int i;
	int i_hubflag=0;

	if (a_port_range_high<a_port_range_low) {
		printf("Illegal socket range (highest port is lower than lowest port!)\n");
		exit(-1); // Illegal params
	}
	if (a_port_range_low==0) {
		printf("Illegal socket range (cannot use a socket of 0)\n");
		exit(-1);				  // Illegal params
	}


	*a_receiver_sockfd = socket(AF_INET, SOCK_DGRAM, 0); // Non-blocking listener
	fcntl(*a_receiver_sockfd, F_SETFL, O_NONBLOCK);


	// First atttempt to open the a broadcast port
	// If this fails then we can assume that a hub is active on this host

	memset(a_receiver_address, 0, sizeof(struct sockaddr_in));
	//	memcpy(a_receiver_address, a_broadcast_address, sizeof(struct sockaddr_in));

	a_receiver_address->sin_family = AF_INET; 
	printf("Address %s\n",inet_ntoa(a_broadcast_address->sin_addr));
	//	a_receiver_address->sin_addr.s_addr=a_receiver_address->sin_addr.s_addr;
	a_receiver_address->sin_port=htons(a_port_range_low);
	//	a_receiver_address->sin_addr.s_addr=a_broadcast_address->sin_addr.s_addr;
	a_receiver_address->sin_addr.s_addr=inet_addr("0.0.0.0");

	//	a_receiver_address->sin_port=htons(a_port_range_low);

	if (bind(*a_receiver_sockfd, (struct sockaddr*)a_receiver_address, sizeof(*a_receiver_address))!=0) {
		printf("Broadcast socket port %d in use \n",a_port_range_low);
		printf("Assuming a hub is active\n");
		i_hubflag=1;
	}


	else {
		printf("Acquired broadcast socket, port %d\n",a_port_range_low);
		printf("Assuming no local hub is active\n");
		i_hubflag=0;
	}

	if (i_hubflag==1) {

		for (i=a_port_range_low; i<a_port_range_high; i++) {

			a_receiver_address->sin_family = AF_INET;
			a_receiver_address->sin_addr.s_addr=inet_addr("127.0.0.1");
			a_receiver_address->sin_port=htons(i);



			if (bind(*a_receiver_sockfd, (struct sockaddr*)a_receiver_address, sizeof(struct sockaddr))!=0) {
				printf("Socket port %d in use\n",i);
			}
			else {
				printf("Discovered port %d\n",i);
				break;
			}
		}
	}


	listen(*a_receiver_sockfd, MAX_QUEUE_BACKLOG);
	return 1; 	
}


int xap_init(int argc, char* argv[], int base) {

	int i_lowest_port=XAP_LOWEST_PORT;
	int i_highest_port=XAP_HIGHEST_PORT;

	strcpy(g_uid, XAP_GUID);

	xap_parse_commandline(argc, argv, base);

	if ((g_interfaceport<XAP_LOWEST_PORT)||(g_interfaceport>XAP_HIGHEST_PORT)){
		i_lowest_port=g_interfaceport;
		i_highest_port=g_interfaceport+1;
	}

	xap_discover_broadcast_network(&g_xap_sender_sockfd, &g_xap_sender_address);
	memcpy(&g_xap_mybroadcast_address, &g_xap_sender_address, sizeof(g_xap_sender_address));
	xap_discover_hub_address(&g_xap_receiver_sockfd, &g_xap_receiver_address, i_lowest_port, i_highest_port);
	return 1;
}

