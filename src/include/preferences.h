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
#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_

#include <QWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QDialog>
#include <QtGui>

class PreferencesDiag : public QDialog
{
	Q_OBJECT

	public:
		PreferencesDiag(QWidget*, bool, bool, QMap< QString, QMap<QString, QWidget*> > *);
		bool getState() { return horizontal; }

	private:
		QPushButton *okBtn;
		QPushButton *cancelBtn;

		QGroupBox *groupBox;
		QGroupBox *groupBox2;
		QGroupBox *groupBox3;
		QVBoxLayout *layout;
		QVBoxLayout *layout2;
		QVBoxLayout *layout3;
		QHBoxLayout *grLayout;
		QVBoxLayout *main_layout;
		QHBoxLayout *btns_layout;

		bool showInBits;
		bool horizontal;
		QMap<QString, QMap<QString, QWidget*> > *widgets;

	signals:
		void showInBitsSignal(bool);

	private slots:
		void showInBitsSlot(bool);
		void invert();
		void okClicked();
};

#endif
