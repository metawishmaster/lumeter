#include "xapdef.h"

#include <stdio.h>
#include <errno.h>

const char* XAP_ME = "test";
const char* XAP_SOURCE = "testsrc";
const char* XAP_GUID = "FFFFFFFF";
const char* XAP_DEFAULT_INSTANCE = "1";

int main(int argc, char *argv[])
{
	int n_buf = 1500;
	int ret = xap_init(argc, argv, 0);
	char buf[n_buf+1];

	printf("xap_init(%d, %s, 0) ret = %d\n", argc, *argv, ret);

	do
	{
		xap_heartbeat_tick(HBEAT_INTERVAL);
		ret = xap_poll_incoming(g_xap_receiver_sockfd, buf, n_buf);
		if(ret == -1) {
			printf("ERROR: %d\n", errno);
			puts(strerror(errno));
			exit(-1);
		}
		printf("xap_poll_incoming ret = %d\n", ret);
		printf("[R]: %s\n", buf);
		xapmsg_parse(buf);
		if(xapmsg_getvalue("wlan0:in", buf) == 1) {
			printf("[R2]: %s\n", buf);
//			break;
		}
		if(xapmsg_getvalue("wlan0:out", buf) == 1) {
			printf("[R2]: %s\n", buf);
//			break;
		}
	} while (1);

	return 0;
}

