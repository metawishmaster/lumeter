#include "xapdef.h"

int xap_send_tick(int a_interval) {


	time_t i_timenow;
	static time_t i_sendtick=0;

	i_timenow = time((time_t*)0);

	// Check timer for heartbeat send

	if ((i_timenow-i_sendtick>=a_interval)||(i_sendtick==0)) {												
		i_sendtick=i_timenow;
		return 1;
	} // send tick
	return 0;
}



int xap_heartbeat_tick(int a_interval) {

	// Call this function periodically.
	// Every HBEAT_INTERVAL, it will send a heartbeat

	time_t i_timenow;
	static time_t i_heartbeattick=0;

	i_timenow = time((time_t*)0);

	// Check timer for heartbeat send

	if ((i_timenow-i_heartbeattick>=a_interval)||(i_heartbeattick==0)) {												
		i_heartbeattick=i_timenow;
		// Send heartbeat to all external listeners
		xap_broadcast_heartbeat(g_xap_sender_sockfd, &g_xap_sender_address); 
		return 1;
	} // heartbeat tick
	return 0;
}


// Send a heartbeat to the real world

int xap_broadcast_heartbeat(int a_sock,  struct sockaddr_in* a_addr) {

	int i;
	char i_buff[1500];

	// Construct the heartbeat message (to say we are alive)
	// If using a hub, this must be sent on startup, because
	// that is how the hub knows who we are

	sprintf(i_buff, "xap-hbeat\n{\nv=12\nhop=1\nuid=%s\nclass=xap-hbeat.alive\nsource=%s.%s.%s\ninterval=%d\nport=%d\n}\n", g_uid, XAP_ME, XAP_SOURCE,  g_instance, HBEAT_INTERVAL, ntohs(g_xap_receiver_address.sin_port));

#ifdef DEBUG
	printf("Heartbeat source=%s, instance=%s, interval=%d, port=%d\n",XAP_SOURCE, g_instance, HBEAT_INTERVAL, 	ntohs(g_xap_receiver_address.sin_port) );
#endif

	// Send it...
	i= sendto(a_sock, i_buff, strlen(i_buff), 0, (struct sockaddr *)a_addr, sizeof(struct sockaddr));	
#ifdef DEBUG
	printf("Broadcasting heartbeat\n");
#endif

	return i;
}


// Build an outgoing xap (broadcast) message 

int xap_send_message(const char* a_buff) {

	// Send it...
	sendto(g_xap_sender_sockfd, a_buff, strlen(a_buff), 0, (struct sockaddr *)&g_xap_sender_address, sizeof(struct sockaddr));	
#ifdef DEBUG
	printf("Outgoing xAP message sent\n");	
#endif
	return 1;
}
