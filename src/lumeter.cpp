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
#include <QApplication>
#include <QSplashScreen>
#include <QProxyStyle>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "basewidget.h"
#include "lxapthread.h"
#include "netstat.h"
#include "xaphub.h"

typedef void (*sidhandler_t)(int);

const char* XAP_DEFAULT_INSTANCE = "Linux";
const char* XAP_SOURCE = "lumeter2c";
const char* XAP_GUID = "FFEEE3EE";
const char* XAP_ME = "lumeter2-gui";

extern "C" int hub_main(int argc, char *argv[]);
extern "C" int srv_main(int argc, char *argv[]);

/*
int hpid, spid;

void killchilds()
{
	int status;
	int ret;

	errno = 0;
	ret = kill(hpid, SIGTERM);
	if(errno) perror("kill HUB process");
	ret = kill(spid, SIGTERM);
	if(errno) perror("kill server process");
	wait(&status);
	if(errno) perror("waiy killed");
}

void daemonize(void)
{
	pid_t sid;

	if(getppid() == 1) return;

	umask(0);

	sid = setsid();
	if(sid < 0) exit(-1);

	if((chdir("/")) < 0) exit(-1);
}

void handler(int n)
{
	int status, ret = n;

	fprintf(stderr, "handeler\n");
	errno = 0;
	ret = kill(hpid, SIGTERM);
	if(errno) perror("kill HUB process");
	ret = kill(spid, SIGTERM);
	if(errno) perror("kill server process");
	ret = kill(getpid(), SIGTERM);
	if(errno) perror("kill this process");
	wait(&status);
	if(errno) perror("while waiting killed");
}
*/
int main(int _argc, char *_argv[])
{
	QApplication app(_argc, _argv);
	xAPHubThread *xaphub;
	xAPSrvThread *xapsrv;

	int ret, argc = _argc;
	char** argv;

	QCoreApplication::setOrganizationName("MW Soft");
	QCoreApplication::setApplicationName("lumeter");

	QPixmap tux(":/tux.png");
	QSplashScreen splash(tux);
	splash.show();

	QSettings settings;
	settings.beginGroup("network");
	if (!settings.contains("lsn_if")) {
		if (_argc != 2) {
			splash.showMessage("<font size=16 color=red>USAGE: lumeter [interface]\nCannot determiine listen interface</font>");
			app.processEvents();
			settings.endGroup();
			sleep(3);
			exit(1);
		} else {
			splash.showMessage("<font size=16 color=green>Starting xAP hub...</font>");
			settings.setValue("lsn_if", _argv[1]);
			argv = _argv;
		}
	} else {
		splash.showMessage("<font size=16 color=green>Starting xAP hub...</font>");
		if (argc == 1) {
			argv = (char**)malloc(3 * sizeof(char*));
			argv[0] = strdup(_argv[0]);
			argv[1] = strdup(settings.value("lsn_if", "eth0").toString().toStdString().c_str());
			argv[2] = NULL;
			argc++;
			_argc = -1;
		} else {
			argv = _argv;
			settings.setValue("lsn_if", _argv[1]);
		}
	}
	app.processEvents();
	settings.endGroup();

	xaphub = new xAPHubThread(argc, argv);
	xapsrv = new xAPSrvThread(argc, argv);

	splash.showMessage("<font size=16 color=green>Starting lumeter...</font>");
	app.processEvents();
	xaphub->start();
	sleep(1);
/*
	splash.showMessage("<font size=16 color=green>Starting lumeter client...</font>");
	app.processEvents();
	xapsrv->start();
	sleep(1);
	printf("started\n");
*/
	xapsrv->start();
#if 0
	hpid = fork();
	if(hpid == 0) {
		daemonize();
		hub_main(argc, argv);
		sleep(1);
		kill(getppid(), SIGINT);
		exit(0);
	}
	sleep(1);

	splash.showMessage("<font size=16 color=green>Starting lumeter server...</font>");
	app.processEvents();
	spid = fork();
	if(spid == 0) {
		daemonize();
		srv_main(argc, argv);
		exit(0);
	}
	sleep(1);
#endif

//	atexit(killchilds);

	NetStat* netstat = new NetStat;
	LXAPThread* xap = LXAPThread::getInstance(argc, argv, netstat);
	xap->start();

	BaseWidget* widget = new BaseWidget(netstat, xap);
	widget->show();

//	for(int i = 1; i < 65; i++)
//	signal(SIGTERM, &handler);
//	signal(SIGINT, &handler);

	splash.finish(widget);
	ret = app.exec();
	if (_argc == -1) {
		for (_argc = 0; _argc < argc; _argc++)
			free(argv[_argc]);
		free(argv);
	}
	//xaphub->exit();

	return ret;
}

