/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2018 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
*                                                                                       *
*  This program is free software; you can redistribute it and/or modify it under        *
*  the terms of the GNU General Public License as published by the Free Software        *
*  Foundation; either version 2 of the License, or (at your option) any later           *
*  version.                                                                             *
*                                                                                       *
*  This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
*  PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
*                                                                                       *
*  You should have received a copy of the GNU General Public License along with         *
*  this program.  If not, see <http://www.gnu.org/licenses/>.                           *
*****************************************************************************************/
#ifndef _STARTER_H_
#define _STARTER_H_

#include <QObject>
#include <QAbstractSocket>
#include <QStringList>

class QLocalServer;
class QLocalSocket;

class MainWindow;
class CommandlineOptions;


/*
********************************************************************************
*                                                                              *
*    Class Starter                                                             *
*                                                                              *
********************************************************************************
*/
/*!
 *  Starter represents wrapper object that is responsible
 * for proper QMMP initialization(only one instance of running
 * MainWindow) and passing command line args to application.
 * @author Vladimir Kuznetsov <vovanec@gmail.com>
 */
class Starter : public QObject
{
    Q_OBJECT
public:
    Starter(int argc,char ** argv,QObject* parent = 0);
    ~Starter();


private slots:
    void writeCommand( const CommandlineOptions& options );
    void readCommand();

private:
    void startPlayer( const CommandlineOptions& options );

private:
    MainWindow           *m_mainwindow;
    CommandlineOptions   *m_option_manager;
    QLocalServer         *m_server;
    QLocalSocket         *m_socket;
};

#endif // _STARTER_H_
