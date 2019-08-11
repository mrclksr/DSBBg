/*-
 * Copyright (c) 2019 Marcel Kaiser. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <QLabel>
#include <QItemSelectionModel>
#include <QDebug>
#include <QFileDialog>
#include <QByteArray>
#include <QProcess>
#include <QMessageBox>
#include <QIcon>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>

#include "mainwin.h"

MainWin::MainWin(QWidget *parent) : QMainWindow(parent)
{
	QIcon	    addIcon    = loadIcon("list-add", 0);
	QIcon	    rmIcon     = loadIcon("edit-delete", 0);
	QIcon	    quitIcon   = loadIcon("application-exit", 0);
	QIcon	    undoIcon   = loadIcon("edit-undo", 0);
	QIcon	    redoIcon   = loadIcon("edit-redo", 0);
	QIcon	    winIcon    = loadIcon("preferences-desktop-wallpaper", 0);
	QIcon	    setIcon    = QApplication::style()->standardIcon(
				     QStyle::SP_DialogApplyButton, 0, this);
	QLabel	    *label     = new QLabel(tr("Mode"));
	QWidget	    *container = new QWidget(this);
	QVBoxLayout *layout    = new QVBoxLayout;
	QHBoxLayout *hbox1     = new QHBoxLayout;
	QHBoxLayout *hbox2     = new QHBoxLayout;
	QPushButton *addPB     = new QPushButton(addIcon, tr("&Add"));
	QPushButton *quitPB    = new QPushButton(quitIcon, tr("&Quit"));
	removePB	       = new QPushButton(rmIcon, tr("&Remove"));
	setPB		       = new QPushButton(setIcon, tr("&Set wallpaper"));
	undoPB		       = new QPushButton(undoIcon, tr("&Undo"));
	redoPB		       = new QPushButton(redoIcon, tr("&Redo"));
	modeCB		       = new QComboBox;
	model		       = new Model(this);
	list		       = new List(this);
	QItemSelectionModel *selections = new QItemSelectionModel(model);

	layout->setContentsMargins(15, 15, 15, 15);

	if (model->init() == -1)
		errWin(model->errmsg());
	list->setModel(model);
	list->setSelectionModel(selections);
	list->setViewMode(QListView::IconMode);
	list->setResizeMode(QListView::Adjust);
	list->setToolTip(tr("Drop images here"));
	list->setUniformItemSizes(true);
	
	hbox1->addStretch(1);
	hbox1->addWidget(addPB,    0, Qt::AlignCenter);
	hbox1->addWidget(removePB, 0, Qt::AlignCenter);
	hbox1->addWidget(undoPB,   0, Qt::AlignCenter);
	hbox1->addWidget(redoPB,   0, Qt::AlignCenter);
	hbox1->addStretch(1);

	modeCB->addItem(tr("Fill"),   QVariant(QString("--bg-fill")));
	modeCB->addItem(tr("Tile"),   QVariant(QString("--bg-tile")));
	modeCB->addItem(tr("Scale"),  QVariant(QString("--bg-scale")));
	modeCB->addItem(tr("Center"), QVariant(QString("--bg-center")));

	hbox2->addWidget(label,  0, Qt::AlignLeft);
	hbox2->addWidget(modeCB, 0, Qt::AlignLeft);
	hbox2->addWidget(setPB,  0, Qt::AlignLeft);
	hbox2->addStretch(1);

	layout->addWidget(list);
	layout->addLayout(hbox1);
	layout->addWidget(mkLine());
	layout->addLayout(hbox2);
	layout->addWidget(mkLine());
	layout->addWidget(quitPB, 0, Qt::AlignRight);
	
	container->setLayout(layout);
	setCentralWidget(container);

	setMinimumSize(700, 500);
	setWindowIcon(winIcon);
	setWindowTitle(tr("Set background image"));
	
	setPB->setEnabled(false);
	removePB->setEnabled(false);
	undoPB->setEnabled(false);
	redoPB->setEnabled(false);

	connect(list->selectionModel(),
	    SIGNAL(selectionChanged(const QItemSelection &,
	        const QItemSelection &)), this,
	    SLOT(catchSelectionChanged(const QItemSelection &,
	        const QItemSelection &)));
	connect(list, SIGNAL(itemDroped(QStringList &)),
	    this, SLOT(catchItemDroped(QStringList &)));
	connect(list, SIGNAL(deleteKeyPressed()), this,
	    SLOT(removeWallpaper()));
	connect(removePB, SIGNAL(clicked()), this, SLOT(removeWallpaper()));
	connect(addPB,    SIGNAL(clicked()), this, SLOT(addWallpaper()));
	connect(setPB,    SIGNAL(clicked()), this, SLOT(setWallpaper()));
	connect(quitPB,   SIGNAL(clicked()), this, SLOT(quit()));
	connect(undoPB,   SIGNAL(clicked()), this, SLOT(undo()));
	connect(redoPB,   SIGNAL(clicked()), this, SLOT(redo()));
}

QFrame *MainWin::mkLine()
{
	QFrame *line = new QFrame(this);
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	return (line);
}

QIcon MainWin::loadIcon(const char *name, ...)
{
	va_list    ap;
	const char *s;

	va_start(ap, name);
	for (s = name; s != NULL || (s = va_arg(ap, char *)); s = NULL) {
		if (!QIcon::hasThemeIcon(s))
			continue;
		QIcon icon = QIcon::fromTheme(s);
		if (icon.isNull())
			continue;
		if (icon.name().isEmpty() || icon.name().length() < 1)
			continue;
		return (icon);
	}
	return (QIcon());
}

void MainWin::errWin(QString message)
{
	QMessageBox msgBox;
	msgBox.setWindowModality(Qt::WindowModal);
	msgBox.setText(tr("An error occured"));
	msgBox.setWindowTitle(tr("Error"));
	msgBox.setIcon(QMessageBox::Critical);
	msgBox.setWindowIcon(msgBox.iconPixmap());
	msgBox.setInformativeText(message);
	msgBox.addButton(tr("Ok"), QMessageBox::ActionRole);
	msgBox.exec();
}

QString MainWin::getHomeDir()
{
	struct passwd *pw;

	if ((pw = getpwuid(getuid())) == NULL) {
		QString errstr(strerror(errno));
		errstr.prepend("getpwuid(): ");
		errWin(errstr);
		exit(1);
	}
	endpwent();
	return (QString(pw->pw_dir));
}

void MainWin::catchSelectionChanged(const QItemSelection &selected,
	const QItemSelection &deselected)
{
	Q_UNUSED(selected);
	Q_UNUSED(deselected);
	
	if (model->rowCount() > 0) {
		removePB->setEnabled(true);
		setPB->setEnabled(true);
	}
}

void MainWin::catchItemDroped(QStringList &files)
{
	for (QString file : files)
		model->addWallpaper(file);
	undoPB->setEnabled(true);
	redoPB->setEnabled(model->canRedo());
}

void MainWin::removeWallpaper()
{
	QModelIndex idx = list->currentIndex();
	if (!idx.isValid())
		return;
	model->removeWallpaper(idx.row());
	undoPB->setEnabled(true);
	redoPB->setEnabled(model->canRedo());
}

void MainWin::addWallpaper()
{
	QString filename = QFileDialog::getOpenFileName(this,
		tr("Open Image"), getHomeDir(),
		tr("Image Files (*.png *.jpg *.bmp)"));
	if (filename.isEmpty())
		return;
	model->addWallpaper(filename);
	undoPB->setEnabled(true);
	redoPB->setEnabled(model->canRedo());
}

void MainWin::setWallpaper()
{
	QString errbuf;
	QString filename = model->getFilename(list->currentIndex().row());
	QString mode = modeCB->itemData(modeCB->currentIndex()).toString();
	QString cmd = QString("%1 %2 %3").arg(PATH_FEH).arg(mode).arg(filename);
	QProcess proc;
	QByteArray line;

	if (filename.isEmpty())
		return;
	proc.setReadChannel(QProcess::StandardError);
	proc.start(cmd);
	(void)proc.waitForStarted(-1);
	if (proc.state() == QProcess::NotRunning) {
		errWin(tr("Failed to execute '%1': %2")
		      .arg(cmd)
		      .arg(proc.errorString()));
	}
	while (proc.waitForReadyRead(-1)) {
		while (!(line = proc.readLine()).isEmpty())
			errbuf += line;
	}
	proc.waitForFinished(-1);
	if (proc.exitCode() != 0) {
		errWin(tr("Command '%1' returned with an error:\n\n%2")
		      .arg(cmd)
		      .arg(errbuf));
	}
}

void MainWin::undo()
{
	model->undo();
	undoPB->setEnabled(model->canUndo());
	redoPB->setEnabled(model->canRedo());
}

void MainWin::redo()
{
	model->redo();
	undoPB->setEnabled(model->canUndo());
	redoPB->setEnabled(model->canRedo());
}

void MainWin::closeEvent(QCloseEvent *event)
{
	MainWin::quit();
	event->ignore();
}

void MainWin::quit()
{
	if (model->save() == -1)
		errWin(model->errmsg());
	QApplication::quit();
}
