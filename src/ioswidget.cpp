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
#include <QFont>
#include <QLCDNumber>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QContextMenuEvent>
#include <QMenu>
#include <QGroupBox>
#include <QApplication>

#include "ioswidget.h"

void IOSWidget::showInBits(bool f)
{
	showBits = f;
	if (showBits)
		this->setToolTip("Bits per second");
	else
		this->setToolTip("Bytes per second");

}

void IOSWidget::checkSpeed(const QPair<long, long>& pair)
{
	QString in, out;

	if (showBits) {
		in.setNum(pair.first << 3);
		out.setNum(pair.second << 3);
	} else {
		in.setNum(pair.first);
		out.setNum(pair.second);
	}

	int l = in.length();
	while(l-- > 3)
		if(l % 3 == 0) in.insert(in.length() - l, " ");
	l = out.length();
	while(l-- > 3)
		if(l % 3 == 0) out.insert(out.length() - l, " ");

	emit updateInSpeed("<font color=green>" + in + "</font>");
	emit updateOutSpeed("<font color=red>" + out + "</font>");
}

IOSWidget::IOSWidget(BaseWidget* _parent, const QString& name)
	: parent(_parent)
{
	if_name = name;

	setObjectName(name);
	QLabel *inLabel = new QLabel("0");
	inLabel->setMargin(2);
	connect(this, SIGNAL(updateInSpeed(QString)), inLabel, SLOT(setText(QString)));

	QLabel *outLabel = new QLabel("0");
	outLabel->setMargin(2);
	connect(this, SIGNAL(updateOutSpeed(QString)), outLabel, SLOT(setText(QString)));

	QHBoxLayout* ioLayout = new QHBoxLayout;
	QHBoxLayout* layout = new QHBoxLayout;
	QGroupBox* gBox = new QGroupBox(name);
	layout->addWidget(inLabel);
	layout->addWidget(outLabel);
	layout->setSpacing(0);
	layout->setMargin(0);
	gBox->setLayout(layout);
	ioLayout->addWidget(gBox);
	ioLayout->setSpacing(0);
	ioLayout->setMargin(0);
	setLayout(ioLayout);
	showBits = qobject_cast<BaseWidget *>(parent)->bits;
}

void IOSWidget::contextMenuEvent(QContextMenuEvent *event)
{
	parent->setHideActOn(this);
	parent->menu()->exec(event->globalPos());
}

