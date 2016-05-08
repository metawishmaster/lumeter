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
#ifndef _IOS_WIDGET_H_
#define _IOS_WIDGET_H_
#include <QWidget>
#include <QHash>

#include "basewidget.h"
class BaseWidget;

class IOSWidget : public QWidget
{
	Q_OBJECT
	public:
		IOSWidget(BaseWidget*, const QString&);
	public slots:
		void checkSpeed(const QPair<long, long>& pair);
	signals:
		void updateInSpeed(QString);
		void updateOutSpeed(QString);
	private:
		QString if_name;
		BaseWidget *parent;
		void contextMenuEvent(QContextMenuEvent *event);
};
#endif

