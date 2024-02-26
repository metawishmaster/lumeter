/*
 * lumeter - simple tool for watching statistics from local/non-local network interfaces
 *
 * Copyright (c) 2009 Bauer Sergej <sergej.bauer@gmail.com>
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
#include "lxapthread.h"
#include "netstat.h"

extern "C" {
#include "xapdef.h"
}
#include <QtCore>

LXAPThread* LXAPThread::instance = NULL;
const QRegExp LXAPThread::srcRX("^src\\n\\{(?:\\n)addr=(\\d{,3}\\.\\d{,3}\\.\\d{,3}\\.\\d{,3})\\n\\}\\n");

LXAPThread::LXAPThread(int argc, char *argv[], NetStat* _stat) : stat(_stat)
{
	xap_init(argc, argv, 0);
	endReq = false;
}

LXAPThread::~LXAPThread()
{
	delete stat;
}

LXAPThread* LXAPThread::getInstance(int argc, char* argv[], NetStat* _stat)
{
	if(!instance) {
		instance = new LXAPThread(argc, argv, _stat);
	}

	return instance;
}

void LXAPThread::deleteInstance()
{
	if(instance)
		delete instance;
}

void LXAPThread::run()
{
	char buf[1500];
	char* s = buf;
    QString cap;

	while(!endReq) {
		xap_heartbeat_tick(HBEAT_INTERVAL);
		xap_poll_incoming(g_xap_receiver_sockfd, buf, 1500);
		if(srcRX.indexIn(buf) != 1) {
			s += srcRX.matchedLength();
			s[1499] = '\0';
			(*stat)[srcRX.cap(1)] << s;
		}
		s = buf;
	}
	quit();
}

void LXAPThread::end()
{
	endReq = true;
}

