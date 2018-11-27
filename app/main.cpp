#include <QtCore/QDebug>
#include <QtCore/QCommandLineParser>
#include <QtCore/QUrlQuery>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QThread>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQml/qqml.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickview.h>
#include <QQuickWindow>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <libhomescreen.hpp>
#include <qlibwindowmanager.h>
#include <taskmanager.h>

#include <unistd.h>

int main(int argc, char *argv[])
{
	QString graphic_role = QString("utility");
	QString myname = QString("TaskManager");
	
	QGuiApplication app(argc, argv);

	QCommandLineParser parser;
	parser.addPositionalArgument("port", app.translate("main", "port for binding"));
	parser.addPositionalArgument("secret", app.translate("main", "secret for binding"));
	parser.addHelpOption();
	parser.addVersionOption();
	parser.process(app);
	QStringList positionalArguments = parser.positionalArguments();

	qmlRegisterType<TaskManager>("TaskManager", 1, 0, "TaskManager");

	QQmlApplicationEngine engine;
	if (positionalArguments.length() != 2) {
		qDebug() << "[ERROR] No port and token specified!";
		return -1;
	}

	int port = positionalArguments.takeFirst().toInt();
	QString secret = positionalArguments.takeFirst();
	QUrl bindingAddress;
	bindingAddress.setScheme(QStringLiteral("ws"));
	bindingAddress.setHost(QStringLiteral("localhost"));
	bindingAddress.setPort(port);
	bindingAddress.setPath(QStringLiteral("/api"));
	QUrlQuery query;
	query.addQueryItem(QStringLiteral("token"), secret);
	bindingAddress.setQuery(query);
	QQmlContext *context = engine.rootContext();
	context->setContextProperty(QStringLiteral("bindingAddress"), bindingAddress);
	qDebug() << "Connect to: " << bindingAddress;

	std::string token = secret.toStdString();
	LibHomeScreen* hs = new LibHomeScreen();
	QLibWindowmanager* qwm = new QLibWindowmanager();

	// WindowManager
	if(qwm->init(port,secret) != 0) {
	    exit(EXIT_FAILURE);
	}
	AGLScreenInfo screenInfo(qwm->get_scale_factor());
	// Request a surface as described in layers.json windowmanager’s file
	if (qwm->requestSurface(graphic_role) != 0) {
	    exit(EXIT_FAILURE);
	}
	// Create an event callback against an event type. Here a lambda is called when SyncDraw event occurs
	qwm->set_event_handler(QLibWindowmanager::Event_SyncDraw, [qwm, &graphic_role](json_object *object) {
	    fprintf(stderr, "Surface got syncDraw!\n");
	    qwm->endDraw(graphic_role);
	});

	// HomeScreen
	hs->init(port, token.c_str());
	// Set the event handler for Event_TapShortcut which will activate the surface for windowmanager
	hs->set_event_handler(LibHomeScreen::Event_TapShortcut, [qwm, &graphic_role](json_object *object){
	    qDebug("Surface %s got tapShortcut\n", graphic_role.toStdString().c_str());
	    qwm->activateWindow(graphic_role);
	});

	context->setContextProperty(QStringLiteral("screenInfo"), &screenInfo);

	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
	QObject *root = engine.rootObjects().first();
	QQuickWindow *window = qobject_cast<QQuickWindow *>(root);
	QObject::connect(window, SIGNAL(frameSwapped()), qwm, SLOT(slotActivateWindow()
	));
	return app.exec();
}
