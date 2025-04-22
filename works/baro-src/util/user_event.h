#ifndef USER_EVENT_H
#define USER_EVENT_H

// #define Q_STATIC_ASSERT(Condition)
// #define Q_STATIC_ASSERT_X(Condition, Message)

#include "mywidget.h"
#include "../baroscript/func_util.h"

#ifdef WINDOW_USE
#include "../baroscript/Console.h"
#endif

#define MAX_TEMP_STR_NUM 256

/************************************************************************/
/*  RunGuard			                                                */
/************************************************************************/
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QPrinter>

class RunGuard {

public:
    RunGuard( const QString& key );
    ~RunGuard();

    bool isAnotherRunning();
    bool tryToRun();
    void release();

private:
    const QString key;
    const QString memLockKey;
    const QString sharedmemKey;

    QSharedMemory sharedMem;
    QSystemSemaphore memLock;

    Q_DISABLE_COPY( RunGuard )
};

/************************************************************************/
/*  UserEvent			                                                */
/************************************************************************/
const QEvent::Type USER_EVENT = static_cast<QEvent::Type>(QEvent::User + 1);

class UserEvent : public QEvent {
public:
    UserEvent(TreeNode* cf, int ty, StrVar* data) : QEvent(USER_EVENT), xcf(cf), xtype(ty), xcallback(NULL) {
        if( data ) {
            xdata.reuse()->add(data);
        }
    }
public:
    TreeNode*	xcf;
    StrVar		xdata;
    int			xtype;
    XFuncNode*	xcallback;
    void setCallback(XFuncNode* callback) { if( callback ) xcallback=callback; }
    void setEventInfo(TreeNode* cf, int ty, StrVar* data=NULL);
};


VOID CALLBACK callbackWatcherRoutine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped );

class WatcherFile {
public:
    WatcherFile(TreeNode* cf);
    ~WatcherFile();
    TreeNode* config() { return xcf; }
    TreeNode* xcf;

public:
    bool start(LPCC targetPath);
    bool go();
    bool addChange(PWCHAR filenm, int filenmLen, int act);
#ifdef WIN32
    void* xol;
    HANDLE			xhandle;
    unsigned long	xbufLen;
    unsigned char*	xbuf;
    StrVar          xWatcherFile;

    // HOOKPROC		xproc;
    // HHOOK		xhook;
#endif
};

#ifdef WIN32
typedef struct : public OVERLAPPED {
    WatcherFile* w;
} Overlapped;
#endif

class UserObjectManager : public QObject {
    Q_OBJECT
public:
    UserObjectManager();
    ~UserObjectManager();
public:
    static UserObjectManager& instance() {
        static UserObjectManager gui;
        return (UserObjectManager&)gui;
    }
public:
    U32 xflag;
    int xsidx;
    XListArr xstrs;
    TreeNode xinfos;
    QTimer xtimer;
#ifdef WINDOW_USE
    CConsole*	xconsole;
#endif
public slots:
    void onTimeout();
    void slotPaintRequested(QPrinter* p);

public:
    bool createConsole();
    void startTimer();
    void stopTimer();
    void printDebug(LPCC msg, int ty=0);
    StrVar*	getStrVar(bool move=true);
    TreeNode* getInfo();

public slots:
    void onButtonClick(TreeNode* cf, bool checked=false);
    void onButtonTrigger(TreeNode* cf, bool checked=false);
    void onSpinValueChanged(TreeNode* cf, const QString & text);

    void onStackedCurrentChanged(TreeNode* cf,  int index );
    void onStackedWidgetRemoved(TreeNode* cf,  int index );

    void onSplitterMoved(TreeNode* cf, int pos, int index);

    void onTabCurrentChanged (TreeNode* cf,  int index );
    void onTabCloseRequested (TreeNode* cf,  int index );
    void onViewCurrentChanged ( TreeNode* cf, const QModelIndex & current, const QModelIndex & previous );
    void onViewCurrentColumnChanged ( TreeNode* cf, const QModelIndex & current, const QModelIndex & previous );
    void onViewCurrentRowChanged ( TreeNode* cf, const QModelIndex & current, const QModelIndex & previous );
    void onViewSelectionChanged ( TreeNode* cf, const QItemSelection & selected, const QItemSelection & deselected );
    void onViewDoubleClicked(TreeNode* cf, const QModelIndex & index);
    void onViewClicked( TreeNode* cf, const QModelIndex & index );

    void onProgressValueChanged(TreeNode* cf, int value);
    void onComboActivated(TreeNode* cf, int index);
    void onComboCurrentChanged(TreeNode* cf, int index);

    void onInputPositionChanged(TreeNode* cf, int prev, int cur);
    void onEditingFinished(TreeNode* cf);
    void onReturnPressed(TreeNode* cf);
    void onSelectionChanged(TreeNode* cf);
    void onTextChanged(TreeNode* cf, const QString & text);

    void onEditorPositionChanged(TreeNode* cf);
    void onCurrentSelectionChanged(TreeNode* cf);
    void onEditorChanged(TreeNode* cf);
    void onActionClick(TreeNode* cf);
    void onChanageClipboard();

protected:
    virtual bool event(QEvent* evt);
};

#endif // USER_EVENT_H
