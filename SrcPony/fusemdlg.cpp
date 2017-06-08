//=========================================================================//
//                                                                         //
//  PonyProg - Serial Device Programmer                                    //
//                                                                         //
//  Copyright (C) 1997-2017   Claudio Lanconelli                           //
//                                                                         //
//  http://ponyprog.sourceforge.net                                        //
//                                                                         //
//-------------------------------------------------------------------------//
//                                                                         //
// This program is free software; you can redistribute it and/or           //
// modify it under the terms of the GNU  General Public License            //
// as published by the Free Software Foundation; either version2 of        //
// the License, or (at your option) any later version.                     //
//                                                                         //
// This program is distributed in the hope that it will be useful,         //
// but WITHOUT ANY WARRANTY; without even the implied warranty of          //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       //
// General Public License for more details.                                //
//                                                                         //
// You should have received a copy of the GNU  General Public License      //
// along with this program (see COPYING);     if not, write to the         //
// Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. //
//                                                                         //
//-------------------------------------------------------------------------//
//=========================================================================//


#include "fusemdlg.h"

#include <QDebug>
#include <QStringList>
#include <QTreeWidget>
#include <QRadioButton>



//======================>>> fuseModalDialog::fuseModalDialog <<<==================
fuseModalDialog::fuseModalDialog(e2CmdWindow *bw, e2AppWinInfo *p, bool readonly, const QString &msg) :
	QDialog(bw)
{
	setupUi(this);

	setWindowTitle(translate(STR_MSGFUSEDLG));

	awip = p;

	cmdw = static_cast<e2CmdWindow *>(bw);

	if (cmdw->getStyleSheet().length() > 0)
	{
		setStyleSheet(cmdw->getStyleSheet());
	}

	qDebug() << "fuseModalDialog::fuseModalDialog()";

	read = write = false;

	setTextWidgets();

	initWidgets(msg, readonly);

	connect(pushRead, SIGNAL(clicked()), this, SLOT(onRead()));
	connect(pushWrite, SIGNAL(clicked()), this, SLOT(onProg()));
	connect(pushOk, SIGNAL(clicked()), this, SLOT(onOk()));
	connect(pushCancel, SIGNAL(clicked()), this, SLOT(reject()));
}


//===================>>> fuseModalDialog::~fuseModalDialog <<<====================
fuseModalDialog::~fuseModalDialog()
{
	qDebug() << "fuseModalDialog::~fuseModalDialog()";
}

void fuseModalDialog::setTextWidgets()
{
	// EK 2017
	// TODO add translations for header of view
	treeWidgetFuse->setHeaderLabels(QStringList() << "Bit" << "Description");
	treeWidgetLock->setHeaderLabels(QStringList() << "Bit" << "Description");

	pushRead->setText(translate(STR_BTNREAD));
	pushWrite->setText(translate(STR_BTNWRITE));
	pushOk->setText(translate(STR_BTNOK));
	pushCancel->setText(translate(STR_BTNCANC));
}


void fuseModalDialog::onOk()
{
	unsigned int l = 0;
	unsigned int f = 0;

	for (int i = 0; i < treeWidgetFuse->topLevelItemCount(); ++i)
	{
		QString t = treeWidgetFuse->topLevelItem(i)->text(0);
		if (treeWidgetFuse->topLevelItem(i)->checkState(0) == false)
		{
			continue;
		}

		int pos = t.indexOf(",");
		if (pos > 0)
		{
			t = t.left(pos);
			t = t.remove("Bit ");
			int bOffset = t.toInt();

			f |= (1 << bOffset);
		}

	}

	for (int i = 0; i < treeWidgetLock->topLevelItemCount(); ++i)
	{
		QString t = treeWidgetLock->topLevelItem(i)->text(0);
		if (treeWidgetLock->topLevelItem(i)->checkState(0) == false)
		{
			continue;
		}

		int pos = t.indexOf(",");
		if (pos > 0)
		{
			t = t.left(pos);
			t = t.remove("Bit ");
			int bOffset = t.toInt();

			l |= (1 << bOffset);
		}
	}

	if (read == true)
	{
		emit done(2);
	}
	else
	{
		awip->SetLockBits(l);
		awip->SetFuseBits(f);

		if (write == true)
		{
			emit done(3);
		}
		else
		{
			accept();
		}
	}
}


void fuseModalDialog::onProg()
{
	write = true;

	emit onOk();
}


void fuseModalDialog::scanMasks()
{
	maskListLock.clear();
	maskListFuse.clear();

	ChipBits fBit = eep_bits.at(currentChip);

	// analyse from mask entries
	foreach (MaskDescr mdes, fBit.lockDescr)
	{
		QString cMask = mdes.mask;
		cMask.replace(QRegExp("\\d+"), "\\d+");
		cMask = "^" + cMask;
		if (maskListLock.indexOf(cMask) == -1)
		{
			maskListLock << cMask;
		}
	}

	qDebug() << maskListLock;

	foreach (MaskDescr mdes, fBit.fuseDescr)
	{
		QString cMask = mdes.mask;
		cMask.replace(QRegExp("\\d+"), "\\d+");
		cMask = "^" + cMask;
		if (maskListFuse.indexOf(cMask) == -1)
		{
			maskListFuse << cMask;
		}
	}

	qDebug() << maskListFuse;
}


void fuseModalDialog::onRead()
{
	read = true;

	emit onOk();
}


void fuseModalDialog::onFuseComboSelected()
{
}


void fuseModalDialog::onFuseBitClicked()
{
}


void fuseModalDialog::onLockComboSelected()
{
}


void fuseModalDialog::onLockBitClicked()
{
}


int fuseModalDialog::eepFindFuses(long type)
{
	for (int k = 0; eep_bits.count(); k++)
	{
		if (eep_bits.at(k).type == type)
		{
			return k;
		}
	}

	return -1;
}

/**
 * @brief
 */
void fuseModalDialog::initWidgets(const QString &msg, bool readonly)
{
	long type = cmdw->GetCurrentChipType();

	QVector<QComboBox *> lLock = (QVector<QComboBox *>() << comboBoxLock0 << comboBoxLock1 << comboBoxLock2 << comboBoxLock3);
	QVector<QComboBox *> lFuse = (QVector<QComboBox *>() << comboBoxFuse0 << comboBoxFuse1 << comboBoxFuse2 << comboBoxFuse3);
	for (int i = 0; i < 4; i++)
	{
		lLock.at(i)->setHidden(true);
		lFuse.at(i)->setHidden(true);
	}

	chkHlp1->setText(translate(STR_FUSEDLGNOTECLR) + " (bit = 1)");
	chkHlp1->setEnabled(false);
	chkHlp0->setText(translate(STR_FUSEDLGNOTESET) + " (bit = 0)");
	chkHlp0->setChecked(true);
	chkHlp0->setEnabled(false);

	if (readonly)
	{
		pushOk->setEnabled(false);
		pushWrite->setEnabled(false);
	}
	else
	{
		pushOk->setEnabled(true);
		pushWrite->setEnabled(true);
	}

	currentChip = eepFindFuses(type);

	if (currentChip < 0)
	{
		return;
	}


	ChipBits fBit = eep_bits.at(currentChip);
	if (fBit.lock.count() > 0)
	{
		unsigned int lock = awip->GetLockBits();

		for (int i = 0; i < fBit.lock.count(); i++)
		{
			QTreeWidgetItem *itm = new QTreeWidgetItem();
			int bitOffset = fBit.lock.at(i).bit;
			itm->setText(0, QString().sprintf("Bit %d, ", bitOffset) + fBit.lock.at(i).ShortDescr);
			if (fBit.lock.at(i).LongDescr.length() > 0)
			{
				itm->setText(1, fBit.lock.at(i).LongDescr);
			}
			itm->setFlags(itm->flags() | Qt::ItemIsUserCheckable);
			itm->setCheckState(0, Qt::Unchecked);

			if (lock & (1 << bitOffset))
			{
				itm->setCheckState(0, Qt::Checked);
			}
			treeWidgetLock->addTopLevelItem(itm);
		}

		treeWidgetLock->expandAll();
		treeWidgetLock->resizeColumnToContents(0);
	}

	if (fBit.fuse.count() > 0)
	{
		unsigned int fuse = awip->GetFuseBits();

		for (int i = 0; i < fBit.fuse.count(); i++)
		{
			QTreeWidgetItem *itm = new QTreeWidgetItem();
			int bitOffset = fBit.fuse.at(i).bit;
			itm->setText(0, QString().sprintf("Bit %d, ", bitOffset) + fBit.fuse.at(i).ShortDescr);
			if (fBit.fuse.at(i).LongDescr.length() > 0)
			{
				itm->setText(1, fBit.fuse.at(i).LongDescr);
			}
			itm->setFlags(itm->flags() | Qt::ItemIsUserCheckable);
			itm->setCheckState(0, Qt::Unchecked);
			if (fuse & (1 << bitOffset))
			{
				itm->setCheckState(0, Qt::Checked);
			}
			treeWidgetFuse->addTopLevelItem(itm);
		}

		treeWidgetFuse->expandAll();
		treeWidgetFuse->resizeColumnToContents(0);
	}

	scanMasks();

	// now working with maskListFuse, maskListLock

	for (int i = 0; i < maskListLock.count(); i++)
	{
		QStringList lst;
		QString currentMask = maskListLock.at(i);
		// two loops are not optimal!
		foreach (MaskDescr mdes, fBit.lockDescr)
		{
			if (mdes.mask.indexOf(QRegExp(currentMask)) >= 0)
			{
				lst << mdes.LongDescr;
			}
		}
		lLock.at(i)->setHidden(false);
		lLock.at(i)->addItems(lst);
	}

	for (int i = 0; i < maskListFuse.count(); i++)
	{
		QStringList lst;
		QString currentMask = maskListFuse.at(i);
		// two loops are not optimal!
		foreach (MaskDescr mdes, fBit.fuseDescr)
		{
			if (mdes.mask.indexOf(QRegExp(currentMask)) >= 0)
			{
				lst << mdes.LongDescr;
			}
		}
		lFuse.at(i)->setHidden(false);
		lFuse.at(i)->addItems(lst);
	}
}



QVector<ChipBits> fuseModalDialog::eep_bits =
{
	{
		AT89S8252,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 5, "LB5", ""},
			{ 6, "LB6", ""},
			{ 7, "LB7", ""}
		},
		{
			// lock mask description
			// ???
		}
	},

	{
		AT89S8253,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "LB0", ""},
			{ 1, "LB1", ""},
			{ 2, "LB2", ""}
		},
		{
			// lock mask description
			// ???
		}
	},

	{
		AT89S51,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 2, "LB2", ""},
			{ 3, "LB3", ""},
			{ 4, "LB4", ""}
		},
		{
			// lock mask description
			{ "LB=000", "Mode 1: No memory lock features enabled"},
			{ "LB=001", "Mode 2: MOVC disabled"},
			{ "LB=011", "Mode 3: Verify disabled"},
			{ "LB=111", "Mode 4: External execution disabled"}
		}
	},

	{
		AT89S52,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 2, "LB2", ""},
			{ 3, "LB3", ""},
			{ 4, "LB4", ""}
		},
		{
			// lock mask description
			{ "LB=000", "Mode 1: No memory lock features enabled"},
			{ "LB=001", "Mode 2: MOVC disabled"},
			{ "LB=011", "Mode 3: Verify disabled"},
			{ "LB=111", "Mode 4: External execution disabled"}
		}
	},

	{
		AT89S53,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 5, "LB5", ""},
			{ 6, "LB6", ""},
			{ 7, "LB7", ""}
		},
		{
			// lock mask description
			{ "LB=000", "Mode 1: No memory lock features enabled"},
			{ "LB=001", "Mode 2: MOVC disabled"},
			{ "LB=011", "Mode 3: Verify disabled"},
			{ "LB=111", "Mode 4: External execution disabled"}
		}
	},

	{
		AT90CAN32,
		{
			// fuse
			// byte low
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTC7"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			// byte high
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watchdog timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"},
			// byte ext
			{ 16, "TA0SEL", "Reserved for factory tests"},
			{ 17, "BODLEVEL0", ""},
			{ 18, "BODLEVEL1", ""},
			{ 19, "BODLEVEL2", ""}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0-16.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0-16.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0-16.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0-16.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0-16.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms; Int. Cap." },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=0110 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms; Int. Cap." },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms; Int. Cap." },
			{ "CKSEL=0101 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms" },
			{ "CKSEL=0111 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms; Int. Cap." },
			{ "CKSEL=0101 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms" },
			{ "CKSEL=0111 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms; Int. Cap." },
			{ "CKSEL=0101 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms" },
			{ "CKSEL=0111 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms; Int. Cap." },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc.; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc.; Start-up time: 6 CK + 65 ms" },

			{ "BOOTSZ=10", "Boot Flash section size=1024 words Boot start address=$3C00" },
			{ "BOOTSZ=01", "Boot Flash section size=2048 words Boot start address=$3800" },
			{ "BOOTSZ=00", "Boot Flash section size=4096 words Boot start address=$3000" },
			{ "BOOTSZ=11", "Boot Flash section size=512 words Boot start address=$3E00" },

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=000", "Brown-out detection level at VCC=2.5 V" },
			{ "BODLEVEL=001", "Brown-out detection level at VCC=2.6 V" },
			{ "BODLEVEL=010", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=011", "Brown-out detection level at VCC=3.8 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=3.9 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=4.0 V" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=4.1 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		AT90CAN64,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTC7"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watchdog timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"},
			{ 16, "TA0SEL", "Reserved for factory tests"},
			{ 17, "BODLEVEL0", ""},
			{ 18, "BODLEVEL1", ""},
			{ 19, "BODLEVEL2", ""}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0-16.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0-16.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0-16.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0-16.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0-16.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms; Int. Cap." },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=0110 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms; Int. Cap." },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms; Int. Cap." },
			{ "CKSEL=0101 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms" },
			{ "CKSEL=0111 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms; Int. Cap." },
			{ "CKSEL=0101 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms" },
			{ "CKSEL=0111 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms; Int. Cap." },
			{ "CKSEL=0101 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms" },
			{ "CKSEL=0111 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms; Int. Cap." },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc.; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc.; Start-up time: 6 CK + 65 ms" },

			{ "BOOTSZ=10", "Boot Flash section size=1024 words Boot start address=$7C00" },
			{ "BOOTSZ=01", "Boot Flash section size=2048 words Boot start address=$7800" },
			{ "BOOTSZ=00", "Boot Flash section size=4096 words Boot start address=$7000" },
			{ "BOOTSZ=11", "Boot Flash section size=512 words Boot start address=$7E00" },

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=000", "Brown-out detection level at VCC=2.5 V" },
			{ "BODLEVEL=001", "Brown-out detection level at VCC=2.6 V" },
			{ "BODLEVEL=010", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=011", "Brown-out detection level at VCC=3.8 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=3.9 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=4.0 V" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=4.1 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		AT90CAN128,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTE7"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watchdog timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"},
			{ 16, "TA0SEL", "Reserved for factory tests"},
			{ 17, "BODLEVEL0", ""},
			{ 18, "BODLEVEL1", ""},
			{ 19, "BODLEVEL2", ""}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0-16.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0-16.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0-16.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0-16.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0-16.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms; Int. Cap." },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=0110 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms; Int. Cap." },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms; Int. Cap." },
			{ "CKSEL=0101 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms" },
			{ "CKSEL=0111 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms; Int. Cap." },
			{ "CKSEL=0101 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms" },
			{ "CKSEL=0111 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms; Int. Cap." },
			{ "CKSEL=0101 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms" },
			{ "CKSEL=0111 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms; Int. Cap." },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc.; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc.; Start-up time: 6 CK + 65 ms" },

			{ "BOOTSZ=10", "Boot Flash section size=1024 words Boot start address=$FC00" },
			{ "BOOTSZ=01", "Boot Flash section size=2048 words Boot start address=$F800" },
			{ "BOOTSZ=00", "Boot Flash section size=4096 words Boot start address=$F000" },
			{ "BOOTSZ=11", "Boot Flash section size=512 words Boot start address=$FE00" },

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=000", "Brown-out detection level at VCC=2.5 V" },
			{ "BODLEVEL=001", "Brown-out detection level at VCC=2.6 V" },
			{ "BODLEVEL=010", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=011", "Brown-out detection level at VCC=3.8 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=3.9 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=4.0 V" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=4.1 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		AT90S1200,
		{
			// fuse
			{ 0, "FB0", "External clock enabled"},
			{ 5, "FB5", "Serial program downloading (SPI) enabled"}
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 1, "LB1", ""},
			{ 2, "LB2", ""}
		},
		{
			// lock mask description
			{ "LB=11", "Mode 1: No memory lock features enabled"},
			{ "LB=10", "Mode 2: Further programming disabled"},
			{ "LB=00", "Mode 3: Further programming and verification disabled"}
		}
	},
	{
		AT90S2313,
		{
			// fuse
			{ 0, "FB0", "Short start-up time enabled"},
			{ 5, "FB5", "Serial program downloading (SPI) enabled"}
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 1, "Lock1", ""},
			{ 2, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},
	{
		AT90S4414,
		{
			// fuse
			{ 0, "FB0", "Short start-up time enabled"},
			{ 1, "SPIEN", ""},
			{ 2, "FSTRT", ""},
			{ 5, "FB5", "Serial program downloading (SPI) enabled"}
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 1, "Lock1", ""},
			{ 2, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},
	{
		AT90S8515,
		{
			// fuse
			{ 0, "FB0", "Short start-up time enabled"},
			{ 1, "SPIEN", ""},
			{ 2, "FSTRT", ""},
			{ 5, "FB5", "Serial program downloading (SPI) enabled"}
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 1, "Lock1", ""},
			{ 2, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},
	{
		AT90S4434,
		{
			// fuse
			{ 0, "FB0", "Short start-up time enabled"},
			{ 1, "SPIEN", ""},
			{ 2, "FSTRT", ""},
			{ 5, "FB5", "Serial program downloading (SPI) enabled"}
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 1, "Lock1", ""},
			{ 2, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},
	{
		AT90S8535,
		{
			// fuse
			{ 0, "FB0", "Short start-up time enabled"},
			{ 1, "SPIEN", ""},
			{ 2, "FSTRT", ""},
			{ 5, "FB5", "Serial program downloading (SPI) enabled"}
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 1, "Lock1", ""},
			{ 2, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},
	{
		AT90S2333, // ???
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "BODEN", "Brown-out detection enabled"},
			{ 4, "BODLEVEL", "Brown-out detection level at VCC=2.7V"},
			{ 5, "SPIEN", "Serial program downloading (SPI) enabled"}
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 1, "Lock1", ""},
			{ 2, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},

	{
		AT90S4433,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "BODEN", "Brown-out detection enabled"},
			{ 4, "BODLEVEL", "Brown-out detection level at VCC=2.7V"},
			{ 5, "SPIEN", "Serial program downloading (SPI) enabled"}
		},
		{
			// fuse mask description
			{ "CKSEL=101", "Ceramic Resonator" },
			{ "CKSEL=111", "Ceramic Resonator; BOD Enabled or power-on reset" },
			{ "CKSEL=110", "Ceramic Resonator; fast rising power" },
			{ "CKSEL=100", "Crystal Oscillator; BOD Enabled or power-on reset" },
			{ "CKSEL=010", "Crystal Oscillator; default value" },
			{ "CKSEL=011", "Crystal Oscillator; fast rising power" },
			{ "CKSEL=001", "External Clock; BOD Enabled or power-on reset" },
			{ "CKSEL=000", "External Clock; slowly rising power" }
		},
		{
			// lock
			{ 1, "Lock1", ""},
			{ 2, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},

	{
		AT90S2323,
		{
			// fuse
			{ 0, "FB0", "Short start-up time enabled"},
			{ 5, "FB5", "Serial program downloading (SPI) enabled"}
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 1, "Lock1", ""},
			{ 2, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},

	{
		AT90S2343,
		{
			// fuse
			{ 0, "FB0", "External clock disabled"}, // enabled but invert
			{ 5, "FB5", "Serial program downloading (SPI) enabled"}
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 1, "Lock1", ""},
			{ 2, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},

	{
		AT90S8534,
		{
			// fuse
			{ 0, "FB0", "Short start-up time enabled"},
			{ 1, "SPIEN", ""},
			{ 2, "FSTRT", ""},
			{ 5, "FB5", "Serial program downloading (SPI) enabled"}
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 1, "Lock1", ""},
			{ 2, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},

	{
		ATtiny12,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "RSTDISBL", "External reset function of PB5 disabled"},
			{ 5, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 6, "BODEN", "Brown-out detection enabled"},
			{ 7, "BODLEVEL", "Brown-out detection level at VCC=1.8 V"}
		},
		{
			// fuse mask description ???
			{ "CKSEL=0000", "External Clock" },
			{ "CKSEL=0001", "External Clock" },
			{ "CKSEL=0010", "Internal RC Ocsillator" },
			{ "CKSEL=0011", "Internal RC Ocsillator" },
			{ "CKSEL=0100", "Internal RC Ocsillator" },
			{ "CKSEL=0101", "External RC Ocsillator" },
			{ "CKSEL=0110", "External RC Ocsillator" },
			{ "CKSEL=0111", "External RC Ocsillator" },
			{ "CKSEL=1000", "External Low-Frequency Crystal" },
			{ "CKSEL=1001", "External Low-Frequency Crystal" },
			{ "CKSEL=1010", "External Crystal / Ceramic Resonator" },
			{ "CKSEL=1011", "External Crystal / Ceramic Resonator" },
			{ "CKSEL=1100", "External Crystal / Ceramic Resonator" },
			{ "CKSEL=1101", "External Crystal / Ceramic Resonator" },
			{ "CKSEL=1110", "External Crystal / Ceramic Resonator" },
			{ "CKSEL=1111", "External Crystal / Ceramic Resonator" }
		},
		{
			// lock
			{ 1, "Lock1", ""},
			{ 2, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},

	{
		ATtiny13,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "SUT0", ""},
			{ 3, "SUT1", ""},
			{ 4, "CKDIV8", "Divide clock by 8 internally"},
			{ 5, "WDTON", "Watch-dog Timer always on"},
			{ 6, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 7, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 8, "RSTDISBL", "Reset Disabled (Enable PB5 as i/o pin)"},
			{ 9, "BODLEVEL0", ""},
			{ 10, "BODLEVEL1", ""},
			{ 11, "DWEN", "Debug Wire enable"},
			{ 12, "SELFPRGEN", "Self Programming enable"}
		},
		{
			// fuse mask description
			{ "CKSEL=00 SUT=00", "Ext. Clock; Start-up time: 14 CK + 0 ms" },
			{ "CKSEL=00 SUT=01", "Ext. Clock; Start-up time: 14 CK + 4 ms" },
			{ "CKSEL=00 SUT=10", "Ext. Clock; Start-up time: 14 CK + 64 ms" },
			{ "CKSEL=11 SUT=00", "Int. RC Osc. 128 kHz; Start-up time: 14 CK + 0 ms" },
			{ "CKSEL=11 SUT=01", "Int. RC Osc. 128 kHz; Start-up time: 14 CK + 4 ms" },
			{ "CKSEL=11 SUT=10", "Int. RC Osc. 128 kHz; Start-up time: 14 CK + 64 ms" },
			{ "CKSEL=01 SUT=00", "Int. RC Osc. 4.8 MHz; Start-up time: 14 CK + 0 ms" },
			{ "CKSEL=01 SUT=01", "Int. RC Osc. 4.8 MHz; Start-up time: 14 CK + 4 ms" },
			{ "CKSEL=01 SUT=10", "Int. RC Osc. 4.8 MHz; Start-up time: 14 CK + 64 ms" },
			{ "CKSEL=10 SUT=00", "Int. RC Osc. 9.6 MHz; Start-up time: 14 CK + 0 ms" },
			{ "CKSEL=10 SUT=01", "Int. RC Osc. 9.6 MHz; Start-up time: 14 CK + 4 ms" },
			{ "CKSEL=10 SUT=10", "Int. RC Osc. 9.6 MHz; Start-up time: 14 CK + 64 ms" },

			{ "BODLEVEL=11", "Brown-out detection disabled" },
			{ "BODLEVEL=10", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=01", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=00", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},
	{
		ATtiny15,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 4, "RSTDISBL", "External reset function of PB5 disabled"},
			{ 5, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 6, "BODEN", "Brown-out detection enabled"},
			{ 7, "BODLEVEL", "Brown-out detection level at VCC=2.7 V"}
		},
		{
			// fuse mask description
			{ "CKSEL=00", "Slowly rising power"},
			{ "CKSEL=01", "Slowly rising power"},
			{ "CKSEL=10", "Quickly rising power"},
			{ "CKSEL=11", "Very quickly rising power"}
		},
		{
			// lock
			{ 1, "Lock1", ""},
			{ 2, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},

	{
		ATtiny22,
		{
			// fuse
			{ 0, "FB0", "External clock disabled"}, // NOT enabled
			{ 5, "FB5", "Serial program downloading (SPI) enabled"}
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 1, "Lock1", ""},
			{ 2, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},
	{
		ATtiny26,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOPT", "CKOPT fuse (operation dependent of CKSEL fuses)"},
			{ 7, "PLLCK", ""},
			{ 8, "BODEN", "Brown-out detection enabled"},
			{ 9, "BODLEVEL", "Brown-out detection level at VCC=2.7 V"},
			{ 10, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 11, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 12, "RSTDISBL", "Reset Disabled (Enable PB7 as i/o pin)"}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00 PLLCK=1", "Ext. Clock; Start-up time: 6 CK + 0 ms"},
			{ "CKSEL=0000 SUT=01 PLLCK=1", "Ext. Clock; Start-up time: 6 CK + 4 ms"},
			{ "CKSEL=0000 SUT=10 PLLCK=1", "Ext. Clock; Start-up time: 6 CK + 64 ms"},
			{ "CKSEL=1111 SUT=01 PLLCK=1", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 0 ms"},
			{ "CKSEL=1111 SUT=10 PLLCK=1", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 4 ms"},
			{ "CKSEL=1111 SUT=11 PLLCK=1", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 64 ms"},
			{ "CKSEL=1110 SUT=10 PLLCK=1", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 0 ms"},
			{ "CKSEL=1110 SUT=11 PLLCK=1", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 4 ms"},
			{ "CKSEL=1111 SUT=00 PLLCK=1", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 64 ms"},
			{ "CKSEL=1110 SUT=00 PLLCK=1", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 4 ms"},
			{ "CKSEL=1110 SUT=01 PLLCK=1", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 64 ms"},
			{ "CKSEL=1011 SUT=01 PLLCK=1", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 0 ms"},
			{ "CKSEL=1011 SUT=10 PLLCK=1", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 4 ms"},
			{ "CKSEL=1011 SUT=11 PLLCK=1", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 64 ms"},
			{ "CKSEL=1010 SUT=10 PLLCK=1", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 0 ms"},
			{ "CKSEL=1010 SUT=11 PLLCK=1", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 4 ms"},
			{ "CKSEL=1011 SUT=00 PLLCK=1", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 64 ms"},
			{ "CKSEL=1010 SUT=00 PLLCK=1", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 4 ms"},
			{ "CKSEL=1010 SUT=01 PLLCK=1", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 64 ms"},
			{ "CKSEL=1101 SUT=01 PLLCK=1", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 0 ms"},
			{ "CKSEL=1101 SUT=10 PLLCK=1", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 4 ms"},
			{ "CKSEL=1101 SUT=11 PLLCK=1", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 64 ms"},
			{ "CKSEL=1100 SUT=10 PLLCK=1", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 0 ms"},
			{ "CKSEL=1100 SUT=11 PLLCK=1", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 4 ms"},
			{ "CKSEL=1101 SUT=00 PLLCK=1", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 64 ms"},
			{ "CKSEL=1100 SUT=00 PLLCK=1", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 4 ms"},
			{ "CKSEL=1100 SUT=01 PLLCK=1", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 64 ms"},
			{ "CKSEL=1001 SUT=00 PLLCK=1", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4 ms"},
			{ "CKSEL=1001 SUT=01 PLLCK=1", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 64 ms"},
			{ "CKSEL=1001 SUT=10 PLLCK=1", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 64 ms"},
			{ "CKSEL=0101 SUT=00 PLLCK=1", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 0 ms"},
			{ "CKSEL=0101 SUT=01 PLLCK=1", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 4 ms"},
			{ "CKSEL=0101 SUT=10 PLLCK=1", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 64 ms"},
			{ "CKSEL=0101 SUT=11 PLLCK=1", "Ext. RC Osc. - 0.9 MHz; Start-up time: 6 CK + 4 ms"},
			{ "CKSEL=0110 SUT=00 PLLCK=1", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 0 ms"},
			{ "CKSEL=0110 SUT=01 PLLCK=1", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 4 ms"},
			{ "CKSEL=0110 SUT=10 PLLCK=1", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 64 ms"},
			{ "CKSEL=0110 SUT=11 PLLCK=1", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 6 CK + 4 ms"},
			{ "CKSEL=0111 SUT=00 PLLCK=1", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 0 ms"},
			{ "CKSEL=0111 SUT=01 PLLCK=1", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 4 ms"},
			{ "CKSEL=0111 SUT=10 PLLCK=1", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 64 ms"},
			{ "CKSEL=0111 SUT=11 PLLCK=1", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 6 CK + 4 ms"},
			{ "CKSEL=1000 SUT=00 PLLCK=1", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 0 ms"},
			{ "CKSEL=1000 SUT=01 PLLCK=1", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 4 ms"},
			{ "CKSEL=1000 SUT=10 PLLCK=1", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 64 ms"},
			{ "CKSEL=1000 SUT=11 PLLCK=1", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 6 CK + 4 ms"},
			{ "CKSEL=0001 SUT=00 PLLCK=1", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 0 ms"},
			{ "CKSEL=0001 SUT=01 PLLCK=1", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 4 ms"},
			{ "CKSEL=0001 SUT=10 PLLCK=1", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 64 ms"},
			{ "CKSEL=0010 SUT=00 PLLCK=1", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 0 ms"},
			{ "CKSEL=0010 SUT=01 PLLCK=1", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 4 ms"},
			{ "CKSEL=0010 SUT=10 PLLCK=1", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 64 ms"},
			{ "CKSEL=0011 SUT=00 PLLCK=1", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 0 ms"},
			{ "CKSEL=0011 SUT=01 PLLCK=1", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 4 ms"},
			{ "CKSEL=0011 SUT=10 PLLCK=1", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 64 ms"},
			{ "CKSEL=0100 SUT=00 PLLCK=1", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 0 ms"},
			{ "CKSEL=0100 SUT=01 PLLCK=1", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 4 ms"},
			{ "CKSEL=0100 SUT=10 PLLCK=1", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 64 ms"},
			{ "CKSEL=0001 SUT=11 PLLCK=0", "PLL Clock; Start-up time: 16K CK + 64 ms"},
			{ "CKSEL=0001 SUT=00 PLLCK=0", "PLL Clock; Start-up time: 1K CK + 0 ms"},
			{ "CKSEL=0001 SUT=01 PLLCK=0", "PLL Clock; Start-up time: 1K CK + 4 ms"},
			{ "CKSEL=0001 SUT=10 PLLCK=0", "PLL Clock; Start-up time: 1K CK + 64 ms"}
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},

	{
		ATtiny2313,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTD2"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "RSTDISBL", "Reset Disabled (Enable PA2 as i/o pin)"},
			{ 9, "BODLEVEL0", ""},
			{ 10, "BODLEVEL1", ""},
			{ 11, "BODLEVEL2", ""},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 15, "DWEN", "Debug Wire enable"},
			{ 16, "SELFPRGEN", "Self programming enable"}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 14 CK + 0 ms"},
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 14 CK + 4.1 ms"},
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 14 CK + 65 ms"},
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 14 CK + 0 ms"},
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 14 CK + 0 ms"},
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 14 CK + 4.1 ms"},
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 14 CK + 4.1 ms"},
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 14 CK + 4.1 ms"},
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 14 CK + 65 ms"},
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 14 CK + 65 ms"},
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 14 CK + 65 ms"},
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 14 CK + 0 ms"},
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 14 CK + 0 ms"},
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 14 CK + 4.1 ms"},
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 14 CK + 4.1 ms"},
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 14 CK + 4.1 ms"},
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 14 CK + 65 ms"},
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 14 CK + 65 ms"},
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 14 CK + 65 ms"},
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 14 CK + 0 ms"},
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 14 CK + 0 ms"},
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 14 CK + 4.1 ms"},
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 14 CK + 4.1 ms"},
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 14 CK + 4.1 ms"},
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 14 CK + 65 ms"},
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 14 CK + 65 ms"},
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 14 CK + 65 ms"},
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 14 CK + 0 ms"},
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 14 CK + 0 ms"},
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 14 CK + 4.1 ms"},
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 14 CK + 4.1 ms"},
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 14 CK + 4.1 ms"},
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 14 CK + 65 ms"},
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 14 CK + 65 ms"},
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 14 CK + 65 ms"},
			{ "CKSEL=0110 SUT=00", "Int. RC Osc. 128 kHz; Start-up time: 14 CK + 0 ms"},
			{ "CKSEL=0110 SUT=01", "Int. RC Osc. 128 kHz; Start-up time: 14 CK + 4 ms"},
			{ "CKSEL=0110 SUT=10", "Int. RC Osc. 128 kHz; Start-up time: 14 CK + 64 ms"},
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 4 MHz; Start-up time: 14 CK + 0 ms"},
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 4 MHz; Start-up time: 14 CK + 4.1 ms"},
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 4 MHz; Start-up time: 14 CK + 65 ms"},
			{ "CKSEL=0100 SUT=00", "Int. RC Osc. 8 MHz; Start-up time: 14 CK + 0 ms"},
			{ "CKSEL=0100 SUT=01", "Int. RC Osc. 8 MHz; Start-up time: 14 CK + 4.1 ms"},
			{ "CKSEL=0100 SUT=10", "Int. RC Osc. 8 MHz; Start-up time: 14 CK + 65 ms"},

			{ "BODLEVEL=11", "Brown-out detection disabled" },
			{ "BODLEVEL=10", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=01", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=00", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},
	{
		ATtiny25,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTB4"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BODLEVEL0", ""},
			{ 9, "BODLEVEL1", ""},
			{ 10, "BODLEVEL2", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "DWEN", "Debug Wire enable"},
			{ 15, "RSTDISBL", "Reset Disabled (Enable PB5 as i/o pin)"},
			{ 16, "SELFPRGEN", "Self Programming enable"}
		},
		{
			// fuse mask description
			{ "CKSEL=0011 SUT=11", "ATtiny15 Comp: Int. RC Osc. 6.4 MHz; Start-up time PWRDWN/RESET: 1 CK/14 CK + 0 ms" },
			{ "CKSEL=0011 SUT=10", "ATtiny15 Comp: Int. RC Osc. 6.4 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0011 SUT=00", "ATtiny15 Comp: Int. RC Osc. 6.4 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },
			{ "CKSEL=0011 SUT=01", "ATtiny15 Comp: Int. RC Osc. 6.4 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 65 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1K CK/14 CK + 0 ms" },
			{ "CKSEL=0110 SUT=01", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1K CK/14 CK + 4 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 32K CK/14 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },
			{ "CKSEL=0001 SUT=01", "PLL Clock; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4 ms" },
			{ "CKSEL=0001 SUT=11", "PLL Clock; Start-up time PWRDWN/RESET: 16K CK/14 CK + 64 ms" },
			{ "CKSEL=0001 SUT=00", "PLL Clock; Start-up time PWRDWN/RESET: 1K CK/14 CK + 4 ms" },
			{ "CKSEL=0001 SUT=10", "PLL Clock; Start-up time PWRDWN/RESET: 1K CK/14 CK + 64 ms" },
			{ "CKSEL=0100 SUT=00", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0100 SUT=10", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},

	{
		ATtiny45,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTB4"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BODLEVEL0", ""},
			{ 9, "BODLEVEL1", ""},
			{ 10, "BODLEVEL2", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "DWEN", "Debug Wire enable"},
			{ 15, "RSTDISBL", "Reset Disabled (Enable PB5 as i/o pin)"},
			{ 16, "SELFPRGEN", "Self Programming enable"}
		},
		{
			// fuse mask description
			{ "CKSEL=0011 SUT=11", "ATtiny15 Comp: Int. RC Osc. 6.4 MHz; Start-up time PWRDWN/RESET: 1 CK/14 CK + 0 ms" },
			{ "CKSEL=0011 SUT=10", "ATtiny15 Comp: Int. RC Osc. 6.4 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0011 SUT=00", "ATtiny15 Comp: Int. RC Osc. 6.4 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },
			{ "CKSEL=0011 SUT=01", "ATtiny15 Comp: Int. RC Osc. 6.4 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 65 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1K CK/14 CK + 0 ms" },
			{ "CKSEL=0110 SUT=01", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1K CK/14 CK + 4 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 32K CK/14 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },
			{ "CKSEL=0001 SUT=01", "PLL Clock; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4 ms" },
			{ "CKSEL=0001 SUT=11", "PLL Clock; Start-up time PWRDWN/RESET: 16K CK/14 CK + 64 ms" },
			{ "CKSEL=0001 SUT=00", "PLL Clock; Start-up time PWRDWN/RESET: 1K CK/14 CK + 4 ms" },
			{ "CKSEL=0001 SUT=10", "PLL Clock; Start-up time PWRDWN/RESET: 1K CK/14 CK + 64 ms" },
			{ "CKSEL=0100 SUT=00", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0100 SUT=10", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},
	{
		ATtiny85,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTB4"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BODLEVEL0", ""},
			{ 9, "BODLEVEL1", ""},
			{ 10, "BODLEVEL2", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "DWEN", "Debug Wire enable"},
			{ 15, "RSTDISBL", "Reset Disabled (Enable PB5 as i/o pin)"},
			{ 16, "SELFPRGEN", "Self Programming enable"}
		},
		{
			// fuse mask description
			{ "CKSEL=0011 SUT=11", "ATtiny15 Comp: Int. RC Osc. 6.4 MHz; Start-up time PWRDWN/RESET: 1 CK/14 CK + 0 ms" },
			{ "CKSEL=0011 SUT=10", "ATtiny15 Comp: Int. RC Osc. 6.4 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0011 SUT=00", "ATtiny15 Comp: Int. RC Osc. 6.4 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },
			{ "CKSEL=0011 SUT=01", "ATtiny15 Comp: Int. RC Osc. 6.4 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 65 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1K CK/14 CK + 0 ms" },
			{ "CKSEL=0110 SUT=01", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1K CK/14 CK + 4 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 32K CK/14 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },
			{ "CKSEL=0001 SUT=01", "PLL Clock; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4 ms" },
			{ "CKSEL=0001 SUT=11", "PLL Clock; Start-up time PWRDWN/RESET: 16K CK/14 CK + 64 ms" },
			{ "CKSEL=0001 SUT=00", "PLL Clock; Start-up time PWRDWN/RESET: 1K CK/14 CK + 4 ms" },
			{ "CKSEL=0001 SUT=10", "PLL Clock; Start-up time PWRDWN/RESET: 1K CK/14 CK + 64 ms" },
			{ "CKSEL=0100 SUT=00", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0100 SUT=10", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},

	{
		ATtiny261,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTB5"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BODLEVEL0", ""},
			{ 9, "BODLEVEL1", ""},
			{ 10, "BODLEVEL2", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "DWEN", "Debug Wire enable"},
			{ 15, "RSTDISBL", "Reset Disabled (Enable PB7 as i/o pin)"},
			{ 16, "SELFPRGEN", "Self Programming enable"}
		},
		{
			// fuse mask description
			{ "CKSEL=1000 SUT=10", "Ext. Ceramic Res.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Ceramic Res.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Ceramic Res.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Ceramic Res.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Ceramic Res.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Ceramic Res.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Ceramic Res.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Ceramic Res.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Ceramic Res.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Ceramic Res.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Ceramic Res.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Ceramic Res.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Ceramic Res.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Ceramic Res.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Ceramic Res.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Ceramic Res.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Ceramic Res.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Ceramic Res.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Ceramic Res.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Ceramic Res.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1 CK + 64 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1 CK 4 ms" },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 32 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },
			{ "CKSEL=0001 SUT=11", "PLL Clock; Start-up time PWRDWN/RESET: 16K CK/14 CK + 68 ms" },
			{ "CKSEL=0001 SUT=01", "PLL Clock; Start-up time PWRDWN/RESET: 16K CK/14 CK + 8 ms" },
			{ "CKSEL=0001 SUT=10", "PLL Clock; Start-up time PWRDWN/RESET: 1K CK/14 CK + 68 ms" },
			{ "CKSEL=0001 SUT=00", "PLL Clock; Start-up time PWRDWN/RESET: 1K CK/14 CK + 8 ms" },
			{ "CKSEL=0011 SUT=00", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },

			// brown out
			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=001", "Brown-out detection level at VCC=1.9 V" },
			{ "BODLEVEL=000", "Brown-out detection level at VCC=2.0 V" },
			{ "BODLEVEL=010", "Brown-out detection level at VCC=2.2 V" },
			{ "BODLEVEL=011", "Brown-out detection level at VCC=2.3 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},

	{
		ATtiny461,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTB5"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BODLEVEL0", ""},
			{ 9, "BODLEVEL1", ""},
			{ 10, "BODLEVEL2", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "DWEN", "Debug Wire enable"},
			{ 15, "RSTDISBL", "Reset Disabled (Enable PB7 as i/o pin)"},
			{ 16, "SELFPRGEN", "Self Programming enable"}
		},
		{
			// fuse mask description
			{ "CKSEL=1000 SUT=10", "Ext. Ceramic Res.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Ceramic Res.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Ceramic Res.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Ceramic Res.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Ceramic Res.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Ceramic Res.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Ceramic Res.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Ceramic Res.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Ceramic Res.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Ceramic Res.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Ceramic Res.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Ceramic Res.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Ceramic Res.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Ceramic Res.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Ceramic Res.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Ceramic Res.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Ceramic Res.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Ceramic Res.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Ceramic Res.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Ceramic Res.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1 CK + 64 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1 CK 4 ms" },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 32 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },
			{ "CKSEL=0001 SUT=11", "PLL Clock; Start-up time PWRDWN/RESET: 16K CK/14 CK + 68 ms" },
			{ "CKSEL=0001 SUT=01", "PLL Clock; Start-up time PWRDWN/RESET: 16K CK/14 CK + 8 ms" },
			{ "CKSEL=0001 SUT=10", "PLL Clock; Start-up time PWRDWN/RESET: 1K CK/14 CK + 68 ms" },
			{ "CKSEL=0001 SUT=00", "PLL Clock; Start-up time PWRDWN/RESET: 1K CK/14 CK + 8 ms" },
			{ "CKSEL=0011 SUT=00", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },

			// brown out
			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=001", "Brown-out detection level at VCC=1.9 V" },
			{ "BODLEVEL=000", "Brown-out detection level at VCC=2.0 V" },
			{ "BODLEVEL=010", "Brown-out detection level at VCC=2.2 V" },
			{ "BODLEVEL=011", "Brown-out detection level at VCC=2.3 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},

	{
		ATtiny861,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTB5"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BODLEVEL0", ""},
			{ 9, "BODLEVEL1", ""},
			{ 10, "BODLEVEL2", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "DWEN", "Debug Wire enable"},
			{ 15, "RSTDISBL", "Reset Disabled (Enable PB7 as i/o pin)"},
			{ 16, "SELFPRGEN", "Self Programming enable"}
		},
		{
			// fuse mask description
			{ "CKSEL=1000 SUT=10", "Ext. Ceramic Res.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Ceramic Res.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Ceramic Res.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Ceramic Res.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Ceramic Res.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Ceramic Res.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Ceramic Res.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Ceramic Res.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Ceramic Res.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Ceramic Res.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Ceramic Res.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Ceramic Res.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Ceramic Res.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Ceramic Res.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Ceramic Res.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Ceramic Res.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Ceramic Res.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Ceramic Res.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Ceramic Res.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Ceramic Res.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1 CK + 64 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1 CK 4 ms" },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 32 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },
			{ "CKSEL=0001 SUT=11", "PLL Clock; Start-up time PWRDWN/RESET: 16K CK/14 CK + 68 ms" },
			{ "CKSEL=0001 SUT=01", "PLL Clock; Start-up time PWRDWN/RESET: 16K CK/14 CK + 8 ms" },
			{ "CKSEL=0001 SUT=10", "PLL Clock; Start-up time PWRDWN/RESET: 1K CK/14 CK + 68 ms" },
			{ "CKSEL=0001 SUT=00", "PLL Clock; Start-up time PWRDWN/RESET: 1K CK/14 CK + 8 ms" },
			{ "CKSEL=0011 SUT=00", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "WD. Osc. 128 kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 64 ms" },

			// brown out
			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=001", "Brown-out detection level at VCC=1.9 V" },
			{ "BODLEVEL=000", "Brown-out detection level at VCC=2.0 V" },
			{ "BODLEVEL=010", "Brown-out detection level at VCC=2.2 V" },
			{ "BODLEVEL=011", "Brown-out detection level at VCC=2.3 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled"},
			{ "Lock=10", "Mode 2: Further programming disabled"},
			{ "Lock=00", "Mode 3: Further programming and verification disabled"}
		}
	},

	{
		ATmega48,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTB0"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BODLEVEL0", ""},
			{ 9, "BODLEVEL1", ""},
			{ 10, "BODLEVEL2", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "DWEN", "Debug Wire enable"},
			{ 15, "RSTDISBL", "Reset Disabled (Enable PB6 as i/o pin)"},
			{ 16, "SELFPRGEN", "Self Programming enable"}
		},
		{
			// fuse mask description
			{ "CKSEL=1100 SUT=00", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 65 ms" },
			{ "CKSEL=1111 SUT=00", "Int. RC Osc. 128kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=1111 SUT=01", "Int. RC Osc. 128kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=10", "Int. RC Osc. 128kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=1110 SUT=01", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=10", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 65 ms" },

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" }
		}
	},

	{
		ATmega88,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTB0"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BODLEVEL0", ""},
			{ 9, "BODLEVEL1", ""},
			{ 10, "BODLEVEL2", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "DWEN", "Debug Wire enable"},
			{ 15, "RSTDISBL", "Reset Disabled (Enable PC6 as i/o pin)"},
			{ 16, "SELFPRGEN", "Self Programming enable"}
		},
		{
			// fuse mask description
			{ "CKSEL=1100 SUT=00", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 65 ms" },
			{ "CKSEL=1111 SUT=00", "Int. RC Osc. 128kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=1111 SUT=01", "Int. RC Osc. 128kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=10", "Int. RC Osc. 128kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=1110 SUT=01", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=10", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 65 ms" },

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" }
		}
	},

	{
		ATmega168,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTB0"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BODLEVEL0", ""},
			{ 9, "BODLEVEL1", ""},
			{ 10, "BODLEVEL2", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "DWEN", "Debug Wire enable"},
			{ 15, "RSTDISBL", "Reset Disabled (Enable PC6 as i/o pin)"},
			{ 16, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 17, "BOOTSZ0", ""},
			{ 18, "BOOTSZ1", ""}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 65 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=0111 SUT=01", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=0111 SUT=10", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=0111 SUT=11", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=0110 SUT=11", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms"},
			{ "CKSEL=0111 SUT=00", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms"},
			{ "CKSEL=0110 SUT=01", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1K CK/14 CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1K CK/14 CK + 4.1 ms" },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1K CK/14 CK + 65 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 32K CK/14 CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 32K CK/14 CK + 4.1 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 32K CK/14 CK + 65 ms" },
			{ "CKSEL=0011 SUT=00", "Int. RC Osc. 128kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. RC Osc. 128kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4.1 ms" },
			{ "CKSEL=0011 SUT=10", "Int. RC Osc. 128kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 65 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4.1 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 65 ms" },

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" },

			// extended
			{ "BOOTSZ=00", "Boot Flash section size=1024 words Boot start address=$1C00" },
			{ "BOOTSZ=11", "Boot Flash section size=128 words Boot start address=$1F80" },
			{ "BOOTSZ=10", "Boot Flash section size=256 words Boot start address=$1F00" },
			{ "BOOTSZ=01", "Boot Flash section size=512 words Boot start address=$1E00" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega328,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTB0"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "DWEN", "Debug Wire enable"},
			{ 15, "RSTDISBL", "Reset Disabled (Enable PC6 as i/o pin)"},
			{ 16, "BODLEVEL0", ""},
			{ 17, "BODLEVEL1", ""},
			{ 18, "BODLEVEL2", ""}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time PWRDWN/RESET: 6 CK/14 CK + 65 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=0111 SUT=01", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 16K CK/14 CK + 0 ms" },
			{ "CKSEL=0111 SUT=10", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 16K CK/14 CK + 4.1 ms" },
			{ "CKSEL=0111 SUT=11", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 16K CK/14 CK + 65 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 1K CK /14 CK + 0 ms" },
			{ "CKSEL=0110 SUT=11", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 1K CK /14 CK + 4.1 ms"},
			{ "CKSEL=0111 SUT=00", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 1K CK /14 CK + 65 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 258 CK/14 CK + 4.1 ms"},
			{ "CKSEL=0110 SUT=01", "Ext. Full-swing Crystal; Start-up time PWRDWN/RESET: 258 CK/14 CK + 65 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1K CK/14 CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1K CK/14 CK + 4.1 ms" },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 1K CK/14 CK + 65 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 32K CK/14 CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 32K CK/14 CK + 4.1 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. Low-Freq. Crystal; Start-up time PWRDWN/RESET: 32K CK/14 CK + 65 ms" },
			{ "CKSEL=0011 SUT=00", "Int. RC Osc. 128kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. RC Osc. 128kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4.1 ms" },
			{ "CKSEL=0011 SUT=10", "Int. RC Osc. 128kHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 65 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 4.1 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 8 MHz; Start-up time PWRDWN/RESET: 6 CK/14 CK + 65 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$3C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$3800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$3F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$3E00"},

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega164,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTB1"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"},
			{ 16, "BODLEVEL0", ""},
			{ 17, "BODLEVEL1", ""},
			{ 18, "BODLEVEL2", ""}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 65 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms" },
			{ "CKSEL=0111 SUT=01", "Full Swing Oscillator; Start-up time: 16K CK + 0 ms; Crystal Osc.; BOD enabled" },
			{ "CKSEL=0111 SUT=10", "Full Swing Oscillator; Start-up time: 16K CK + 4.1 ms; Crystal Osc.; fast rising power" },
			{ "CKSEL=0111 SUT=11", "Full Swing Oscillator; Start-up time: 16K CK + 65 ms; Crystal Osc.; slowly rising power" },
			{ "CKSEL=0110 SUT=10", "Full Swing Oscillator; Start-up time: 1K CK + 0 ms; Ceramic res.; BOD enable" },
			{ "CKSEL=0110 SUT=11", "Full Swing Oscillator; Start-up time: 1K CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0111 SUT=00", "Full Swing Oscillator; Start-up time: 1K CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0110 SUT=00", "Full Swing Oscillator; Start-up time: 258 CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0110 SUT=01", "Full Swing Oscillator; Start-up time: 258 CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0011 SUT=00", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc.; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc.; Start-up time: 6 CK + 65 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$3C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$3800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$3F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$3E00"},

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega324,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTB1"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"},
			{ 16, "BODLEVEL0", ""},
			{ 17, "BODLEVEL1", ""},
			{ 18, "BODLEVEL2", ""}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 65 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms" },
			{ "CKSEL=0111 SUT=01", "Full Swing Oscillator; Start-up time: 16K CK + 0 ms; Crystal Osc.; BOD enabled" },
			{ "CKSEL=0111 SUT=10", "Full Swing Oscillator; Start-up time: 16K CK + 4.1 ms; Crystal Osc.; fast rising power" },
			{ "CKSEL=0111 SUT=11", "Full Swing Oscillator; Start-up time: 16K CK + 65 ms; Crystal Osc.; slowly rising power" },
			{ "CKSEL=0110 SUT=10", "Full Swing Oscillator; Start-up time: 1K CK + 0 ms; Ceramic res.; BOD enable" },
			{ "CKSEL=0110 SUT=11", "Full Swing Oscillator; Start-up time: 1K CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0111 SUT=00", "Full Swing Oscillator; Start-up time: 1K CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0110 SUT=00", "Full Swing Oscillator; Start-up time: 258 CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0110 SUT=01", "Full Swing Oscillator; Start-up time: 258 CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0011 SUT=00", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc.; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc.; Start-up time: 6 CK + 65 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$3C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$3800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$3F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$3E00"},

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega644,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTB1"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"},
			{ 16, "BODLEVEL0", ""},
			{ 17, "BODLEVEL1", ""},
			{ 18, "BODLEVEL2", ""}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 65 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms" },
			{ "CKSEL=0111 SUT=01", "Full Swing Oscillator; Start-up time: 16K CK + 0 ms; Crystal Osc.; BOD enabled" },
			{ "CKSEL=0111 SUT=10", "Full Swing Oscillator; Start-up time: 16K CK + 4.1 ms; Crystal Osc.; fast rising power" },
			{ "CKSEL=0111 SUT=11", "Full Swing Oscillator; Start-up time: 16K CK + 65 ms; Crystal Osc.; slowly rising power" },
			{ "CKSEL=0110 SUT=10", "Full Swing Oscillator; Start-up time: 1K CK + 0 ms; Ceramic res.; BOD enable" },
			{ "CKSEL=0110 SUT=11", "Full Swing Oscillator; Start-up time: 1K CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0111 SUT=00", "Full Swing Oscillator; Start-up time: 1K CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0110 SUT=00", "Full Swing Oscillator; Start-up time: 258 CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0110 SUT=01", "Full Swing Oscillator; Start-up time: 258 CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0011 SUT=00", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc.; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc.; Start-up time: 6 CK + 65 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$7C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$7800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$7F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$7E00"},

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega640,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTE7"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"},
			{ 16, "BODLEVEL0", ""},
			{ 17, "BODLEVEL1", ""},
			{ 18, "BODLEVEL2", ""}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 65 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms" },
			{ "CKSEL=0111 SUT=01", "Full Swing Oscillator; Start-up time: 16K CK + 0 ms; Crystal Osc.; BOD enabled" },
			{ "CKSEL=0111 SUT=10", "Full Swing Oscillator; Start-up time: 16K CK + 4.1 ms; Crystal Osc.; fast rising power" },
			{ "CKSEL=0111 SUT=11", "Full Swing Oscillator; Start-up time: 16K CK + 65 ms; Crystal Osc.; slowly rising power" },
			{ "CKSEL=0110 SUT=10", "Full Swing Oscillator; Start-up time: 1K CK + 0 ms; Ceramic res.; BOD enable" },
			{ "CKSEL=0110 SUT=11", "Full Swing Oscillator; Start-up time: 1K CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0111 SUT=00", "Full Swing Oscillator; Start-up time: 1K CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0110 SUT=00", "Full Swing Oscillator; Start-up time: 258 CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0110 SUT=01", "Full Swing Oscillator; Start-up time: 258 CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0011 SUT=00", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc.; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc.; Start-up time: 6 CK + 65 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$7C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$7800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$7F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$7E00"},

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega1280,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTE7"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"},
			{ 16, "BODLEVEL0", ""},
			{ 17, "BODLEVEL1", ""},
			{ 18, "BODLEVEL2", ""}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 65 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms" },
			{ "CKSEL=0111 SUT=01", "Full Swing Oscillator; Start-up time: 16K CK + 0 ms; Crystal Osc.; BOD enabled" },
			{ "CKSEL=0111 SUT=10", "Full Swing Oscillator; Start-up time: 16K CK + 4.1 ms; Crystal Osc.; fast rising power" },
			{ "CKSEL=0111 SUT=11", "Full Swing Oscillator; Start-up time: 16K CK + 65 ms; Crystal Osc.; slowly rising power" },
			{ "CKSEL=0110 SUT=10", "Full Swing Oscillator; Start-up time: 1K CK + 0 ms; Ceramic res.; BOD enable" },
			{ "CKSEL=0110 SUT=11", "Full Swing Oscillator; Start-up time: 1K CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0111 SUT=00", "Full Swing Oscillator; Start-up time: 1K CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0110 SUT=00", "Full Swing Oscillator; Start-up time: 258 CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0110 SUT=01", "Full Swing Oscillator; Start-up time: 258 CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0011 SUT=00", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc.; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc.; Start-up time: 6 CK + 65 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$7C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$7800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$7F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$7E00"},

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega1281,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTE7"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"},
			{ 16, "BODLEVEL0", ""},
			{ 17, "BODLEVEL1", ""},
			{ 18, "BODLEVEL2", ""}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 65 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms" },
			{ "CKSEL=0111 SUT=01", "Full Swing Oscillator; Start-up time: 16K CK + 0 ms; Crystal Osc.; BOD enabled" },
			{ "CKSEL=0111 SUT=10", "Full Swing Oscillator; Start-up time: 16K CK + 4.1 ms; Crystal Osc.; fast rising power" },
			{ "CKSEL=0111 SUT=11", "Full Swing Oscillator; Start-up time: 16K CK + 65 ms; Crystal Osc.; slowly rising power" },
			{ "CKSEL=0110 SUT=10", "Full Swing Oscillator; Start-up time: 1K CK + 0 ms; Ceramic res.; BOD enable" },
			{ "CKSEL=0110 SUT=11", "Full Swing Oscillator; Start-up time: 1K CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0111 SUT=00", "Full Swing Oscillator; Start-up time: 1K CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0110 SUT=00", "Full Swing Oscillator; Start-up time: 258 CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0110 SUT=01", "Full Swing Oscillator; Start-up time: 258 CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0011 SUT=00", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc.; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc.; Start-up time: 6 CK + 65 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$7C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$7800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$7F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$7E00"},

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega2560,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTE7"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"},
			{ 16, "BODLEVEL0", ""},
			{ 17, "BODLEVEL1", ""},
			{ 18, "BODLEVEL2", ""}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 65 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms" },
			{ "CKSEL=0111 SUT=01", "Full Swing Oscillator; Start-up time: 16K CK + 0 ms; Crystal Osc.; BOD enabled" },
			{ "CKSEL=0111 SUT=10", "Full Swing Oscillator; Start-up time: 16K CK + 4.1 ms; Crystal Osc.; fast rising power" },
			{ "CKSEL=0111 SUT=11", "Full Swing Oscillator; Start-up time: 16K CK + 65 ms; Crystal Osc.; slowly rising power" },
			{ "CKSEL=0110 SUT=10", "Full Swing Oscillator; Start-up time: 1K CK + 0 ms; Ceramic res.; BOD enable" },
			{ "CKSEL=0110 SUT=11", "Full Swing Oscillator; Start-up time: 1K CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0111 SUT=00", "Full Swing Oscillator; Start-up time: 1K CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0110 SUT=00", "Full Swing Oscillator; Start-up time: 258 CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0110 SUT=01", "Full Swing Oscillator; Start-up time: 258 CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0011 SUT=00", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc.; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc.; Start-up time: 6 CK + 65 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$7C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$7800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$7F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$7E00"},

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega2561,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTE7"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watch-dog Timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"},
			{ 16, "BODLEVEL0", ""},
			{ 17, "BODLEVEL1", ""},
			{ 18, "BODLEVEL2", ""}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 65 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms" },
			{ "CKSEL=0111 SUT=01", "Full Swing Oscillator; Start-up time: 16K CK + 0 ms; Crystal Osc.; BOD enabled" },
			{ "CKSEL=0111 SUT=10", "Full Swing Oscillator; Start-up time: 16K CK + 4.1 ms; Crystal Osc.; fast rising power" },
			{ "CKSEL=0111 SUT=11", "Full Swing Oscillator; Start-up time: 16K CK + 65 ms; Crystal Osc.; slowly rising power" },
			{ "CKSEL=0110 SUT=10", "Full Swing Oscillator; Start-up time: 1K CK + 0 ms; Ceramic res.; BOD enable" },
			{ "CKSEL=0110 SUT=11", "Full Swing Oscillator; Start-up time: 1K CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0111 SUT=00", "Full Swing Oscillator; Start-up time: 1K CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0110 SUT=00", "Full Swing Oscillator; Start-up time: 258 CK + 4.1 ms; Ceramic res.; fast rising power" },
			{ "CKSEL=0110 SUT=01", "Full Swing Oscillator; Start-up time: 258 CK + 65 ms; Ceramic res.; slowly rising power" },
			{ "CKSEL=0011 SUT=00", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "Int. 128kHz RC Osc.; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc.; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc.; Start-up time: 6 CK + 65 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$7C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$7800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$7F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$7E00"},

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" }
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega603, // ???
		{
			// fuse
			{ 0, "SUT0", ""},
			{ 1, "SUT1", ""},
			{ 3, "EESAVE", ""}
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 1, "Lock1", ""},
			{ 2, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" }
		}
	},

	{
		ATmega103, // ???
		{
			// fuse
			{ 0, "SUT0", ""},
			{ 1, "SUT1", ""},
			{ 3, "EESAVE", ""}
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 1, "Lock1", ""},
			{ 2, "Lock2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" }
		}
	},

	{
		ATmega8,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "BODEN", "Brown-out detection enabled"},
			{ 7, "BODLEVEL", "No Brown-out detection level at VCC=2.7 V"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "CKOPT", "CKOPT fuse (operation dependent of CKSEL fuses"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "WDTON", "Watch-dog Timer always on"},
			{ 15, "RSTDISBL", "Reset Disabled (Enable PC6 as i/o pin)"}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 64 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0101 SUT=11", "Ext. RC Osc. - 0.9 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0110 SUT=01", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0110 SUT=11", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0111 SUT=00", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0111 SUT=01", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0111 SUT=10", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0111 SUT=11", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0001 SUT=00", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0001 SUT=01", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0001 SUT=10", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0011 SUT=00", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0100 SUT=00", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0100 SUT=10", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 64 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$0C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$0800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$0F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$0E00"}
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega8515,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "BODEN", "Brown-out detection enabled"},
			{ 7, "BODLEVEL", "Brown-out detection level at VCC=2.7 V"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "CKOPT", "CKOPT fuse (operation dependent of CKSEL fuses)"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "WDTON", "Watch-dog Timer always on"},
			{ 15, "S8515C", "AT90S4414/8515 compatibility mode"}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 64 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0101 SUT=11", "Ext. RC Osc. - 0.9 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0110 SUT=01", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0110 SUT=11", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0111 SUT=00", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0111 SUT=01", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0111 SUT=10", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0111 SUT=11", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0001 SUT=00", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0001 SUT=01", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0001 SUT=10", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0011 SUT=00", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0100 SUT=00", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0100 SUT=10", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 64 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$0C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$0800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$0F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$0E00"}
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega8535,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "BODEN", "Brown-out detection enabled"},
			{ 7, "BODLEVEL", "Brown-out detection level at VCC=2.7 V"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "CKOPT", "CKOPT fuse (operation dependent of CKSEL fuses)"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "WDTON", "Watch-dog Timer always on"},
			{ 15, "S8515C", "AT90S4414/8515 compatibility mode"}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 64 ms"},
			{ "CKSEL=1110 SUT=10", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 64 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0101 SUT=11", "Ext. RC Osc. - 0.9 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0110 SUT=01", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0110 SUT=11", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0111 SUT=00", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0111 SUT=01", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0111 SUT=10", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0111 SUT=11", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0001 SUT=00", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0001 SUT=01", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0001 SUT=10", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0011 SUT=00", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0100 SUT=00", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0100 SUT=10", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 64 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$0C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$0800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$0F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$0E00"}
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega64,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "BODEN", "Brown-out detection enabled"},
			{ 7, "BODLEVEL", "Not Brown-out detection level at VCC=2.7 V"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "CKOPT", "CKOPT fuse (operation dependent of CKSEL fuses)"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"},
			{ 16, "WDTON", "Watchdog Timer always on"},
			{ 17, "M103C", "ATmega103 Compatibility Mode"}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 64 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0101 SUT=11", "Ext. RC Osc. - 0.9 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0110 SUT=01", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0110 SUT=11", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0111 SUT=00", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0111 SUT=01", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0111 SUT=10", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0111 SUT=11", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0001 SUT=00", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0001 SUT=01", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0001 SUT=10", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0011 SUT=00", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0100 SUT=00", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0100 SUT=10", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 64 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$7C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$7800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$7F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$7E00"}
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega128,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "BODEN", "Brown-out detection enabled"},
			{ 7, "BODLEVEL", "Not Brown-out detection level at VCC=2.7 V"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "CKOPT", "CKOPT fuse (operation dependent of CKSEL fuses)"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"},
			{ 16, "WDTON", "Watchdog Timer always on"},
			{ 17, "M103C", "ATmega103 Compatibility Mode"}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 64 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0101 SUT=11", "Ext. RC Osc. - 0.9 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0110 SUT=01", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0110 SUT=11", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0111 SUT=00", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0111 SUT=01", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0111 SUT=10", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0111 SUT=11", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0001 SUT=00", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0001 SUT=01", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0001 SUT=10", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0011 SUT=00", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0100 SUT=00", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0100 SUT=10", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 64 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$FC00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$F800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$FF00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$FE00"}
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega16,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "BODEN", "Brown-out detection enabled"},
			{ 7, "BODLEVEL", "Not Brown-out detection level at VCC=2.7 V"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "CKOPT", "CKOPT fuse (operation dependent of CKSEL fuses)"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 64 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0101 SUT=11", "Ext. RC Osc. - 0.9 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0110 SUT=01", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0110 SUT=11", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0111 SUT=00", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0111 SUT=01", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0111 SUT=10", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0111 SUT=11", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0001 SUT=00", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0001 SUT=01", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0001 SUT=10", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0011 SUT=00", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0100 SUT=00", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0100 SUT=10", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 64 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$1C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$1800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$1F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$1E00"}
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega161,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "SUT", "Start-up time"},
			{ 4, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 5, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"}

		},
		{
			// fuse mask description
			{"CKSEL=000", "External Clock; fast rising power"},
			{"CKSEL=001", "External Clock; BOD Enabled or power-on reset"},
			{"CKSEL=010", "Crystal Oscillator; slowly rising power"},
			{"CKSEL=011", "Crystal Oscillator; fast rising power"},
			{"CKSEL=100", "Crystal Oscillator; BOD Enabled or power-on reset"},
			{"CKSEL=101", "Ceramic Resonator / External Clock; Slowly rising power"},
			{"CKSEL=110", "Ceramic Resonator; fast rising power"},
			{"CKSEL=111", "Ceramic Resonator; BOD Enabled or power-on reset"}

		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega163,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 5, "SPIEN", "Serial program downloading (SPI) enabled" },
			{ 6, "BODEN", "Brown-out detection enabled"},
			{ 7, "BODLEVEL", "Not Brown-out detection level at VCC=2.7 V"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""}
		},
		{
			// fuse mask description
			{"CKSEL=0000", "External Clock fast rising power"},
			{"CKSEL=0001", "External Clock BOD enabled"},
			{"CKSEL=0010", "Internal RC Ocsillator slowly rising power"},
			{"CKSEL=0011", "Internal RC Ocsillator fast rising power"},
			{"CKSEL=0100", "Internal RC Oscillator BOD enabled"},
			{"CKSEL=0101", "External RC Oscillator slowly rising power"},
			{"CKSEL=0110", "External RC Oscillator fast rising power"},
			{"CKSEL=0111", "External RC Oscillator BOD enabled"},
			{"CKSEL=1000", "External Low-Frequency Crystal"},
			{"CKSEL=1001", "External Low-Frequency Crystal"},
			{"CKSEL=1010", "Crystal Oscillator slowly rising power"},
			{"CKSEL=1011", "Crystal Oscillator fast rising power"},
			{"CKSEL=1100", "Crystal Oscillator BOD enabled"},
			{"CKSEL=1101", "Ceramic Resonator/External Clock slowly rising power"},
			{"CKSEL=1110", "Ceramic Resonator fast rising power"},
			{"CKSEL=1111", "Ceramic Resonator BOD enabled"},

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$1C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$1800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$1F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$1E00"}
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega162,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTB0"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watchdog timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"},
			{ 17, "BODLEVEL0", ""},
			{ 18, "BODLEVEL1", ""},
			{ 19, "BODLEVEL1", ""},
			{ 20, "M161C", "ATmega161 compability mode"}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 65 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=0100 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms; Int. Cap." },
			{ "CKSEL=0100 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=0110 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms; Int. Cap." },
			{ "CKSEL=0100 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms; Int. Cap." },
			{ "CKSEL=0101 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms" },
			{ "CKSEL=0111 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms; Int. Cap." },
			{ "CKSEL=0101 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms" },
			{ "CKSEL=0111 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms; Int. Cap." },
			{ "CKSEL=0101 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms" },
			{ "CKSEL=0111 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms; Int. Cap." },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc.; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc.; Start-up time: 6 CK + 65 ms" },

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$1C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$1800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$1F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$1E00"}
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega169,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "CKOUT", "Clock output on PORTE7"},
			{ 7, "CKDIV8", "Divide clock by 8 internally"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "WDTON", "Watchdog timer always on"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"},
			{ 16, "RSTDISBL", "Reset Disabled (Enable PG5 as i/o pin)"},
			{ 17, "BODLEVEL0", ""},
			{ 18, "BODLEVEL1", ""},
			{ 19, "BODLEVEL2", ""}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 65 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. Crystal Osc.; Frequency 0.4-0.9 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal Osc.; Frequency 0.9-3.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal Osc.; Frequency 3.0-8.0 MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 16K CK + 65 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 4.1 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal Osc.; Frequency 8.0- MHz; Start-up time: 258 CK + 65 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=0110 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4.1 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 65 ms" },
			{ "CKSEL=0111 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 0 ms" },
			{ "CKSEL=0111 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 4.1 ms" },
			{ "CKSEL=0111 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 65 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc.; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc.; Start-up time: 6 CK + 4.1 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc.; Start-up time: 6 CK + 65 ms" },

			{ "BODLEVEL=111", "Brown-out detection disabled" },
			{ "BODLEVEL=110", "Brown-out detection level at VCC=1.8 V" },
			{ "BODLEVEL=101", "Brown-out detection level at VCC=2.7 V" },
			{ "BODLEVEL=100", "Brown-out detection level at VCC=4.3 V" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$1C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$1800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$1F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$1E00"}
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega323,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 6, "BODEN", "Brown-out detection enabled"},
			{ 7, "BODLEVEL", "Not Brown-out detection level at VCC=2.7 V"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory during through Chip Erase Cycle"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"}
		},
		{
			// fuse mask description
			{"CKSEL=0000", "External Clock fast rising power"},
			{"CKSEL=0001", "External Clock BOD enabled"},
			{"CKSEL=0010", "Internal RC Ocsillator slowly rising power"},
			{"CKSEL=0011", "Internal RC Ocsillator fast rising power"},
			{"CKSEL=0100", "Internal RC Oscillator BOD enabled"},
			{"CKSEL=0101", "External RC Oscillator slowly rising power"},
			{"CKSEL=0110", "External RC Oscillator fast rising power"},
			{"CKSEL=0111", "External RC Oscillator BOD enabled"},
			{"CKSEL=1000", "External Low-Frequency Crystal"},
			{"CKSEL=1001", "External Low-Frequency Crystal"},
			{"CKSEL=1010", "Crystal Oscillator slowly rising power"},
			{"CKSEL=1011", "Crystal Oscillator fast rising power"},
			{"CKSEL=1100", "Crystal Oscillator BOD enabled"},
			{"CKSEL=1101", "Ceramic Resonator/External Clock slowly rising power"},
			{"CKSEL=1110", "Ceramic Resonator fast rising power"},
			{"CKSEL=1111", "Ceramic Resonator BOD enabled"},

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$3C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$3800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$3F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$3E00"}
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		ATmega32,
		{
			// fuse
			{ 0, "CKSEL0", ""},
			{ 1, "CKSEL1", ""},
			{ 2, "CKSEL2", ""},
			{ 3, "CKSEL3", ""},
			{ 4, "SUT0", ""},
			{ 5, "SUT1", ""},
			{ 6, "BODEN", "Brown-out detection enabled"},
			{ 7, "BODLEVEL", "Not Brown-out detection level at VCC=2.7 V"},
			{ 8, "BOOTRST", "Boot Reset vector Enabled (default address=$0000)"},
			{ 9, "BOOTSZ0", ""},
			{ 10, "BOOTSZ1", ""},
			{ 11, "EESAVE", "Preserve EEPROM memory through the Chip Erase cycle"},
			{ 12, "CKOPT", "CKOPT fuse (operation dependent of CKSEL fuses)"},
			{ 13, "SPIEN", "Serial program downloading (SPI) enabled"},
			{ 14, "JTAGEN", "JTAG Interface Enabled"},
			{ 15, "OCDEN", "On-Chip Debug Enabled"}
		},
		{
			// fuse mask description
			{ "CKSEL=0000 SUT=00", "Ext. Clock; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0000 SUT=01", "Ext. Clock; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0000 SUT=10", "Ext. Clock; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=1111 SUT=01", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1111 SUT=10", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1111 SUT=11", "Ext. Crystal/Resonator High Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1110 SUT=10", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1110 SUT=11", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1111 SUT=00", "Ext. Crystal/Resonator High Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1110 SUT=00", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1110 SUT=01", "Ext. Crystal/Resonator High Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1011 SUT=01", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1011 SUT=10", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1011 SUT=11", "Ext. Crystal/Resonator Low Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1010 SUT=10", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1010 SUT=11", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1011 SUT=00", "Ext. Crystal/Resonator Low Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1010 SUT=00", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1010 SUT=01", "Ext. Crystal/Resonator Low Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1101 SUT=01", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 0 ms" },
			{ "CKSEL=1101 SUT=10", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 4 ms" },
			{ "CKSEL=1101 SUT=11", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 16K CK + 64 ms" },
			{ "CKSEL=1100 SUT=10", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 0 ms" },
			{ "CKSEL=1100 SUT=11", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1101 SUT=00", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1100 SUT=00", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 4 ms" },
			{ "CKSEL=1100 SUT=01", "Ext. Crystal/Resonator Medium Freq.; Start-up time: 258 CK + 64 ms" },
			{ "CKSEL=1001 SUT=00", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 4 ms" },
			{ "CKSEL=1001 SUT=01", "Ext. Low-Freq. Crystal; Start-up time: 1K CK + 64 ms" },
			{ "CKSEL=1001 SUT=10", "Ext. Low-Freq. Crystal; Start-up time: 32K CK + 64 ms" },
			{ "CKSEL=0101 SUT=00", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0101 SUT=01", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0101 SUT=10", "Ext. RC Osc. - 0.9 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0101 SUT=11", "Ext. RC Osc. - 0.9 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0110 SUT=00", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0110 SUT=01", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0110 SUT=10", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0110 SUT=11", "Ext. RC Osc. 0.9 MHz - 3.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0111 SUT=00", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=0111 SUT=01", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=0111 SUT=10", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=0111 SUT=11", "Ext. RC Osc. 3.0 MHz - 8.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=1000 SUT=00", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 0 ms" },
			{ "CKSEL=1000 SUT=01", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 4 ms" },
			{ "CKSEL=1000 SUT=10", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 18 CK + 64 ms" },
			{ "CKSEL=1000 SUT=11", "Ext. RC Osc. 8.0 MHz - 12.0 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0001 SUT=00", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0001 SUT=01", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0001 SUT=10", "Int. RC Osc. 1 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0010 SUT=00", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0010 SUT=01", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0010 SUT=10", "Int. RC Osc. 2 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0011 SUT=00", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0011 SUT=01", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0011 SUT=10", "Int. RC Osc. 4 MHz; Start-up time: 6 CK + 64 ms" },
			{ "CKSEL=0100 SUT=00", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 0 ms" },
			{ "CKSEL=0100 SUT=01", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 4 ms" },
			{ "CKSEL=0100 SUT=10", "Int. RC Osc. 8 MHz; Start-up time: 6 CK + 64 ms" },

			{ "BOOTSZ=01", "Boot Flash section size=1024 words Boot start address=$3C00" },
			{ "BOOTSZ=00", "Boot Flash section size=2048 words Boot start address=$3800" },
			{ "BOOTSZ=11", "Boot Flash section size=256 words Boot start address=$3F00"},
			{ "BOOTSZ=10", "Boot Flash section size=512 words Boot start address=$3E00"}
		},
		{
			// lock
			{ 0, "Lock1", ""},
			{ 1, "Lock2", ""},
			{ 2, "BootLock1", ""},
			{ 3, "BootLock2", ""},
			{ 4, "BootLoad1", ""},
			{ 5, "BootLoad2", ""}
		},
		{
			// lock mask description
			{ "Lock=11", "Mode 1: No memory lock features enabled" },
			{ "Lock=10", "Mode 2: Further programming disabled" },
			{ "Lock=00", "Mode 3: Further programming and verification disabled" },

			{ "BootLock=11", "Application Protection Mode 1: No lock on SPM and LPM in Application Section" },
			{ "BootLock=10", "Application Protection Mode 2: SPM prohibited in Application Section" },
			{ "BootLock=00", "Application Protection Mode 3: LPM and SPM prohibited in Application Section" },
			{ "BootLock=01", "Application Protection Mode 4: LPM prohibited in Application Section" },

			{ "BootLoad=11", "Boot Loader Protection Mode 1: No lock on SPM and LPM in Boot Loader Section" },
			{ "BootLoad=10", "Boot Loader Protection Mode 2: SPM prohibited in Boot Loader Section" },
			{ "BootLoad=00", "Boot Loader Protection Mode 3: LPM and SPM prohibited in Boot Loader Section" },
			{ "BootLoad=01", "Boot Loader Protection Mode 4: LPM prohibited in Boot Loader Section" }
		}
	},

	{
		PIC12508,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "CP", ""},
			{ 4, "MCLRE", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC12509,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "CP", ""},
			{ 4, "MCLRE", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC12508A,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "CP", ""},
			{ 4, "MCLRE", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC12509A,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "CP", ""},
			{ 4, "MCLRE", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC12E518,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "CP", ""},
			{ 4, "MCLRE", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC12E519,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "CP", ""},
			{ 4, "MCLRE", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC12671,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "FOSC2", ""},
			{ 3, "WDTE", ""},
			{ 4, "PWRTE", ""},
			{ 5, "CP0", ""},
			{ 6, "CP1", ""},
			{ 7, "MCLRE", ""},
			{ 8, "CP0", ""},
			{ 9, "CP1", ""},
			{ 10, "CP0", ""},
			{ 11, "CP1", ""},
			{ 12, "CP0", ""},
			{ 13, "CP1", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC12672,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "FOSC2", ""},
			{ 3, "WDTE", ""},
			{ 4, "PWRTE", ""},
			{ 5, "CP0", ""},
			{ 6, "CP1", ""},
			{ 7, "MCLRE", ""},
			{ 8, "CP0", ""},
			{ 9, "CP1", ""},
			{ 10, "CP0", ""},
			{ 11, "CP1", ""},
			{ 12, "CP0", ""},
			{ 13, "CP1", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC12673,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "FOSC2", ""},
			{ 3, "WDTE", ""},
			{ 4, "PWRTE", ""},
			{ 5, "CP0", ""},
			{ 6, "CP1", ""},
			{ 7, "MCLRE", ""},
			{ 8, "CP0", ""},
			{ 9, "CP1", ""},
			{ 10, "CP0", ""},
			{ 11, "CP1", ""},
			{ 12, "CP0", ""},
			{ 13, "CP1", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC12674,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "FOSC2", ""},
			{ 3, "WDTE", ""},
			{ 4, "PWRTE", ""},
			{ 5, "CP0", ""},
			{ 6, "CP1", ""},
			{ 7, "MCLRE", ""},
			{ 8, "CP0", ""},
			{ 9, "CP1", ""},
			{ 10, "CP0", ""},
			{ 11, "CP1", ""},
			{ 12, "CP0", ""},
			{ 13, "CP1", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC1683,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "PWRTE", ""},
			{ 4, "CP", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC1684,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "PWRTE", ""},
			{ 4, "CP", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC1684A,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "PWRTE", ""},
			{ 4, "CP", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC16870,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "PWRTE", ""},
			{ 4, "CP0", ""},
			{ 5, "CP1", ""},
			{ 6, "BODEN", ""},
			{ 7, "LVP", ""},
			{ 8, "CPD", ""},
			{ 9, "WRT", ""},
			{ 11, "RESV", ""},
			{ 12, "CP0", ""},
			{ 13, "CP1", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC16871,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "PWRTE", ""},
			{ 4, "CP0", ""},
			{ 5, "CP1", ""},
			{ 6, "BODEN", ""},
			{ 7, "LVP", ""},
			{ 8, "CPD", ""},
			{ 9, "WRT", ""},
			{ 11, "RESV", ""},
			{ 12, "CP0", ""},
			{ 13, "CP1", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC16872,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "PWRTE", ""},
			{ 4, "CP0", ""},
			{ 5, "CP1", ""},
			{ 6, "BODEN", ""},
			{ 7, "LVP", ""},
			{ 8, "CPD", ""},
			{ 9, "WRT", ""},
			{ 11, "RESV", ""},
			{ 12, "CP0", ""},
			{ 13, "CP1", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC16873,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "PWRTE", ""},
			{ 4, "CP0", ""},
			{ 5, "CP1", ""},
			{ 6, "BODEN", ""},
			{ 7, "LVP", ""},
			{ 8, "CPD", ""},
			{ 9, "WRT", ""},
			{ 11, "RESV", ""},
			{ 12, "CP0", ""},
			{ 13, "CP1", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC16874,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "PWRTE", ""},
			{ 4, "CP0", ""},
			{ 5, "CP1", ""},
			{ 6, "BODEN", ""},
			{ 7, "LVP", ""},
			{ 8, "CPD", ""},
			{ 9, "WRT", ""},
			{ 11, "RESV", ""},
			{ 12, "CP0", ""},
			{ 13, "CP1", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC16876,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "PWRTE", ""},
			{ 4, "CP0", ""},
			{ 5, "CP1", ""},
			{ 6, "BODEN", ""},
			{ 7, "LVP", ""},
			{ 8, "CPD", ""},
			{ 9, "WRT", ""},
			{ 11, "RESV", ""},
			{ 12, "CP0", ""},
			{ 13, "CP1", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC16877,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTE", ""},
			{ 3, "PWRTE", ""},
			{ 4, "CP0", ""},
			{ 5, "CP1", ""},
			{ 6, "BODEN", ""},
			{ 7, "LVP", ""},
			{ 8, "CPD", ""},
			{ 9, "WRT", ""},
			{ 11, "RESV", ""},
			{ 12, "CP0", ""},
			{ 13, "CP1", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC16873A,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTEN", ""},
			{ 3, "/PWRTEN", ""},
			{ 6, "BOREN", ""},
			{ 7, "LVP", ""},
			{ 8, "CPD", ""},
			{ 9, "WRT0", ""},
			{ 11, "WRT1", ""},
			{ 12, "DEBUG", ""},
			{ 14, "CP", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC16874A,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTEN", ""},
			{ 3, "/PWRTEN", ""},
			{ 6, "BOREN", ""},
			{ 7, "LVP", ""},
			{ 8, "CPD", ""},
			{ 9, "WRT0", ""},
			{ 11, "WRT1", ""},
			{ 12, "DEBUG", ""},
			{ 14, "CP", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC16876A,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTEN", ""},
			{ 3, "/PWRTEN", ""},
			{ 6, "BOREN", ""},
			{ 7, "LVP", ""},
			{ 8, "CPD", ""},
			{ 9, "WRT0", ""},
			{ 11, "WRT1", ""},
			{ 12, "DEBUG", ""},
			{ 14, "CP", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC16877A,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTEN", ""},
			{ 3, "/PWRTEN", ""},
			{ 6, "BOREN", ""},
			{ 7, "LVP", ""},
			{ 8, "CPD", ""},
			{ 9, "WRT0", ""},
			{ 11, "WRT1", ""},
			{ 12, "DEBUG", ""},
			{ 14, "CP", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC16627,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTEN", ""},
			{ 3, "/PWRTEN", ""},
			{ 4, "FOSC2", ""},
			{ 5, "MCLRE", ""},
			{ 6, "BOREN", ""},
			{ 7, "LVP", ""},
			{ 8, "CPD", ""},
			{ 10, "CP0", ""},
			{ 11, "CP1", ""},
			{ 12, "CP0", ""},
			{ 13, "CP1", ""}
		},
		{
			// lock mask description
		}
	},

	{
		PIC16628,
		{
			// fuse
		},
		{
			// fuse mask description
		},
		{
			// lock
			{ 0, "FOSC0", ""},
			{ 1, "FOSC1", ""},
			{ 2, "WDTEN", ""},
			{ 3, "/PWRTEN", ""},
			{ 4, "FOSC2", ""},
			{ 5, "MCLRE", ""},
			{ 6, "BOREN", ""},
			{ 7, "LVP", ""},
			{ 8, "CPD", ""},
			{ 10, "CP0", ""},
			{ 11, "CP1", ""},
			{ 12, "CP0", ""},
			{ 13, "CP1", ""}
		},
		{
			// lock mask description
		}
	}
};

