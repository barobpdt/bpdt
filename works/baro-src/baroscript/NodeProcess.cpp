#include "NodeProcess.h"
#include <qprocess.h>

#define FUNC_NODE_USE

NodeProcess::NodeProcess(TreeNode* cf, QObject * parent) : QProcess(parent), XNode(4), xcf(cf), xroot(NULL), xfnProcess(NULL) {

#if USE_TEMP_FILE
	xfile.open(); // Create temporary file
	QString filename = xfile.fileName();
	setStandardOutputFile( filename );
	qDebug("NodeProcess::NodeProcess: temporary file: %s\n", filename.toUtf8().data());
	xfile.close();
	// connect(&xfile, SIGNAL(readyRead()), this, SLOT(readTmpFile()) );
	connect(&timer, SIGNAL(timeout()), this, SLOT(readTmpFile()) );
#else
	connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(readStdOut()) );
#endif
	connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(procFinished(int, QProcess::ExitStatus)) ); 
	connect(this, SIGNAL(error(QProcess::ProcessError)), this, SLOT(procError(QProcess::ProcessError)) );
}

  

bool NodeProcess::isRun() {
	return (state() == QProcess::Running);
}

bool NodeProcess::startProcess(LPCC program, const QStringList& args, U32 flag ) {    
	if( slen(program)==0 ) 
		return false;
    StrVar* sv=xcf->gv("@callback");
    if(SVCHK('f',0) ) {
        xfnProcess=(XFuncNode*)SVO;
    }
	if( isRun() ) {
		if( xfnProcess ) {
            PARR arrs=getLocalParams(xfnProcess);
            arrs->add()->set("error");
            arrs->add()->set("process is running == true");
            setFuncNodeParams(xfnProcess, arrs);
            xfnProcess->call();
		}		
	}
	config()->set("@program", program);
 	//	setProcessChannelMode( QProcess::MergedChannels );
	/*
	enum ProcessChannelMode {
        SeparateChannels,
        MergedChannels,
        ForwardedChannels
    }; **************************************************************************/

	if( flag&PROC_FORWORDED ) 
		setProcessChannelMode( QProcess::ForwardedChannels );
	else if( flag&PROC_SEPARATE )
		setProcessChannelMode( QProcess::SeparateChannels );
    else if( flag&0x800 )
        setProcessChannelMode( QProcess::ForwardedOutputChannel );
    else
		setProcessChannelMode( QProcess::MergedChannels );
	if( flag&0x20 ) {
		LPCC path=config()->get("@workPath");
		UL64 processId=0;
		QString workPath=  slen(path) ? KR(path) : QCoreApplication::applicationDirPath();
		QProcess::startDetached(KR(program), args, workPath, &processId);
		config()->GetVar("@processId")->setVar('1',0).addUL64(processId);
	} else {
        this->start(KR(program), args);
        if(flag&0x1) {
            connect(this, SIGNAL(readyReadStandardOutput()), this, SLOT(readStdOut()) );
            connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(procFinished(int, QProcess::ExitStatus)) );
            connect(this, SIGNAL(error(QProcess::ProcessError)), this, SLOT(procError(QProcess::ProcessError)) );
        }
	}

#ifdef USE_TEMP_FILE
	//bool r = xfile.open(QIODevice::ReadOnly);
	bool r = xfile.open();
	xtimer.start(50);
	qDebug("NodeProcess::start: r: %d\n", r);
#endif
	bool rtn = waitForStarted();
    if( xfnProcess ) {
        PARR arrs=getLocalParams(xfnProcess);
        arrs->add()->set("start");
        arrs->add()->set(rtn?"start":"error");
        setFuncNodeParams(xfnProcess, arrs);
        xfnProcess->call();
	}
    if(flag&0x2) {
        bool ok=waitForFinished();
        if( xfnProcess ) {
            PARR arrs=getLocalParams(xfnProcess);
            arrs->add()->set("finish");
            arrs->add()->set(ok?"ok":"wait");
            setFuncNodeParams(xfnProcess, arrs);
            xfnProcess->call();
        }
    }
	return rtn;
}

bool NodeProcess::stop(LPCC cmd) {
	if( cmd ) {
		writeToStdin( cmd );
	}
	if( !waitForFinished(1000)) {
		qWarning("Core::stopMplayer: process didn't finish. Killing it...");
		kill();
	}
	return true;
}

/***************************************************************************************
enum ProcessError {
    FailedToStart, //### file not found, resource error
    Crashed,
    Timedout,
    ReadError,
    WriteError,
    UnknownError
};
**************************************************************************************/

void NodeProcess::procError(QProcess::ProcessError err) {
	qDebug("NodeProcess::procError: %d\n", (int) err );
	if( xfnProcess && xparams.size()>1 ) { 
        PARR arrs=getLocalParams(xfnProcess);
        arrs->add()->set("error");
        arrs->add()->fmt("error: %d", (int)err);
        setFuncNodeParams(xfnProcess, arrs);
        xfnProcess->call();
	}
}

void NodeProcess::writeToStdin(LPCC text) {
	if( slen(text)==0 ) {
		return;
	}
	if (isRun()) {
		//qDebug("MplayerProcess::writeToStdin"); 
        write( text );
        if( xfnProcess ) {
            PARR arrs=getLocalParams(xfnProcess);
            arrs->add()->set("write");
            arrs->add()->set(text);
            setFuncNodeParams(xfnProcess, arrs);
			xfnProcess->call();
        }
	} else {
        if( xfnProcess  ) {
            PARR arrs=getLocalParams(xfnProcess);
            arrs->add()->set("error");
            arrs->add()->fmt("writeToStdin: process not running: %s", text);
            setFuncNodeParams(xfnProcess, arrs);
            xfnProcess->call();
		} 
		// qWarning("Process::writeToStdin: process not running");
	} 
}

void NodeProcess::readStdOut() {
    QByteArray ba=readAllStandardOutput();
    genericRead( ba );

}

void NodeProcess::readTmpFile() {
#ifdef USE_TEMP_FILE
	genericRead( xfile.readAll() );
#endif
}

void NodeProcess::procFinished(int exitCode, QProcess::ExitStatus exitStatus) {	
    // qDebug("NodeProcess::processFinished: exitCode: %d, status: %d\n", exitCode, (int) exitStatus);
#ifdef USE_TEMP_FILE
	xtimer.stop();
	qDebug("NodeProcess::procFinished: Bytes available: %ld\n", xfile.bytesAvailable());
	if ( xfile.bytesAvailable() > 0 ) readTmpFile();
	qDebug("NodeProcess::procFinished: Bytes available: %ld\n", xfile.bytesAvailable());
	xfile.close();
#else
	if( bytesAvailable() > 0 )
		readStdOut();
#endif
    if( xfnProcess ) {
        PARR arrs=getLocalParams(xfnProcess);
        arrs->add()->set("finish");
        arrs->add()->set("ok");
        setFuncNodeParams(xfnProcess, arrs);
        xfnProcess->call();
    }
}

void NodeProcess::genericRead(QByteArray& buf) {
    if( xfnProcess ) {
        PARR arrs=getLocalParams(xfnProcess);
        arrs->add()->set("read");
        arrs->add()->set(buf.constData());
        setFuncNodeParams(xfnProcess, arrs);
		xfnProcess->call();
	}

#ifdef USE_TEMP_FILE
	if( isNodeFlag(PROC_CALL_ONE) ) 
		return;

	ParseVar pv(var);
	while( pv.start<pv.wep ) {
		if( pv.Find("\n") ) {
			emit lineAvailable(pv.FindValue());
		} else {
			var->set(pv.get(),pv.remain());
			break;
		}
	}
#endif
}

bool NodeProcess::killProcess() {
	// kill();
    if( !waitForFinished(1000) ) {
		qWarning("Core::stopMplayer: process didn't finish. Killing it...");
		kill();
	}
	return true;
}

//
//
//

