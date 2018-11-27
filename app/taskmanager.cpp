#include <QJsonArray>
#include <QJsonObject>
#include <QtDebug>
#include <QString>
#include <unistd.h>
#include <iostream>
#include <QtCore>
#include "taskmanager.h"

TaskManager::TaskManager(QObject* parent) : QObject(parent) {
	qDebug() << "Inside contructor";
	connect(&m_socket, SIGNAL(textMessageReceived(QString)), this, SLOT(onSocketTextReceived(QString)));
}

void TaskManager::open(const QUrl &bindingAddress) {
	m_socket.open(bindingAddress);
	timer = new QTimer();
	connect(timer, SIGNAL(timeout()), this, SLOT(query()));
	timer->start(3000);
}

void TaskManager::kill(int tid) {
	qDebug() << "killing " << tid;
	callService(QString("kill_process"), QJsonValue(tid));
}

void TaskManager::query() {
	qDebug() << "querying";
	callService(QString("get_process_list"), QJsonValue());
}

void TaskManager::callService(const QString& command, QJsonValue value) {
	qDebug() << "Entered callService";

	QJsonArray msg;
	msg.append(2); // Call
	msg.append(QString::number(m_nextCallId));
	msg.append(QString("taskmanager/") + command);
	msg.append(value);
	m_nextCallId++;

	QJsonDocument jsonDoc;
	jsonDoc.setArray(msg);

	m_socket.sendTextMessage(jsonDoc.toJson(QJsonDocument::Compact));
}

void TaskManager::onSocketTextReceived(QString msg)
{

	QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8());
	QJsonArray arr = doc.array();

	switch(arr[0].toInt()) {
	case 3: // RetOK
	case 4: // RetErr
		ProcessResponse(arr[0].toInt() == 3, arr[2]);
		break;
	}
}

void TaskManager::ProcessResponse(bool r, const QJsonValue& val)
{
	std::vector<ProcInfo> procs;
		
	if (r) {
		QJsonObject ret_val = val.toObject();
		QJsonObject response = ret_val["response"].toObject();
		QJsonArray processes = response["processes"].toArray();

		for(auto it = processes.constBegin(); it != processes.constEnd(); ++it)
		{
			ProcInfo Proc(it->toObject());
			procs.push_back(Proc);
		}
	}

	int flag;
	if(m_procinfos.empty()){
		for(auto it = procs.begin(); it != procs.end(); ++it){ // if m_procinfos is empty then this is first call
			emit addProcess(it->cmd(), it->tid(), it->euid(), it->scpu(), it->ucpu(), it->resident_memory(), it->state()); 
		}			
	} else {
		for(auto it = procs.begin(); it != procs.end(); ++it){						// loop through procs, it = procs element (ProcInfo obj)
			flag = 0;
			for(auto it2 = m_procinfos.begin(); it2 != m_procinfos.end(); ++it2){	// loop through m_procinfos, it2 m_procinfos element (ProcInfo obj)
				// if(*it == *it2){  // if the same ID exists in both vectors
				if(it->tid() == it2->tid()){
					// qDebug() << "Compares TIDs" << it->tid() << "and" << it2->tid();
					if(it->cmd() == it2->cmd()){ // if the name is still the same
						// qDebug() << "Compares CMDs" << it->cmd() << "and" << it2->cmd();
						if(!(it == it2)){ // if the same process has new data
							// qDebug () << "Compares if two obj are the same";
							emit updateProcess(it->cmd(), it->tid(), it->euid(), it->scpu(), it->ucpu(), it->resident_memory(), it->state());
							// qDebug() << "processes was updated in QML";
							m_procinfos.erase(it2);
							// qDebug() << "process was removed from m_procinfos";
							flag = 1;
							// qDebug() << "obj comparison is finished";
							break;
						}
					} else { // process in m_procinfos has died and a new one has its ID
						qDebug() << "The process ID has been reused for another process";
						emit updateProcess(it->cmd(), it->tid(), it->euid(), it->scpu(), it->ucpu(), it->resident_memory(), it->state());
						m_procinfos.erase(it2);
						flag = 1;
					}
				}
			}
			if(flag == 0){ // if no ID was found in old vector; that means it's a new process
				qDebug() << it->cmd() << " process has been added";
				emit addProcess(it->cmd(), it->tid(), it->euid(), it->scpu(), it->ucpu(), it->resident_memory(), it->state()); 
			}
		}
		for(auto it2 = m_procinfos.begin(); it2 != m_procinfos.end(); ++it2){ // remaining processes in m_procinfos are all dead
			qDebug() << "Dead processes are removed";
			qDebug() << it2->cmd();
			emit removeProcess(it2->tid());
		}
	}
	m_procinfos = procs;
}
