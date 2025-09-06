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

#ifndef _NETSTAT_
#define _NETSTAT_

#include <QtCore>
#include <QMap>
#include <QTime>
#include <QPair>
#include <QString>
#include <QRegExp>
#include <QReadLocker>
#include <QReadWriteLock>

class IOStat
{
	QMap<QString, QPair<long, long>> map;
#if QT_VERSION >= 0x040500
	static const QRegExp rx;
#else
	static QRegExp rx;
#endif
public:
	QReadWriteLock *lock;
	QTime time_stamp;

	IOStat() : lock(new QReadWriteLock)
	{
	}

	~IOStat()
	{
		delete lock;
	}

	IOStat(const IOStat& other) : lock(new QReadWriteLock), map(other.map), time_stamp(other.time_stamp)
	{
	}

	IOStat& operator=(const IOStat& other)
	{
		if (this != &other) {
			QWriteLocker locker(lock);
			QWriteLocker otherLocker(other.lock);
			map = other.map;
			time_stamp = other.time_stamp;
		}
		return *this;
	}

	const QPair<long, long> operator[](const QString& s) const
	{
		QReadLocker locker(lock);
		return map.value(s);
	}

	void operator<<(const char* s)
	{
		QWriteLocker locker(lock);
		const QString str(s);
		int pos = 0;
		time_stamp = QTime::currentTime();
		map.clear();

		while (rx.indexIn(str, pos) != -1) {
			if (rx.captureCount() == 3) {
				map[rx.cap(1)] = qMakePair(rx.cap(2).toLong(), rx.cap(3).toLong());
				pos += rx.matchedLength();
			} else {
				break;
			}
		}
	}

	QMap<QString, QPair<long, long>> getMap() const
	{
		QReadLocker locker(lock);
		return map;
	}

	QMap<QString, QPair<long, long>>::const_iterator const_begin() const
	{
		QReadLocker locker(lock);
		return map.constBegin();
	}

	QMap<QString, QPair<long, long>>::const_iterator const_end() const
	{
		QReadLocker locker(lock);
		return map.constEnd();
	}
};

typedef QMap<QString, IOStat> NetStat;
#endif
