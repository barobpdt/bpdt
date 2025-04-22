#ifndef FUNC_FTP_H
#define FUNC_FTP_H

#include "qftp.h"
#include "func_util.h"

class MyFtp : public QObject {
    Q_OBJECT
public:
    MyFtp(TreeNode* cf, QObject *parent = 0);
    TreeNode* config() { return xcf; }

public:
    QFtp* ftp;
    TreeNode* xcf;
public:
    int getState() { return ftp? (int)ftp->state(): 0; }
    int getCommand(){ return ftp? (int)ftp->currentCommand(): 0; }
    int setMode(LPCC mode);
    int connectHost(LPCC serveer, int port);
    int cd(LPCC path);
    int put(LPCC localFileName, LPCC remoteFileName);
    int get(LPCC remoteFileName, LPCC saveFileName);
    int login(LPCC userId, LPCC userPwd );
    void getFileList();
    void ftpCheckDisconnect();

private slots:
    void ftpStateChanged(int state);
    void ftpFinished(int request, bool error);
    void ftpListInfo(QUrlInfo info);
    void ftpProgress(qint64 done, qint64 total);

private:
    QFile *file;
    QStringList files;
};

#endif // FUNC_FTP_H
