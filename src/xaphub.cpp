#include "xaphub.h"
//#include <QException>

extern "C" int hub_main(int argc, char *argv[]);
extern "C" int srv_main(int argc, char *argv[]);

xAPHubThread::xAPHubThread(int _argc, char *_argv[]) :
									argc(_argc),
									argv(_argv),
									QThread()
{
}

void xAPHubThread::run()
{
	printf("RUNNING xAP HUB thread\n");
	hub_main(argc, argv);
}

xAPSrvThread::xAPSrvThread(int _argc, char *_argv[]) :
									argc(_argc),
									argv(_argv),
									QThread()
{
}

void xAPSrvThread::run()
{
	printf("RUNNING zAP Srv thread\n");
	srv_main(argc, argv);
}
