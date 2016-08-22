#include "pcbsd-SingleApplication.h"
#include <QDir>
#include <QFile>
#include <QLocalSocket>
#include <QDebug>

#include <unistd.h> //for getlogin()

PCSingleApplication::PCSingleApplication(int &argc, char **argv) : QApplication(argc, argv){
  //Initialize a couple convenience internal variables
  cfile = QDir::tempPath()+"/.PCSingleApp-%1-%2";
  QString username = QString(getlogin());
  QString appname = this->applicationName();
  cfile = cfile.arg( username, appname );
  lockfile = new QLockFile(cfile+"-lock");
    lockfile->setStaleLockTime(0); //long-lived processes
  for(int i=1; i<argc; i++){ inputlist << QString(argv[i]); }
  isActive = false;
  lserver = 0;
  PerformLockChecks();
}

PCSingleApplication::~PCSingleApplication(){
  if(lserver != 0 && lockfile->isLocked() ){ 
    //currently locked instance: remove the lock now
    lserver->close();
    QLocalServer::removeServer(cfile);
    lockfile->unlock(); 
  }
}

bool PCSingleApplication::isPrimaryProcess(){
  return isActive;	
}

void PCSingleApplication::PerformLockChecks(){
  bool primary = lockfile->tryLock();
  //qDebug() << "Try Lock: " << primary;
  if(!primary){
    //Pre-existing lock - check it for validity
    QString appname, hostname;
    qint64 pid;
    lockfile->getLockInfo(&pid, &hostname, &appname); //PID already exists if it gets this far, ignore hostname
    //qDebug() << " - Lock Info:" << pid << hostname << appname;
    if( appname!=this->applicationName() || !QFile::exists(cfile) ){
      //Some other process has the same PID or the server does not exist - stale lock
      //qDebug() << " - Stale Lock";
      lockfile->removeStaleLockFile();
      //Now re-try to create the lock
      primary = lockfile->tryLock();
      //qDebug() << " - Try Lock Again:" << primary;
    }
  }
  if(primary){
    //Create the server socket
    //qDebug() << "Create Local Server";
    if(QFile::exists(cfile)){ QLocalServer::removeServer(cfile); } //stale socket/server file
    lserver = new QLocalServer(this);
      connect(lserver, SIGNAL(newConnection()), this, SLOT(newInputsAvailable()) );
     if( lserver->listen(cfile) ){
        lserver->setSocketOptions(QLocalServer::UserAccessOption);
	//qDebug() << " - Success";
	isActive = true;
     }else{
	//qDebug() << " - Failure:" << lserver->errorString();
	lockfile->unlock();
     }
      
  }else{
    //forward the current inputs to the locked process for processing and exit
    //qDebug() << "Forward inputs to locking process:" << inputlist;
    QLocalSocket socket(this);
	socket.connectToServer(cfile);
	socket.waitForConnected();
	if(!socket.isValid()){ exit(1); } //error - could not forward info
	socket.write( inputlist.join("::::").toLocal8Bit() );
	socket.waitForDisconnected(500); //max out at 1/2 second (only hits this if no inputs
  }
  
}

//New messages detected
void PCSingleApplication::newInputsAvailable(){
  while(lserver->hasPendingConnections()){
    QLocalSocket *sock = lserver->nextPendingConnection();
    QByteArray bytes;
    //qDebug() << "New Socket Connection";
    sock->waitForReadyRead();
    while(sock->bytesAvailable() > 0){ //if(sock->waitForReadyRead()){
	//qDebug() << "Info Available";
	bytes.append( sock->readAll() );
    }
    sock->disconnectFromServer();
    QStringList inputs = QString::fromLocal8Bit(bytes).split("::::");
    qDebug() << " - New Inputs Detected:" << inputs;
    emit InputsAvailable(inputs);
  }
}
