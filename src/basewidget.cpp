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

#include <QMessageBox>

#include <basewidget.h>
#include "netstat.h"

BaseWidget::~BaseWidget()
{
	QMapIterator< QString, QMap<QString, QWidget*> > i(widgets);

	while(i.hasNext()) {
		i.next();
		settings.beginGroup(i.key());
		QMapIterator<QString, QWidget*> ii(widgets[i.key()]);
		while(ii.hasNext()) {
			ii.next();
			settings.setValue(ii.key(), !((QWidget*)(ii.value()))->isHidden());
		}
		settings.endGroup();
		widgets[i.key()].clear();
	}

	widgets.clear();
	settings.beginGroup("Layout");
	settings.setValue("horizontal", hLayout);
	settings.setValue("pos", pos());
	settings.setValue("show_in_bits", bits);
	settings.endGroup();
	xap->exit(0);
	xap->wait();
	xap->deleteInstance();
}

BaseWidget::BaseWidget(NetStat* _stat, LXAPThread* xap_) : netstat(_stat), xap(xap_), time_interval(1000), bits(false), diag(nullptr)
{
	settings.beginGroup("Layout");
	hLayout = settings.value("horizontal", true).toBool();
	bits = settings.value("show_in_bits", false).toBool();
	QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
	settings.endGroup();

	move(pos);
	if(hLayout) layout = new QHBoxLayout();
	else layout = new QVBoxLayout();

	setMaximumSize(QSize(20, 40));

	layout->setContentsMargins(0, 0, 0, 0); // вместо setMargin(0)
	layout->setSpacing(0);
	layout->setSpacing(0);
	setLayout(layout);

	QTimer *speedTimer = new QTimer(this);

	connect(speedTimer, SIGNAL(timeout()), this, SLOT(checkSpeed()));
	speedTimer->start(time_interval);

	diag = NULL;

	setWindowFlags(Qt::Window);
	setAttribute(Qt::WA_DeleteOnClose);
	setAttribute(Qt::WA_QuitOnClose);

	prefs_action = new QAction(tr("&Preferences"), this);
	connect(prefs_action, SIGNAL(triggered()), this, SLOT(showPrefDiag()));
	hide_action = new QAction(tr("&Hide"), this);
	about_action = new QAction(tr("&About"), this);
	connect(about_action, SIGNAL(triggered()), this, SLOT(about()));
	aboutQt_action = new QAction(tr("About &Qt"), this);
	connect(aboutQt_action, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
	exit_action = new QAction(tr("E&xit"), this);
	connect(exit_action, SIGNAL(triggered()), this, SLOT(close()));
	connect(exit_action, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

	m_menu.addAction(prefs_action);
	m_menu.addAction(hide_action);
	m_menu.addSeparator();
	m_menu.addAction(about_action);
	m_menu.addAction(aboutQt_action);
	m_menu.addAction(exit_action);
	
//	diag = new PreferencesDiag(this, hLayout, &widgets);
}

void BaseWidget::closeEvent(QCloseEvent *event)
{
	xap->end();
	event->accept();
}

void BaseWidget::lopOff(const QString& ip)
{
	QWidget *i;
	layout->removeItem(layouts[ip]);
	netstat->remove(ip);
	foreach(i, widgets[ip]) {
		delete i;
	}
	layouts.remove(ip);
	widgets.remove(ip);
	netstat->remove(ip);
//	delete layouts[ip];
}

void BaseWidget::addIpLayout(const QString& ip)
{
	QLayout* l = NULL;
	qDebug() << "ADD" << ip;
	layouts[ip] = l;
	QStringList	ips = layouts.keys();
	int idx = ips.indexOf(ip);

	if(hLayout) {
		l = new QHBoxLayout;
		((QHBoxLayout*)layout)->insertLayout(idx, l);
	} else {
		l = new QVBoxLayout;
		((QVBoxLayout*)layout)->insertLayout(idx, l);
	}
	l->setMargin(0);
	l->setSpacing(0);
	layouts[ip] = l;
}

void BaseWidget::checkSpeed()
{
	int i;

	setUpdatesEnabled(false);
//FIXME: Race condition?
	QMapIterator<QString, IOStat> ii(*netstat);
	while(ii.hasNext()) {
		ii.next();
		if(ii.key() == "") {
			continue;
		}
		const QString& ip(ii.key());
		IOStat& iostat = (IOStat&)ii.value();
		QReadLocker locker(iostat.lock);

		int ts = QTime::currentTime().secsTo(iostat.time_stamp);
		if (ts < 0) {
			if(ts < -3) {
				qDebug() << "DEL" << ip;
				lopOff(ip);
				continue;
			}// else
			//	widgets[ip]->setEnabled(false);
		}
		if(!layouts.contains(ip))
			addIpLayout(ip);

		settings.beginGroup(ip);
		QMap < QString, QPair<long, long> >::const_iterator it = iostat.const_begin();
		for(; it != iostat.const_end(); it++) {
			const QString& if_name(it.key());
			if(!widgets.contains(ip) || !widgets[ip].contains((const QString)if_name)) {
				IOSWidget *newWidget  = new IOSWidget(this, if_name + " on " + ip);
//				connect(this, SIGNAL(showInBitsSignal(bool)), newWidget, SLOT(showInBits(bool)));
				newWidget->showInBits(this->bits);
				widgets[ip][if_name] = newWidget;
				qDebug() << if_name << "on" << ip;
				layouts[ip]->addWidget(newWidget);
				if(settings.value(if_name, true).toBool())
					((IOSWidget*)(widgets[ip][if_name]))->show();
				else
					((IOSWidget*)(widgets[ip][if_name]))->hide();
			}
			if(widgets[ip].contains(if_name) && (IOSWidget*)widgets[ip][if_name] && ((IOSWidget*)widgets[ip][if_name])->isVisible())
				((IOSWidget*)widgets[ip][if_name])->checkSpeed(it.value());
		}
		settings.endGroup();

		if (iostat.getMap().size() < widgets[ip].size()) {
			for (i = 0; i < widgets[ip].size(); i++) {
				if (!iostat.getMap().contains(widgets[ip].keys().at(i))) {
					QWidget *tmp =  widgets[ip][widgets[ip].keys().at(i)];
					widgets[ip].remove(widgets[ip].keys().at(i));
					delete tmp;
				}
			}
		}
	}
	setUpdatesEnabled(true);
}

void BaseWidget::showInBits(bool b)
{
	this->bits = b;
//	emit PreferencesDiag::showInBitsSignal(b);
}

void BaseWidget::showPrefDiag()
{
	bool h;
	QLayout* l;
	QLayout *tmp;

	diag = new PreferencesDiag(this, hLayout, this->bits, &widgets);
	connect(diag, SIGNAL(showInBitsSignal(bool)), this, SLOT(showInBits(bool)));
	diag->exec();
	diag->hide();
	h = (diag->getState() == 1);
	printf("HORIZONTAL = %d\n", h);
	delete diag;
	if(h != hLayout) {
		if(h) tmp = new QHBoxLayout(0);
		else tmp = new QVBoxLayout(0);

		tmp->setSizeConstraint(QLayout::SetFixedSize);
		tmp->setMargin(0);
		tmp->setSpacing(0);

		QMapIterator<QString, QMap<QString, QWidget*> > i(widgets);
		while(i.hasNext()) {
			i.next();
			QString ip(i.key());
			if(!h) {
				l = new QVBoxLayout;
				((QHBoxLayout*)tmp)->addLayout(l);
			} else {
				l = new QHBoxLayout;
				((QVBoxLayout*)tmp)->addLayout(l);
			}
			l->setMargin(0);
			l->setSpacing(0);

			QMapIterator<QString, QWidget*> m(widgets[i.key()]);
			while(m.hasNext()) {
				m.next();
				l->addWidget(m.value());
				layout->removeWidget(m.value());
			}
			layouts[ip] = l;
		}
		delete layout;
		layout = tmp;
		setLayout(layout);
	}
	hLayout = h;

	QMapIterator< QString, QMap<QString, QWidget*> > i(widgets);

	while(i.hasNext()) {
		i.next();
		settings.beginGroup(i.key());
		QMapIterator<QString, QWidget*> ii(widgets[i.key()]);
		while(ii.hasNext()) {
			ii.next();
			settings.setValue(ii.key(), !((QWidget*)(ii.value()))->isHidden());
		}
		settings.endGroup();
//		widgets[i.key()].clear();
	}
}

QMenu* BaseWidget::menu()
{
	return &m_menu;
}

void BaseWidget::setHideActOn(QWidget *widget)
{
	disconnect(hide_action, 0, 0, 0);
	connect(hide_action, SIGNAL(triggered()), widget, SLOT(hide()));
}

void BaseWidget::about()
{
	QString aboutCaption = QString("About lumeter-%1").arg(LUMETER_VERSION);
	QString aboutText = QString(
			"<p>© Sergej Bauer 2019 Moscow, Russian Federatio</p>"
			"<p>mailto:sergej.bauer@gmail.com</p>"
			"<p>Version: %1</p>"
			"<p>Built on: %2 at %3</p>"
    ).arg(LUMETER_VERSION).arg(__DATE__).arg(__TIME__);
	QMessageBox::about(this, aboutCaption, aboutText);
}

void BaseWidget::contextMenuEvent(QContextMenuEvent *event)
{
	m_menu.exec(event->globalPos());
}

