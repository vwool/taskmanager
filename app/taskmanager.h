#include <QObject>
#include <QString>
#include <QSharedPointer>
#include <QStringList>
#include <QVector>
#include <QtCore>
#include <QWebSocket>
#include "procinfo.h"

#ifndef TASKMANAGER_H
#define TASKMANAGER_H

class TaskManager : public QObject
{
	Q_OBJECT

public:
    explicit TaskManager(QObject* parent = nullptr);

    Q_INVOKABLE void open(const QUrl& url);
    Q_INVOKABLE void kill(int tid);
    QTimer *timer;

signals:
	void updateProcess(const QString& cmd_, int tid_, int euid_, double scpu_, double ucpu_, double resident_memory_, const QString& state_);
	void addProcess(const QString& cmd_, int tid_, int euid_, double scpu_, double ucpu_, double resident_memory_, const QString& state_);
	void removeProcess(int tid_);

private slots:
	void query();
	void callService(const QString& ccommand, QJsonValue value);
	void onSocketTextReceived(QString msg);

private:
	QWebSocket m_socket;
	int m_nextCallId;
	std::vector<ProcInfo> m_procinfos;

	void ProcessResponse(bool r, const QJsonValue &val);
};

#endif // TASKMANAGER_H
