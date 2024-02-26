/*
 * lumeter - simple tool for watching statistics from local/non-local network interfaces
 *
 * Copyright (c) 2009 Bauer Sergej <sergej.bauer@gmail.com>
 * Copyright (c) 2009 Taranenko Oleg <oleg@taranenko.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "xapdef_ext.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>

typedef int bool;
const bool false = 0;
const bool true = 1;

#include "simple-cache.h"

#ifndef LUMETER_GUI
const char* XAP_ME = "lumeterD";
const char* XAP_SOURCE = "server";
const char* XAP_GUID = "F0E0DECE";
const char* XAP_DEFAULT_INSTANCE = "1";
#endif

typedef struct interface_info_tag
{
	unsigned long long int ibytes;
	unsigned long long int obytes;
	double last_time;
} interface_info;
static simple_cache statistic;
static bool filter_ifaces = 0;

double hitime()
{
	struct timeval t;
	if (gettimeofday(&t, NULL)) return 0;
	return (t.tv_sec + (t.tv_usec/(double)1000000));
}

interface_info* get_iinfo(const char* iname, bool add_if_need)
{
	interface_info *ret = 0;
	ret = scache_get(&statistic, iname, 0);
	if (!ret && add_if_need)
	{
		ret = (interface_info*)malloc(sizeof(interface_info));
		ret->ibytes = 0;
		ret->obytes = 0;
		ret->last_time = 0;
		scache_set(&statistic, iname, ret);
	}

	return ret;
}

void gather(FILE* f, char* buf)
{
	static char iname[20], tmp[1400];
	unsigned long long int ibytes, obytes, ispeed = 0, ospeed = 0;
	unsigned int j = 0, len;
	char *c;
	interface_info *iinfo;

	tmp[0] = '\0';
	while(fgets(buf, 1400 - 1, f) != NULL) {
		c = buf;
		while(*c == ' ') c++;
		while(*c != ':' && j < sizeof(iname) - 1)
			iname[j++] = *(c++);
		iname[j] = '\0';
		j = 0;

		iinfo = get_iinfo(iname, !filter_ifaces);
		if (filter_ifaces && !iinfo) {
			printf("%s\n", iname);
			continue;
		}

		c++;
		while(*c == ' ') c++;
		sscanf(c, "%llu %*u %*u %*u %*u %*u %*u %*u %llu %*u %*u %*u %*u %*u %*u %*u\n", &ibytes, &obytes);

		if (iinfo)
		{
			double t = hitime();
			double tdif = t - iinfo->last_time;
			if (tdif > 1e-6 )
			{
				ispeed = (ibytes - iinfo->ibytes)/tdif;
				ospeed = (obytes - iinfo->obytes)/tdif;
			}
			iinfo->ibytes = ibytes;
			iinfo->obytes = obytes;
			iinfo->last_time = t;
		}
		else
		{
			ispeed = 0;
			ospeed = 0;	
		}
		len = strlen(tmp);
		snprintf(tmp + len, 1400 - len, "%s\n{\nin=%llu\nout=%llu\nispeed=%llu\nospeed=%llu\n}\n", iname, ibytes, obytes, ispeed, ospeed);
	}
	strncpy(buf, tmp, 1400);
}

void get_actual_ifaces(int* n, char* argv[])
{
	int i, pos = 0, argc = *n;

	for(i = 0; i < argc; i++) {
		if(!strcmp(argv[i], "ifaces")) {
			pos = i + 1;
			filter_ifaces = true;
			break;
		}
	}

	if (!filter_ifaces) return;
	for(i = pos; i < argc; i++) get_iinfo(argv[i], true);	
	*n = pos - 1;
}

#ifdef LUMETER_GUI
int srv_main(int argc, char* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	char *s, buf[1500];
	int fd;
	FILE* f;
	long pos = 0;
	struct ifreq interface;

	memset(&interface, 0, sizeof(interface));
	interface.ifr_addr.sa_family = AF_INET;
	if(argc > 1)
		strcpy(interface.ifr_name, argv[1]);
	else
		strcpy(interface.ifr_name, "eth0");

	scache_init(&statistic, 20, free);
	get_actual_ifaces(&argc, argv);

	if(xap_init(argc, argv, 0) != 1) {
		fprintf(stderr, "error while initializing xAP\n");
		scache_free(&statistic);
		return -1;
	}

	if(ioctl(g_xap_sender_sockfd, SIOCGIFADDR, &interface) != 0) {
		printf("error while determining address\n");
		exit(-1);
	} else {
		sprintf(buf, "src\n{\naddr=%s\n}\n", inet_ntoa(((struct sockaddr_in*)(&interface.ifr_addr))->sin_addr));
		s = buf + strlen(buf);
	}

	fd = open("/proc/net/dev", O_ASYNC | O_NONBLOCK);
	fcntl(fd, F_SETFL, O_ASYNC);
	f = fdopen(fd, "r");

	s = fgets(s, 1400, f);
	s = fgets(s, 1400, f);
	pos = ftell(f);

	while(1) {
		xap_heartbeat_tick(HBEAT_INTERVAL);
		gather(f, s);

		xap_send_message(buf);
		sleep(1);
		fseek(f, pos, SEEK_SET);
	}

	scache_free(&statistic);
	return 0;
}

