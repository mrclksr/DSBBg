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

#include "model.h"

Model::Model(QObject *parent) : QAbstractListModel(parent)
{

}

int Model::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return (rc);
}

int Model::rowCount() const
{
	return (rc);
}

QVariant Model::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	if (role == Qt::DisplayRole)
		return (QVariant());
	if (role == Qt::DecorationRole)
		return (wallpapers.at(index.row()).pix);
	return QVariant();
}

QVariant Model::headerData(int section, Qt::Orientation orientation,
	int role) const
{
	Q_UNUSED(section);
	Q_UNUSED(orientation);
	Q_UNUSED(role);
	return QVariant();
}

QString Model::_removeWallpaper(int row)
{
	QModelIndex idx = this->index(row);
	if (!idx.isValid())
		return (QString());
	QString filename = wallpapers.at(row).filename;
	beginRemoveRows(idx, row, row);
	removeRow(row);
	rc--;
	endRemoveRows();
	wallpapers.removeAt(row);

	return (filename);
}

void Model::removeWallpaper(int row)
{
	QString filename = _removeWallpaper(row);
	if (filename.isEmpty())
		return;
	Action action;
	action.row	= row;
	action.filename	= filename;
	action.action	= ACTION_REMOVE;
	if (undoQueue.count() - 1 > undoIdx) {
		while (undoQueue.count() - 1 != undoIdx)
			undoQueue.removeLast();
	}
	undoQueue.insert(++undoIdx, action);
}

int Model::_addWallpaper(QString filename, int row)
{
	if (row < 0 || row > rc)
		row = rc;
	QModelIndex idx = this->index(row);
	Wallpaper   wp;
	wp.filename = filename;
	wp.pix	    = QPixmap(filename)
			.scaled(QSize(210, 140), Qt::IgnoreAspectRatio);
	if (!wp.pix)
		return (-1);
	beginInsertRows(idx, row, row);
	insertRows(row, 1);
	wallpapers.insert(row, wp);
	rc++;
	endInsertRows();

	return (row);
}

void Model::addWallpaper(QString filename)
{
	int row;
	
	if ((row = _addWallpaper(filename)) == -1)
		return;
	Action action;
	action.row	= row;
	action.filename	= filename;
	action.action	= ACTION_ADD;
	if (undoQueue.count() - 1 > undoIdx) {
		while (undoQueue.count() - 1 != undoIdx)
			undoQueue.removeLast();
	}
	undoQueue.insert(++undoIdx, action);
}

int Model::init()
{
	QStringList list = bglist.read();

	if (list.isEmpty() && bglist.error()) {
		_errmsg = bglist.errmsg(); bglist.clearerr();
		return (-1);
	}
	for (QString s : list)
		_addWallpaper(s);
	return (0);
}

int Model::save()
{
	QStringList list;
	
	for (int i = 0; i < wallpapers.count(); i++)
		list += wallpapers.at(i).filename;
	if (bglist.write(list) == -1) {
		_errmsg = bglist.errmsg(); bglist.clearerr();
		return (-1);
	}
	return (0);
}

QString Model::getFilename(int row)
{
	QModelIndex idx = this->index(row);
	if (!idx.isValid())
		return QString();
	return (wallpapers.at(row).filename);
}

QString Model::errmsg() const
{
	return (_errmsg);
}

bool Model::canUndo()
{
	return (undoIdx < 0 ? false : true);
}

bool Model::canRedo()
{
	if (undoIdx >= -1 && undoIdx < undoQueue.count() - 1)
		return (true);
	return (false);
}

void Model::undo()
{
	if (!canUndo())
		return;
	switch (undoQueue.at(undoIdx).action) {
	case ACTION_ADD:
		_removeWallpaper(undoQueue.at(undoIdx).row);
		break;
	case ACTION_REMOVE:
		_addWallpaper(undoQueue.at(undoIdx).filename,
		    undoQueue.at(undoIdx).row);
		break;
	}
	undoIdx--;
}

void Model::redo()
{
	if (!canRedo())
		return;
	undoIdx++;

	switch (undoQueue.at(undoIdx).action) {
	case ACTION_REMOVE:
		_removeWallpaper(undoQueue.at(undoIdx).row);
		break;
	case ACTION_ADD:
		_addWallpaper(undoQueue.at(undoIdx).filename,
		    undoQueue.at(undoIdx).row);
		break;
	}
}
