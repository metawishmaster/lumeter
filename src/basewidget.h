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
#ifndef _BASE_WIDGET_H_
#define _BASE_WIDGET_H_

#include <QHBoxLayout>
#include <QWidget>
#include <QTimer>
#include <QFile>
#include <QHash>
#include <QString>
#include <QRegExp>
#include <QMenu>
#include <QContextMenuEvent>
#include <QSizePolicy>

#include "ioswidget.h"
#include "preferences.h"
#include "lxapthread.h"
#include "netstat.h"

typedef QHash <const QString, QPair<long, long> > Hash;

class BaseWidget : public QWidget
{
	Q_OBJECT

public:
		BaseWidget(NetStat* netstat, LXAPThread* xap_);
		~BaseWidget();
		QSettings settings;
		void setHideActOn(QWidget*);
		QMenu* menu();

private:
		NetStat* netstat;
		LXAPThread* xap;
		int time_interval;
		QMap <QString, QLayout*> layouts;
		QMap < QString, QPair<long, long> > obsoleteMap;
		QMap < QString, QMap<QString, QWidget*> > widgets;
		Hash speeds;
		QLayout *layout;
		PreferencesDiag *diag;
		bool hLayout;
		void lopOff(const QString&);
		void addIpLayout(const QString&);

		QMenu m_menu;
		QAction *aboutQt_action;
		QAction *about_action;
		QAction *exit_action;
		QAction *prefs_action;
		QAction *hide_action;
		void contextMenuEvent(QContextMenuEvent *event);

public slots:
		void checkSpeed();
		void showPrefDiag();
		void about();

signals:
		void updateSpeed(QPair<long, long>&);

protected:
		virtual void closeEvent(QCloseEvent *event);
};

#endif

