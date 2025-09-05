/*:
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
#include <QtGui>
#include <QPushButton>
#include <QRadioButton>
#include "preferences.h"
#include "basewidget.h"

PreferencesDiag::PreferencesDiag(QWidget *parent, bool h, bool bits,
		QMap<QString, QMap<QString, QWidget*> > *lMap) : QDialog(parent),
		horizontal(h), showInBits(bits), widgets(lMap)
{
	QCheckBox *checkBox;
	QMapIterator<QString, QMap<QString, QWidget*> > i(*widgets);
	QPushButton *invertBtn;

	setWindowTitle(tr("Preferences (items & layout)"));
	groupBox2 = new QGroupBox(tr("Alignment"));
	groupBox3 = new QGroupBox(tr("Show"));
	okBtn = new QPushButton("Ok");
	cancelBtn = new QPushButton("Cancel");

	connect(cancelBtn, SIGNAL(clicked()), this, SLOT(close()));
	connect(okBtn, SIGNAL(clicked()), this, SLOT(okClicked()));

	layout = new QVBoxLayout();
	layout2 = new QVBoxLayout();
	layout3 = new QVBoxLayout();
	grLayout = new QHBoxLayout();
	main_layout = new QVBoxLayout();
	btns_layout = new QHBoxLayout();

	while(i.hasNext()) {
		i.next();
		groupBox = new QGroupBox(i.key());
		QVBoxLayout* l = new QVBoxLayout;
		invertBtn = new QPushButton("Invert");
		connect(invertBtn, SIGNAL(clicked()), this, SLOT(invert()));
		l->addWidget(invertBtn);
		QMapIterator<QString, QWidget*> ii(i.value());
		while(ii.hasNext()) {
			ii.next();
			checkBox = new QCheckBox(ii.key());
			checkBox->setObjectName(ii.key());
			l->addWidget(checkBox);
			if((QWidget*)(ii.value())->isVisible())
				checkBox->setCheckState(Qt::Checked);
			else
				checkBox->setCheckState(Qt::Unchecked);
		}
		l->addStretch();
		groupBox->setLayout(l);
		grLayout->addWidget(groupBox);
	}

	QRadioButton *radio = new QRadioButton("Horizontal");
	radio->setChecked(h);
	radio->setObjectName("horizontalRadioButton");
	layout2->addWidget(radio);
	radio = new QRadioButton("Vertical");
	radio->setChecked(!h);
	radio->setObjectName("verticalRadioButton");
	layout2->addWidget(radio);

	layout2->setSpacing(1);
	groupBox2->setLayout(layout2);
	QVBoxLayout *xxLayout = new QVBoxLayout();
	grLayout->addLayout(xxLayout);
	xxLayout->addWidget(groupBox2);

	radio = new QRadioButton("Bits");
	radio->setChecked(qobject_cast<BaseWidget *>(parent)->bits);
	connect(radio, SIGNAL(toggled(bool)), this, SLOT(showInBitsSlot(bool)));
	layout3->addWidget(radio);
	radio = new QRadioButton("Bytes");
	radio->setChecked(!qobject_cast<BaseWidget *>(parent)->bits);
	layout3->addWidget(radio);

	layout3->setSpacing(1);
	groupBox3->setLayout(layout3);
	xxLayout->addWidget(groupBox3);

	btns_layout->addWidget(okBtn);
	btns_layout->addWidget(cancelBtn);
	btns_layout->addStretch();
	main_layout->addLayout(grLayout);
	main_layout->addLayout(btns_layout);
	setLayout(main_layout);
}

void PreferencesDiag::showInBitsSlot(bool b)
{
	qDebug() << "showInBitsSlot:" << b;
	showInBits = b;
}

void PreferencesDiag::invert()
{
	QList<QCheckBox *> l_checkboxes = sender()->parent()->findChildren<QCheckBox *>();
	QList<QCheckBox *>::iterator li;

	for(li = l_checkboxes.begin(); li != l_checkboxes.end(); ++li) {
		(*li)->setChecked(!(*li)->checkState());
	}
}

void PreferencesDiag::okClicked()
{
	int n, nn;
	QCheckBox* checkBox;
	QGroupBox* g;
	QLayout* l;
	QWidget* w;

	for (n = 0; n < layout2->count(); n++) {
		QWidget* widget = layout2->itemAt(n)->widget();
		if (!widget) continue;

		if (widget->objectName() == "horizontalRadioButton") {
			QRadioButton* radioButton = qobject_cast<QRadioButton*>(widget);
			if (radioButton && radioButton->isChecked()) {
				horizontal = 1;
				break;
			}
		} else if (widget->objectName() == "verticalRadioButton") {
			QRadioButton* radioButton = qobject_cast<QRadioButton*>(widget);
			if (radioButton && radioButton->isChecked()) {
				horizontal = 0;
				break;
			}
		}
	}
	printf("--- horizontal == %d\n", horizontal);

	for(n = 0; n < grLayout->count(); ++n) {
		g = qobject_cast<QGroupBox*>(grLayout->itemAt(n)->widget());
		if(g == NULL) continue;
		QString ip(g->title());
		l = qobject_cast<QLayout*>(g->layout());
		if(l == NULL) continue;

		for(nn = 0; nn < l->count(); ++nn) {
			checkBox = qobject_cast<QCheckBox*>(l->itemAt(nn)->widget());
			if(checkBox == NULL) continue;

			if(!widgets->operator[](ip).contains(checkBox->objectName())) {
				printf("%s contains %s\n", ip.toStdString().c_str(), checkBox->objectName().toStdString().c_str());
				continue;
			}
			QMapIterator<QString, QWidget*> ii(widgets->operator[](ip));
			while(ii.hasNext()) {
				ii.next();
				if(ii.key() != checkBox->objectName()) {
					continue;
				}
				w = ii.value();

				if(checkBox->checkState() == Qt::Checked) {
					w->show();
				} else {
					w->hide();
				}
				connect(this, SIGNAL(showInBitsSignal(bool)), qobject_cast<IOSWidget*>(w), SLOT(showInBits(bool)));
				printf("%s: %s <-> %s\n", ip.toStdString().c_str(), ii.key().toStdString().c_str(), checkBox->objectName().toStdString().c_str());
			}
		}
	}

	emit showInBitsSignal(showInBits);

	hide();
}

