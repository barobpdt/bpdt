#include "func_util.h"
#include "../util/user_event.h"
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>
#include <QPrinterInfo>
#include <QPrintEngine>

#include "barcode.h"

inline double mmToPixels(QPrinter* printer, int mm) {
    if( printer==NULL) return 0;
    return mm * 0.039370147 * printer->resolution();
}

inline bool checkBool(TreeNode* cf, LPCC code) {
    bool chk=true;
    StrVar* sv=cf->gv(code);
    if( checkObject(sv,'3') ) {
        if(SVCHK('3',0)) chk=false;
    } else if(sv) {
        LPCC type=sv->get();
        if(ccmp(type,"false")) chk=false;
    }
    return chk;
}
inline QPrinter* getPrinter(TreeNode* tn, StrVar* var) {
    QPrinter* printer=NULL;
    LPCC mode=NULL;
    TreeNode* cf=NULL;
    StrVar* sv=var;
    if( SVCHK('n',0)) {
        cf=(TreeNode*)SVO;
        mode=cf->get("mode");
    }
    sv=tn->gv("@printer");
    if( SVCHK('p',11) ) {
        printer=(QPrinter*)SVO;
        if( slen(mode) && !tn->cmp("@mode", mode) ) {
            sv = tn->GetVar("@d");
            if( SVCHK('n',0) ) {
                TreeNode* d = (TreeNode*)SVO;
                sv=d->gv("@g");
                if( SVCHK('g',0) ) {
                    QPainter* painterPrev =(QPainter*)SVO;
                    SAFE_DELETE(painterPrev);
                }
                d->reuse();
            }
            SAFE_DELETE(printer);
        }
    }
    if( printer==NULL) {
        printer=new QPrinter(
         ccmp(mode,"screen")?QPrinter::ScreenResolution:
         ccmp(mode,"printer")?QPrinter::PrinterResolution:
         ccmp(mode,"high")?QPrinter::HighResolution: QPrinter::ScreenResolution
        );
        tn->GetVar("@printer")->setVar('p',11,(LPVOID)printer);
    }
    printerSetup(printer, var);
    return printer;
}

bool callPdfFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc) {
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        ref =
            ccmp(fnm,"drawObject") ? 1:
            ccmp(fnm,"preview") ? 2:
            ccmp(fnm,"end") ? 3:
            ccmp(fnm,"nextPage") ? 4:
            ccmp(fnm,"pageNumber") ? 5:
            ccmp(fnm,"html") ? 6:
            ccmp(fnm,"printState") ? 7:
            ccmp(fnm,"printerSetup") ? 8:
            ccmp(fnm,"abort") ? 9: ccmp(fnm,"pixels") ? 10:
            ccmp(fnm,"printerList")? 11:
            ccmp(fnm,"barcodeSetup")? 12:
            ccmp(fnm,"drawBarcode")? 13:
            0;
        if( fc ) fc->xfuncRef = ref;
    }

    if( ref==0 )
        return false;
    StrVar* sv=NULL;
    switch( ref ) {
    case 1: {		// drawObject
        /*
        sv = tn->gv("@printer");
        if( SVCHK('p',11) ) {
            QPrinter* printer = (QPrinter*)SVO;
            SAFE_DELETE(printer);
            sv->reuse();
        }
        */
        QPainter* painter = new QPainter();
        QPrinter* printer = getPrinter(tn, arrs? arrs->get(0): NULL);
        TreeNode* d=NULL;
        sv = tn->gv("@printer");
        if( SVCHK('p',11) ) {
            printer=(QPrinter*)SVO;
        } else {
            printer=new QPrinter(QPrinter::ScreenResolution);
            tn->GetVar("@printer")->setVar('p',11,(LPVOID)printer);
        }
        sv = tn->GetVar("@d");
        if( SVCHK('n',0) ) {
            d = (TreeNode*)SVO;
        } else {
            d=new TreeNode(2, true);
            sv->setVar('n',0,(LPVOID)d);
        }
        sv=d->gv("@g");
        if( SVCHK('g',0) ) {
            QPainter* painterPrev =(QPainter*)SVO;
            if( painterPrev!=painter ) {
                painterPrev->end();
                SAFE_DELETE(painterPrev);
            }
        }
        d->xstat=FNSTATE_DRAW;
        setVarRectF(d->GetVar("@rect"), QRectF(0,0,printer->width(),printer->height()) );
        d->GetVar("@g")->setVar('g',0,(LPVOID)painter);
        d->GetVar("@printer")->setVar('p',11,(LPVOID)printer);
        tn->GetVar("@d")->setVar('n',0,(LPVOID)d);
        tn->GetVar("@printer")->setVar('p',11,(LPVOID)printer);
        painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform, true);
        if( painter->begin(printer) ) {
            rst->setVar('n',0,(LPVOID)d);
        } else {
            d->GetVar("@g")->reuse();
            SAFE_DELETE(painter);
        }
    } break;
    case 2: {		// preview
        if( arrs==NULL ) {
            return true;
        }
        QPrinter* printer = getPrinter(tn, arrs->get(0));
        TreeNode* cf=NULL;
        XFuncSrc* fsrc=NULL;        
        sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            cf=(TreeNode*)SVO;
            sv=arrs->get(1);
        }
        if( SVCHK('f',1) ) {
            fsrc=(XFuncSrc*)SVO;
        }
        if( fsrc) {
            XFuncNode* fnCur = setCallbackFunction(fsrc, fn, tn, NULL, uom.getInfo()->GetVar("@pdfCallBack") );
            QPrintPreviewDialog preview(printer);
            QList<QPrintPreviewWidget*> list = preview.findChildren<QPrintPreviewWidget*>();
            if( !list.isEmpty() )  {
                list.first()->setZoomMode(QPrintPreviewWidget::FitToWidth);
            }
            if( cf ) {
                sv=cf->gv("icon");
                if( sv ) {
                    QPixmap* p = getStrVarPixmap(sv);
                    if( p ) {
                        preview.setWindowIcon(QIcon(*p));
                    }
                }
                sv=cf->gv("state");
                if( sv ) {
                    qtSetWindowState(&preview, toString(sv));
                }
                sv=cf->gv("flags");
                if( sv ) {
                    qtSetWindowFlags(&preview, sv );
                }
            }
            fnCur->GetVar("@this")->setVar('n',0,(LPVOID)tn);
            tn->setInt("@pageNum", 1);
            qDebug("----------- print preview start ----------");
            preview.resize(1000, 850);
            preview.connect(&preview, SIGNAL(paintRequested(QPrinter* )), &uom, SLOT(slotPaintRequested(QPrinter*)));
            preview.exec();
            rst->setVar('3',1);
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 3: {		// end
        sv=tn->GetVar("@d");
        if( SVCHK('n',0) ) {
            TreeNode* d=(TreeNode*)SVO;
            sv=d->gv("@g");
            if( SVCHK('g',0) ) {
                QPainter* p=(QPainter*)SVO;
                qDebug("#0#pdf end ok");
                p->end();
            }
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 4: {		// nextPage
        sv = tn->gv("@printer");
        if( SVCHK('p',11) ) {
            QPrinter* p = (QPrinter*)SVO;
            p->newPage();
            tn->incrNum("@pageNum");
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 5: {		// pageNumber
        rst->setVar('0',0).addInt( tn->getInt("@pageNum") );
    } break;
    case 6: {		// html
        if( arrs==NULL ) {
            return true;
        }
        QPrinter* printer = getPrinter(tn, arrs->get(0));
        QTextDocument document;
        sv=arrs->get(0);
        if( SVCHK('n',0)) sv=arrs->get(1);
        if( sv ) {
            LPCC html=toString(sv);
            document.setHtml(KR(html));
            document.setPageSize(printer->pageRect().size());
            document.print(printer);
        }
    } break;
    case 7: {		// printerState
        sv = tn->gv("@printer");
        if( SVCHK('p',11) ) {
            QPrinter* printer = (QPrinter*)SVO;
            switch(printer->printerState()) {
            case QPrinter::Idle: rst->set("idle"); break;
            case QPrinter::Active: rst->set("active"); break;
            case QPrinter::Aborted: rst->set("aborted"); break;
            default: rst->set("error"); break;
            }
        }
    } break;
    case 8: {       // printerSetup
        if( arrs==NULL) {
            return true;
        }
        QPrinter* p=getPrinter(tn, arrs->get(0));
        if( p ) {
            setVarRectF(rst, p->pageRect());
        }
    } break;
    case 9: {		// abort
        sv = tn->gv("@printer");
        if( SVCHK('p',11) ) {
            QPrinter* printer = (QPrinter*)SVO;
            printer->abort();
            switch(printer->printerState()) {
            case QPrinter::Idle: rst->set("idle"); break;
            case QPrinter::Active: rst->set("active"); break;
            case QPrinter::Aborted: rst->set("aborted"); break;
            default: rst->set("error"); break;
            }
        }
    } break;
    case 10: {		// pixels
        int num=10;
        if( arrs ) {
            sv=arrs->get(0);
            if(isNumberVar(sv)) num=toInteger(sv);
        }
        sv = tn->gv("@printer");
        if( SVCHK('p',11) ) {
            QPrinter* printer = (QPrinter*)SVO;
            rst->setVar('4',0).addDouble(mmToPixels(printer, num));
        }
    } break;
    case 11: {		// printerList
        TreeNode* root=NULL;
        if(arrs ) {
            sv=arrs->get(0);
            if( SVCHK('n',0)) {
                root=(TreeNode*)SVO;
            }
        }
        if( root==NULL ) {
            root=getTreeNodeTemp();
        }
        char buf[32];
        int idx=0;
        QList<QPrinterInfo> info_list = QPrinterInfo::availablePrinters();
        foreach ( QPrinterInfo info, info_list ) {
            TreeNode* cur=root->addNode();
            idx++;
            cur->set("type", "printer");
            cur->setInt("idx", idx);
            cur->set("name", Q2A(info.printerName()));
            QList<QPrinter::PaperSize> sizes=info.supportedPaperSizes();
            foreach ( QPrinter::PaperSize sz, sizes ) {
                TreeNode* sub=cur->addNode();
                sprintf(buf, "p%d", (int)sz);
                sv=sub->gv(buf);
                if(sv==NULL) {
                    sub->GetVar(buf)->setVar('3',1);
                    sub->GetVar("paperIndex")->setVar('0',0).addInt((int)sz);
                }
            }
        }
        rst->setVar('n',0,(LPVOID)root);
    } break;
    case 12: {		// barcodeSetup
        if( arrs==NULL ) {
            return true;
        }
    } break;
    case 13: {		// drawBarcode
        if( arrs==NULL ) {
            return true;
        }
#ifdef BARCORD_USE

#endif
    } break;
    default: return false;
    }
    return true;
}

TreeNode* printerSetup(QPrinter* printer, StrVar* sv) {
    if( sv==NULL) return NULL;
    if( isNumberVar(sv) || SVCHK('f',1) || SVCHK('3',1) || SVCHK('3',0) ) {
        return NULL;
    }
    TreeNode* cf=NULL;
    LPCC path=NULL,type=NULL;
    if( SVCHK('n',0) ) {
        cf=(TreeNode*)SVO;
        path=cf->get("pdfFileName");
        if(slen(path)==0) path=cf->get("fileName");
    } else {
        path=toString(sv);
    }
    // qDebug("#0#printer setup file path: %s\n", path);

    if( slen(path) ) {
        printer->setOutputFormat(QPrinter::PdfFormat);
        printer->setOutputFileName(KR(path));
    } else {
        LPCC name=cf->get("name");
        if( slen(name)) {
            printer->setPrinterName(KR(name));
        } else {
            QString printerName = printer->printEngine()->property(QPrintEngine::PPK_PrinterName).toString();
            qDebug()<<"print setup printer name: " << printerName;
            if( !printerName.isEmpty()) {
                printer->setPrinterName(printerName);
            }
        }
    }
    if( cf==NULL ) {
        return NULL;
    }
    /*
    [mode] ScreenResolution, PrinterResolution, HighResolution, Compatible
    [size] A4, B5, Letter, Legal, Executive, A0, A1, A2, A3, A5, A6, A7, A8, A9, B0,
    B1, B10, B2, B3, B4, B6, B7, B8, B9, C5E, Comm10E, DLE, Folio, Ledger, Tabloid
    [orientation] Portrait, Landscape
    [order] FirstPageFirst, LastPageFirst
    [color] GrayScale, Color
    [paper] OnlyOne, Lower, Middle, Manual, Envelope, EnvelopeManual, Auto,
       Tractor, SmallFormat, LargeFormat, LargeCapacity, Cassette, FormSource
    [range] AllPages, Selection, PageRange
    [option] PrintToFile, PrintSelection, PrintPageRange
    */


    type=cf->get("order");
    if(ccmp(type,"first")) {
        printer->setPageOrder(QPrinter::FirstPageFirst);
    } else if( ccmp(type,"last")) {
        printer->setPageOrder(QPrinter::LastPageFirst);
    }
    float w=0, h=0;
    sv=cf->gv("size");
    if( sv ) {
        if( isNumberVar(sv) ) {
            int num=toInteger(sv);
            printer->setPageSize((QPrinter::PageSize)num);
        } else if( sv && sv->find(",") ) {
            XParseVar pv(sv);
            while(pv.valid()) {
                LPCC val=pv.findEnd(",").v();
                if(isnum(val)) {
                    w=atof(val);
                }
                if(pv.valid()) {
                    val=pv.findEnd(",").v();
                    if(isnum(val)) h=atof(val);
                }
            }
        }
#ifdef QT5_USE
        if( w>0 && h>0 ) {
            QPageSize pageSize(QSizeF(w,h),QPageSize::Millimeter,"",QPageSize::ExactMatch);
            printer->setPageSize(pageSize);
            printer->setFullPage(true);
        } else
#endif
        {
            type=cf->get("size");
            // A4, B5, Letter, Legal
            if( ccmp(type,"A4") ) {
                printer->setPageSize(QPrinter::A4);
            } else if( ccmp(type,"A3") ) {
                printer->setPageSize(QPrinter::A3);
            } else if( ccmp(type,"B5") ) {
                printer->setPageSize(QPrinter::B5);
            } else if( ccmp(type,"Letter") ) {
                printer->setPageSize(QPrinter::Letter);
            } else if( ccmp(type,"Legal") ) {
                printer->setPageSize(QPrinter::Legal);
            } else if( ccmp(type,"Executive") ) {
                printer->setPageSize(QPrinter::Executive);
            } else {
                printer->setPageSize(QPrinter::A4);
            }
        }

    }


    type=cf->get("type");
    if( ccmp(type,"hbox") ) {
        printer->setOrientation(QPrinter::Landscape);
    } else {
        printer->setOrientation(QPrinter::Portrait);
    }
    int x=15, y=15, r=15, b=15;
    sv=cf->gv("margin");
    if( sv ) {
        XParseVar pv(sv);
        int n=0;
        for( ; n<4 && pv.valid(); n++) {
            LPCC val=pv.findEnd(",").v();
            if(isnum(val)) {
                if(n==0) x=atoi(val);
                else if(n==1) y=atoi(val);
                else if(n==2) r=atoi(val);
                else if(n==3) b=atoi(val);
            }
        }
        if(n==1) {
            y=x, r=x, b=x;
        }
    }
    printer->setPageMargins (x,y,r,b,QPrinter::Millimeter);
    type=cf->get("full");
    if(ccmp(type,"true")) {
        printer->setFullPage( true );
    } else {
        printer->setFullPage( false );
    }
    sv=cf->gv("count");
    if( isNumberVar(sv)) {
        printer->setCopyCount(toInteger(sv));
    }
    sv=cf->gv("resolution");
    if( isNumberVar(sv)) {
        printer->setResolution(toInteger(sv));
    }
    sv=cf->gv("auto");
    if( checkBool(cf,"auto") ) {
        printer->setPaperSource(QPrinter::Auto);
    }
    if( checkBool(cf,"native") ) {
        printer->setOutputFormat(QPrinter::NativeFormat);
    }

    if( checkBool(cf,"color") ) {
        printer->setColorMode(QPrinter::Color);
    } else if( checkBool(cf,"gray") ) {
        printer->setColorMode(QPrinter::GrayScale);
    }
    return cf;
}
