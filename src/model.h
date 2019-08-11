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

#pragma once
#include <QVariant>
#include <QList>
#include <QPixmap>
#include <QAbstractListModel>

#include "bglist.h"

enum { ACTION_ADD, ACTION_REMOVE };

class Model : public QAbstractListModel
{
	Q_OBJECT
public:
	Model(QObject *parent = nullptr);

	int	 rowCount(const QModelIndex &parent) const override;
	int	 rowCount(void) const;
	int	 init(void);
	int	 save(void);
	void	 addWallpaper(QString filename);
	void	 removeWallpaper(int row);
	void	 undo(void);
	void	 redo(void);
	bool	 canUndo(void);
	bool	 canRedo(void);
	QString	 errmsg(void) const;
	QString	 getFilename(int row);
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
private:
	int	_addWallpaper(QString filename, int row = -1);
	QString	_removeWallpaper(int row);
private:
	struct Wallpaper {
		QString filename;
		QPixmap pix;
	};
	struct Action {
		int	row;
		int	action;
		QString filename;
	};
	int		 rc = 0;
	int		 undoIdx = -1;
	BgList		 bglist;
	QString		 _errmsg;
	QList<Action>	 undoQueue;
	QList<Wallpaper> wallpapers;
};
