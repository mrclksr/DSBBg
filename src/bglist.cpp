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

#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>

#include "bglist.h"
#include "dsbcfg.h"

bool BgList::error()
{
	return (_error);
}

QString BgList::errmsg() const
{
	return (_errmsg);
}

void BgList::clearerr()
{
	_error = false;
}

QString BgList::getPath()
{
	char *dir;

	if (_error)
		return (QString());
	if ((dir = dsbcfg_mkdir(NULL)) == NULL) {
		_error = true;
		_errmsg = dsbcfg_strerror();
		return (QString());
	}
	return (QString("%1/%2").arg(dir).arg(PATH_BGLIST));
}

QStringList BgList::read()
{
	if (_error)
		return (QStringList());
	QString	    path = getPath();
	QStringList list;
	if (path.isEmpty())
		return (QStringList());
	QFile file(path);
	if (!file.exists())
		return (QStringList());
	if (!file.open(QIODevice::ReadOnly)) {
		_error = true;
		_errmsg = QString("Failed to open %1: %2")
				 .arg(path)
				 .arg(file.errorString());
		return (QStringList());
	}
	QTextStream in(&file);
	while (!in.atEnd()) {
		QString s = in.readLine();
		if (s.isEmpty())
			continue;
		list += s;
	}
	file.close();

	return (list);
}

int BgList::write(QStringList &sl)
{
	if (_error)
		return (-1);
	QString path = getPath();
	
	if (path.isEmpty())
		return (-1);
	QTemporaryFile file(path);
	if (!file.open()) {
		_error = true;
		_errmsg = QString("Failed to create %1: %2")
				 .arg(file.fileName())
				 .arg(file.errorString());
		return (-1);
	}
	QTextStream out(&file);
	for (QString s : sl)
		out << s << endl;
	file.close();
	if (QFile::exists(path) && !QFile::remove(path)) {
		_error = true;
		_errmsg = QString("Failed to delete %1: %2")
				 .arg(path)
				 .arg(file.errorString());
		return (-1);
	}	
	if (!QFile::copy(file.fileName(), path)) {
		_error = true;
		_errmsg = QString("Failed to copy %1 to %2: %3")
				 .arg(file.fileName())
				 .arg(path)
				 .arg(file.errorString());
		return (-1);
	}
	return (0);
}
