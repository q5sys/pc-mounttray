// 
//  Source code for pc-mounttray
//
#ifndef _PC_MOUNTTRAY_GLOBALS_H
#define _PC_MOUNTTRAY_GLOBALS_H

#include <QProcess>
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>
#include <QCoreApplication>

inline QStringList runShellCommand(QString cmd){
  QProcess p;  
   //Make sure we use the system environment to properly read system variables, etc.
   p.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
   //Merge the output channels to retrieve all output possible
   p.setProcessChannelMode(QProcess::MergedChannels);   
   p.start(cmd);
   while(p.state()==QProcess::Starting || p.state() == QProcess::Running){
     p.waitForFinished(200);
     QCoreApplication::processEvents();
   }
   QString tmp = p.readAllStandardOutput();
    if(tmp.endsWith("\n")){ tmp.chop(1); }
   return tmp.split("\n");
}

#endif
