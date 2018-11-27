import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import TaskManager 1.0

Window {
    id: root
    visible: true
    width: 745
    height: 480

    TaskManager {
        id: taskmgr

        onUpdateProcess: {
            var index = findId(tid_);
            libraryModel.set(index, {"cmd": cmd_, "tid": tid_, "user": euid_, "system_cpu": scpu_,
                                     "user_cpu": ucpu_, "resident_memory": resident_memory_, "state": state_});
        }

        onAddProcess: {
            libraryModel.append({"cmd": cmd_, "tid": tid_, "user": euid_, "system_cpu": scpu_,
                                 "user_cpu": ucpu_, "resident_memory": resident_memory_, "state": state_});
        }

        onRemoveProcess: {
            var index = findId(tid_);
            libraryModel.remove(index);
        }

        function findId(tid) {
            for(var i = 0; i < libraryModel.count; i++) {
                if(tid == libraryModel.get(i).tid) {
                      return i;
                  }
            }
        }

        Component.onCompleted: {
            taskmgr.open(bindingAddress);
        }
    }

    ListModel {
        id: libraryModel
    }

    Rectangle {
        id: mainview
        width: parent.width
        height: parent.height

        Row {
            id: buttonRow
            width: parent.width
            anchors.top: mainview.top

            RowLayout {
                id: buttonRowLayout
                spacing: 6

                Button {
                    id: infoButton
                    text: "Info"
                    enabled: tableView.currentRow != -1
                }

                Button {
                    id: killButton
                    text: "Kill"
                    enabled: tableView.currentRow != -1
                    onClicked: {
                        tableView.selection.forEach( function(rowIndex) {
                            taskmgr.kill(libraryModel.get(rowIndex).tid)}
                        )
                        tableView.selection.clear()
                    }
                }
            }
        }

        TableView {
            id: tableView
            height: parent.height - buttonRow.height
            width: parent.width
            anchors.top: buttonRow.bottom

            TableViewColumn {
                role: "cmd"
                title: "Process"
                width: 150
            }
            TableViewColumn {
                role: "tid"
                title: "ID"
                width: 80
            }
            TableViewColumn {
                role: "user"
                title: "User"
                width: 80
            }
            TableViewColumn {
                role: "system_cpu"
                title: "System %"
                width: 100
            }
            TableViewColumn {
                role: "user_cpu"
                title: "User %"
                width: 100
            }
            TableViewColumn {
                role: "resident_memory"
                title: "Memory"
                width: 100
            }
            TableViewColumn {
                role: "state"
                title: "State"
                width: 90
            }
            model: libraryModel
        }
    }
}
