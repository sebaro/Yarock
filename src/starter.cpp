
#include "starter.h"
#include "commandlineoptions.h"
#include "mainwindow.h"
#include "config.h"
#include "utilities.h"
#include "debug.h"

#include <QApplication>
#include <QTranslator>
#include <QNetworkReply>
#include <QDir>
#include <QString>
#include <QTime>
#include <QLocalServer>
#include <QLocalSocket>
#include <QLocale>
#include <QLibraryInfo>
#include <QDebug>
#include <QRandomGenerator>

#include <unistd.h> // for getuid
#include <X11/Xlib.h>
#include <csignal>


#define UDS_PATH QString("/tmp/yarock.sock.%1").arg(getuid()).toLatin1().constData()

void LoadTranslation(const QString& prefix, const QString& path, const QString& language) {
  QTranslator* t = new QTranslator();
  if (t->load(prefix + "_" + language, path))
    QCoreApplication::installTranslator(t);
  else
    delete t;
}

Starter::Starter(int argc,char **argv, QObject* parent) : QObject(parent) {
    m_mainwindow = 0;

    /* get command line */
    CommandlineOptions options(argc, argv);

    /* Help option -- or Invalid option */
    if( options.Parse() == false )
    {
      exit(0);
    }

    m_server = new QLocalServer(this);
    m_socket = new QLocalSocket(this);

    /* trying to create server */
    if( m_server->listen (UDS_PATH) )
    {
        startPlayer( options );
    }
    /* application already started */
    else if( QFile::exists(UDS_PATH) )
    {
       m_socket->connectToServer(UDS_PATH); //connecting
       m_socket->waitForConnected();

       if( !m_socket->isValid() ) //invalid connection
       {
           if( !QLocalServer::removeServer(UDS_PATH) )
           {
               qDebug("[Starter] unable to remove invalid socket file");
               exit(1);
               return;
           }

           qWarning("[Starter] removed invalid socket file");

           if( m_server->listen (UDS_PATH) )
           {
               startPlayer( options );
           }
           else
           {
               qWarning("[Starter] server error: %s", qPrintable(m_server->errorString()));
               exit(1);
           }
       }
       else
       {
           writeCommand( options );

           qDebug("[Starter] Exiting ...");

           exit(0);
       }
   }
   else
   {
       exit(0);
   }
}

Starter::~Starter() {
    qDebug("[~Starter]");

    if (m_mainwindow)
        delete m_mainwindow;
}

void Starter::startPlayer(const CommandlineOptions& options) {
    qDebug("[Starter] start a new instance of player");

    //! init debug activation
    qDebug() << "== main -> options.debug()" << options.debug();
    Debug::setDebugEnabled( options.debug() );
    if(!options.debug())
    {
        qDebug() << "**********************************************************************************************";
        qDebug() << "** YAROCK WAS STARTED IN NORMAL MODE. IF YOU WANT TO SEE DEBUGGING INFORMATION, PLEASE USE: **";
        qDebug() << "** yarock --debug                                                                           **";
        qDebug() << "**********************************************************************************************";
    }

    //! get langage (user or locale)
    QString language = options.language();
    language = language.isEmpty() ? QLocale::system().name() : language;
    qDebug() << "[Starter] locale : " << QLocale::system().name();

    //! setup translations
    LoadTranslation("qt", QLibraryInfo::path(QLibraryInfo::TranslationsPath), language);
    LoadTranslation("yarock", QLatin1String( CMAKE_INSTALL_TRANS ), language);
    LoadTranslation("yarock", QCoreApplication::applicationDirPath() + "../translation", language); // in case of local running
    qDebug() << "[Starter] translation loaded ";

    //! QRand initialisation
    QRandomGenerator::global()->generate();

    //! Directories settings
    QDir().mkpath(UTIL::CONFIGDIR);
    QDir().mkpath(UTIL::CONFIGDIR + "/radio");
    QDir().mkpath(UTIL::CONFIGDIR + "/albums");
    QDir().mkpath(UTIL::CONFIGDIR + "/artists");
    QDir().mkpath(UTIL::CONFIGDIR + "/lyrics");

    m_mainwindow = new MainWindow();
    connect(m_server, SIGNAL(newConnection()), SLOT(readCommand()));
    m_mainwindow->show();
}

void Starter::writeCommand(const CommandlineOptions& options) {
    qDebug("[Starter] writeCommand");
    QByteArray ba = options.Serialize();

    m_socket->write(ba);
    m_socket->flush();
}

void Starter::readCommand() {
    qDebug("[Starter] readCommand:");

    QLocalSocket *socket = m_server->nextPendingConnection();
    socket->waitForReadyRead();

    QByteArray inputArray = socket->readAll();

    if(inputArray.isEmpty())
    {
        socket->deleteLater();
        return;
    }

    CommandlineOptions options;
    options.Load( inputArray );

    m_mainwindow->set_command_line( options );
    m_mainwindow->commandlineOptionsHandle();

    socket->deleteLater();
}
