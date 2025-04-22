#ifndef NODE_PROC_H
#define NODE_PROC_H

#include <QProcess>

#include "../baroscript/func_util.h"

#define PROC_LINE_CMD		0x10
#define PROC_CALL_ONE		0x20
#define PROC_MERGED			0x100
#define PROC_FORWORDED		0x200
#define PROC_SEPARATE		0x400

 
#define PROC_START			1
#define PROC_STARTED		2
#define PROC_READ			3
#define PROC_WRITE			4
#define PROC_FINISH			5
#define PROC_ERROR			9

class NodeEvent;
class NodeProcess : public QProcess, public XNode {
	Q_OBJECT
public:
	NodeProcess(TreeNode* cf=NULL, QObject * parent = 0);
	bool isRun();
	void genericRead(QByteArray& buf);
	void writeToStdin(LPCC text);
	bool startProcess(LPCC program, const QStringList& args, U32 flag=0);
	bool killProcess();
	bool stop(LPCC cmd=NULL);
 
signals:
	void lineAvailable(LPCC);

protected slots:
	void readStdOut();	
	void readTmpFile();
	void procFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void procError(QProcess::ProcessError err);
public:
#ifdef USE_TEMP_FILE
	QTimer			xtimer;
	QTemporaryFile	xfile;
#endif
	XListArr	xarr;
	XListArr	xparams;
	XFuncNode*	xfnProcess;
	TreeNode*	xroot;
	TreeNode*	xcf;
public:
	TreeNode* config() { return xcf; }
};

#endif

