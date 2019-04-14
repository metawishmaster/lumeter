#ifndef _XAPHUB_H_
#define _XAPHUB_H_

#include <QThread>

class xAPHubThread : public QThread
{
	Q_OBJECT

private:
	int argc;
	char **argv;

public:
	xAPHubThread(int argc, char *argv[]);

protected:
	void run();
};

class xAPSrvThread : public QThread
{
	Q_OBJECT

private:
	int argc;
	char **argv;

public:
	xAPSrvThread(int argc, char *argv[]);

protected:
	void run();
};

#endif

