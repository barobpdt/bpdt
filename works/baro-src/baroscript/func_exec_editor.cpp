#include "func_util.h"
#include <QPrintPreviewDialog>
#include <QFileDialog>
#include <QScrollBar>

inline int textEndPosition(QTextDocument* doc ) {
    return doc->characterCount() - 1;
    /*
    if( c.isNull() ) return 0;
    c.movePosition(QTextCursor::End);
    return c.position();
    */
}

void mergeFormatOnWordOrSelection(GTextEdit* widget, const QTextCharFormat &format) {
    QTextCursor cursor = widget->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    widget->mergeCurrentCharFormat(format);
}

QTextCursor::MoveOperation getTypeMoveOperation(LPCC ty) {
    if( ccmp(ty,"start") ) {
        return QTextCursor::Start;
    } else if( ccmp(ty,"end") ) {
        return QTextCursor::End;
    } else if( ccmp(ty,"up") ) {
        return QTextCursor::Up;
    } else if( ccmp(ty,"lineStart") ) {
        return QTextCursor::StartOfLine;
    } else if( ccmp(ty,"lineEnd") ) {
        return QTextCursor::EndOfLine;
    } else if( ccmp(ty,"blockStart") ) {
        return QTextCursor::StartOfBlock;
    } else if( ccmp(ty,"blockEnd") ) {
        return QTextCursor::EndOfBlock;
    } else if( ccmp(ty,"wordStart") ) {
        return QTextCursor::StartOfWord;
    } else if( ccmp(ty,"wordEnd") ) {
        return QTextCursor::EndOfWord;
    } else if( ccmp(ty,"wordRight") ) {
        return QTextCursor::WordRight;
    } else if( ccmp(ty,"prevWord") ) {
        return QTextCursor::PreviousWord;
    } else if( ccmp(ty,"nextWord") ) {
        return QTextCursor::NextWord;
    } else if( ccmp(ty,"prevChar") ) {
        return QTextCursor::PreviousCharacter;
    } else if( ccmp(ty,"nextChar") ) {
        return QTextCursor::NextCharacter;
    } else if( ccmp(ty,"down") ) {
        return QTextCursor::Down;
    } else if( ccmp(ty,"left") ) {
        return QTextCursor::Left;
    } else if( ccmp(ty,"right") ) {
        return QTextCursor::Right;
    }
    return QTextCursor::NoMove;
}

int editorMovePos( QTextCursor* c, LPCC ty, TreeNode* tn ) {
    if( c==NULL ) return 0;
    StrVar* sv=NULL;
    int pos=0;
    if( ccmp(ty,"selectStart") || ccmp(ty,"selectEnd") ) {
        if( ccmp(ty,"selectStart") )
            pos = c->selectionStart();	// sv->getInt(4);
        else
            pos = c->selectionEnd();	// sv->getInt(8);
    } else if( ccmp(ty,"wordStart") ) {
        c->select(QTextCursor::WordUnderCursor);
        pos=c->selectionStart();
    } else if( ccmp(ty,"wordEnd") ) {
        c->select(QTextCursor::WordUnderCursor);
        pos=c->selectionEnd();
    } else if( ccmp(ty,"sp") ) {
        sv = tn->gv("@sp");
        if( isNumberVar(sv) ) {
            pos = toInteger(sv);
        }

    } else if( ccmp(ty,"nextTab") ) {
        sv = tn->gv("@tapPositions");
        if( SVCHK('a',0) ) {
            XListArr* arr = (XListArr*)SVO;
            int idx= tn->incrNum("tabPositionsIndex");
            sv = arr->get(idx);
            if( SVCHK('0',0) ) {
                pos = sv->getInt(4);
                c->movePosition(getTypeMoveOperation("end"), QTextCursor::MoveAnchor);
                pos = min(pos,c->position());
                if( idx==(arr->size()-1) ) arr->reuse();
            } else {
                arr->reuse();
            }
        }
    } else if( ccmp(ty,"movePos") ) {
        sv = tn->gv("@movePos");
        if( isNumberVar(sv) ) {
            pos = toInteger(sv);
        }
    } else {
        c->movePosition(getTypeMoveOperation(ty), QTextCursor::MoveAnchor);
        pos = c->position();
    }
    if( pos<0 ) pos = 0;
    return pos;
}

bool callTextEditFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, GTextEdit* w, XFunc* fc) {
    int cnt = arrs ? arrs->size(): 0;
    StrVar* sv=NULL;
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        ref=
            ccmp(fnm,"value") ? 811 :
            ccmp(fnm,"search") ? 1 :
            ccmp(fnm,"searchNext") ? 2 :
            ccmp(fnm,"searchPrev") ? 3 :
            ccmp(fnm,"replace") ? 4 :
            ccmp(fnm,"replaceAll") ? 5 :
            ccmp(fnm,"print") ? 6 :
            ccmp(fnm,"printPdf") ? 7 :
            ccmp(fnm,"text") ? 13 :
            ccmp(fnm,"append") ? 15 :
            ccmp(fnm,"select") ? 16 :
            ccmp(fnm,"syntax") ? 22:
            ccmp(fnm,"insert") ? 24 :
            ccmp(fnm,"sp") ? 31 :
            ccmp(fnm,"spText") ? 32 :
            ccmp(fnm,"cursorRect") ? 33 :
            ccmp(fnm,"cursorPos") ? 34 :
            ccmp(fnm,"findAll") ? 41 :
            ccmp(fnm,"insertImage") ? 42 :
            ccmp(fnm,"hitTest") ? 43 :
            ccmp(fnm,"mark") ? 44 :
            ccmp(fnm,"clear") ? 45 :
            ccmp(fnm,"toSyntax") ? 50 :
            ccmp(fnm,"anchorAt") ? 52 :
            ccmp(fnm,"setCursor") ? 53 :
            ccmp(fnm,"zoomIn") ? 54 :
            ccmp(fnm,"zoomOut") ? 55 :
            ccmp(fnm,"lineHide") ? 56 :
            ccmp(fnm,"lineNumber") ? 57 :
            ccmp(fnm,"lineRect") ? 58 :
            ccmp(fnm,"lineNumWidth") ? 60 :
            ccmp(fnm,"insertCursor") ? 61 :
            ccmp(fnm,"insertHtml") ? 62 :
            ccmp(fnm,"scrollTop") ? 63 :
            ccmp(fnm,"scrollPos") ? 64 :
            ccmp(fnm,"frameMargin") ? 65 :
            ccmp(fnm,"update") ? 66 :
            ccmp(fnm,"css") ? 67 :
            ccmp(fnm,"clearUndo") ? 68 :
            // ccmp(fnm,"htmlPaste") ? 69 :
            ccmp(fnm,"mergeFormat") ? 70 :
            ccmp(fnm,"html") ? 71 :
            ccmp(fnm,"linePos") ? 72 :
            ccmp(fnm,"copy") ? 73 :
            ccmp(fnm,"paste") ? 74 :
            ccmp(fnm,"pos") ? 171 :
            ccmp(fnm,"movePos") ? 172 :
            ccmp(fnm,"is") ? 173 :
            ccmp(fnm,"deletePoint") ? 175 :
            ccmp(fnm,"imageUrl") ? 176 :
            ccmp(fnm,"lineCount") ? 177 :
            ccmp(fnm,"align") ? 178 :
            ccmp(fnm,"syntaxAt") ? 179 :
            ccmp(fnm,"highlight") ? 180 :
            999;
    }
    if( ref==0 )
        return false;
    switch( ref ) {
    case 1: // search
    case 2:
    case 3: {
        if( cnt==0 ) return false;
        tn->GetVar("@markTick")->setVar('1',0).addUL64(GetTickCount());
        LPCC findStr = AS(0);
        U32 flag = 0;
        if( isNumberVar(arrs->get(1)) ) {
            flag=toInteger(arrs->get(1));
        } else {
            flag=EF_WRAPEND;
        }
        QTextCursor cursor=w->textCursor();
        int count=findEditorCursor(w->document(), cursor, findStr, flag, ref==3? true: false, NULL, sv);
        if( count>0 ) {
            if( flag & EF_MARK ) {
                w->textCursorPositionChanged();
            } else {
                w->setTextCursor(cursor);
            }
        }
        rst->setVar('0',0).addInt(count);
    } break;
    case 4:		// replace
    case 5: {	// replaceAll
        if( cnt==0 ) return false;
        tn->GetVar("@markTick")->setVar('1',0).addUL64(GetTickCount());
        LPCC findStr = AS(0);
        LPCC repStr=AS(1);
        U32 flag = toInteger(arrs->get(2));
        if( ref==5 ) flag|=EF_REPLACEALL;
        QTextCursor cursor=w->textCursor();
        if( cursor.hasSelection() && flag==0 ) {
            flag|=EF_SELECTTEXT;
        }
        int count=findEditorCursor(w->document(), cursor, findStr, flag, false, repStr, rst->reuse());
        if( count>0 ) {
            w->setTextCursor(cursor);
            w->textCursorPositionChanged();
        }
        rst->setVar('0',0).addInt(count);
    } break;
    case 6: {	// print
        QPrinter printer(QPrinter::HighResolution);
        QPrintPreviewDialog preview(&printer, w);
        w->connect(&preview, SIGNAL(paintRequested(QPrinter*)), SLOT(printPreview(QPrinter*)));
        preview.exec();
    } break;
    case 7: {	// printPdf
        QString fileName = QFileDialog::getSaveFileName(w, "PDF 출력",QString(), "*.pdf");
        if( !fileName.isEmpty()) {
            if (QFileInfo(fileName).suffix().isEmpty())
                fileName.append(".pdf");
            QPrinter printer(QPrinter::HighResolution);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(fileName);
            w->document()->print(&printer);
        }
    } break;
    case 13: {	// text
        rst->reuse();
        StrVar* sv = NULL; // tn->gv("@cursor");
        QTextCursor c = w->textCursor(); // SVCHK('c',2)? static_cast<QTextCursor>(*((QTextCursor*)SVO)) :
        int cnt=arrs ? arrs->size(): 0;
        if( cnt==0 ) {
            rst->set( Q2A(w->toPlainText()) );
        } else if( isNumberVar(arrs->get(0)) && cnt==2 ) {
            int sp = toInteger(arrs->get(0));
            // c.movePosition(getTypeMoveOperation("end"), QTextCursor::MoveAnchor);
            int end=textEndPosition( w->document() );
            c.setPosition(sp);
            if( isNumberVar(arrs->get(1)) ) {
                int ep = toInteger(arrs->get(1));
                if( ep>end ) {
                    ep=end;
                }
                if( sp<ep ) {
                    c.setPosition(ep, QTextCursor::KeepAnchor);
                } else {
                    return true;
                }
            } else {
                LPCC ty=AS(1);
                c.movePosition(getTypeMoveOperation(ty), QTextCursor::KeepAnchor);
                // c.movePosition(getTypeMoveOperation("end"), QTextCursor::KeepAnchor);
            }
            rst->set( Q2A(c.selectedText()), -2 );
        } else if( checkFuncObject(arrs->get(0),'i',2) ) {
            StrVar* sv=arrs->get(0);
            int sp = sv->getInt(4), ep=sv->getInt(8);
            c.setPosition(sp);
            if( sp<ep ) {
                c.setPosition(ep, QTextCursor::KeepAnchor);
            } else {
                return false;
            }
            rst->set( Q2A(c.selectedText()), -2 );
        } else {
            bool ok=false;
            if( cnt==1 ) {
                LPCC ty=toString(arrs->get(0));
                if( ccmp(ty,"word") ) {
                    c.select(QTextCursor::WordUnderCursor);
                    if( isVarTrue(arrs->get(1)) ) {
                        w->setTextCursor(c);
                    }
                    tn->GetVar("@movePos")->setVar('0',0).addInt(c.selectionEnd());
                } else if( ccmp(ty,"select") ) {
                    if( c.hasSelection() ) {
                        rst->set(Q2A(c.selectedText()),-2);
                    }
                } else {
                    c.movePosition(getTypeMoveOperation(ty), QTextCursor::KeepAnchor);
                }
            } else {                
                StrVar* sv=arrs->get(cnt-1);
                if( SVCHK('3',1) ) {
                    ok=true;
                    cnt-=1;
                }
                int last=cnt-1; //, end=textEndPosition( w->document() );
                for( int n=0; n<cnt; n++ ) {
                    sv=arrs->get(n);
                    if( isNumberVar(sv) ) {
                        c.setPosition( toInteger(sv), n==0 || n<last ? QTextCursor::MoveAnchor: QTextCursor::KeepAnchor);
                    } else {
                        LPCC ty=toString(sv);
                        c.movePosition(getTypeMoveOperation(ty), n==0 || n<last ? QTextCursor::MoveAnchor: QTextCursor::KeepAnchor);
                    }
                }
            }
            rst->set(Q2A(c.selectedText()),-2);
            if( ok ) {
                w->setTextCursor(c);
            }
        }
    } break;
    case 811: {	// value
        int cnt = arrs ? arrs->size(): 0;
        if( cnt==0 ) {
            rst->set( Q2A(w->toPlainText()) );
        } else if( cnt==1 ) {
            w->setPlainText(KR(AS(0)) );
            rst->setVar('n',0,(LPVOID)tn);
        }

    } break;
    case 15: {	// append
        if( cnt==0 ) {
            return true;
        }
        LPCC txt=AS(0);        
        if( slen(txt) ) {
            StrVar* sv=arrs->get(2);
            if( SVCHK('3',1) ) {
                QTextCursor c = w->textCursor();
                if( isVarTrue(arrs->get(1)) ) {
                    w->xscrollLock=true;
                }
                w->append(KR(txt));
                if( w->xscrollLock ) {
                    w->xscrollLock=false;
                }
            } else {
                /*
                bool isBottom = false;
                if( textEndPosition(w->document()) == w->textCursor().position() ) {
                    isBottom=true;
                }
                */
                QTextCursor c = QTextCursor(w->document());
                QScrollBar* svar = w->verticalScrollBar();                
                c.movePosition(QTextCursor::End);
                tn->setInt("@insertPostion", c.position());
                c.insertText(KR(txt));
                sv=arrs->get(1);
                if( SVCHK('3',1) ) {
                    c.insertBlock();
                    svar->setValue(svar->maximum());
                } else {
                    bool isBottom=(svar->value() == svar->maximum());
                    if(isBottom) svar->setValue(svar->maximum());
                }
            }
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 16: {	// select
        QTextCursor c = w->textCursor();
        if( arrs ) {
            if( isNumberVar(arrs->get(0)) ) {
                c.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
                // int end=c.position();
                int end=textEndPosition( w->document() );
                if( isNumberVar(arrs->get(1)) ) {
                    int sp=toInteger(arrs->get(0)), ep=toInteger(arrs->get(1));
                    if( sp<0 ) sp=0;
                    if( ep> end ) ep=end;
                    if( sp==ep ) {
                        c.setPosition(sp, QTextCursor::MoveAnchor);
                        w->setTextCursor(c);
                    } else if( sp<ep) {
                        c.setPosition(sp, QTextCursor::MoveAnchor);
                        c.setPosition(ep, QTextCursor::KeepAnchor);
                        w->setTextCursor(c);
                    } else {
                        if(sp>end) sp=end;
                        if(ep<0) ep=0;
                        c.setPosition(sp, QTextCursor::MoveAnchor);
                        c.setPosition(ep, QTextCursor::KeepAnchor);
                        w->setTextCursor(c);
                    }
                } else if( arrs->get(1) ) {
                    LPCC txt=AS(0);
                    if( ccmp(txt,"word") ) {
                        int sp=toInteger(arrs->get(0));
                        if( sp<end ) {
                            c.setPosition(sp, QTextCursor::MoveAnchor);
                        }
                        c.select(QTextCursor::WordUnderCursor);
                        w->setTextCursor(c);
                    }
                }
            } else {
                LPCC txt=AS(0);
                if( ccmp(txt,"word") ) {
                    c.select(QTextCursor::WordUnderCursor);
                    w->setTextCursor(c);
                }
            }
        } else {
            c.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
            c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            w->setTextCursor(c);
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 22: {	// syntax
        if( cnt==0 ) {
            rst->reuse()->add( tn->gv("@syntax") );
            if( w->xsyntax ) {
                w->xsyntax->rehighlight();
            }
            return true;
        }
        bool reload=true, removeAll=false;
        int cnt = arrs->size();
        StrVar* sv=arrs->get(1);
        if( SVCHK('3',1) ) {
            if( w->xsyntax ) {
                tn->setInt("@blockIdx",1);
                for( int n=0,sz=w->xsyntax->xfmts.size(); n<sz; n++ ) {
                    XHighlightData* hd=w->xsyntax->xfmts.getvalue(n);
                    SAFE_DELETE(hd);
                }
                w->xsyntax->xfmts.reuse();
                for( int n=0,sz=w->xsyntax->xblockFmts.size(); n<sz; n++ ) {
                    XHighlightData* hd=w->xsyntax->xblockFmts.getvalue(n);
                    SAFE_DELETE(hd);
                }
                w->xsyntax->xblockFmts.reuse();
                removeAll=true;
            }
        }
        TreeNode* cur=NULL;
        sv=arrs->get(0);
        if( SVCHK('n',0) ) {
            TreeNode* syntaxNode=(TreeNode*)SVO;
            sv=tn->gv("@syntax");
            if( SVCHK('n',0) ) {
                cur=(TreeNode*)SVO;
                if( syntaxNode==cur ) {
                    reload=removeAll ? true : false;;
                }
            }
            cur=syntaxNode;
            // deleteAllTreeNode(cur);
        } else if( sv ) {
            if( sv->charAt(0)=='@' ) {
                LPCC val=toString(sv);
                if( w->xsyntax->xstringHighlightData ) {
                    if( ccmp(val,"@string.multiLine") ) {
                        w->xsyntax->xstringHighlightData->xtype=52;
                    } else if( ccmp(val,"@string.singleLine") ) {
                        w->xsyntax->xstringHighlightData->xtype=51;
                    }
                }
                w->xsyntax->rehighlight();
                return true;
            }
            int sp, ep;
            StrVar* syntaxData=getStrVarPointer(sv, &sp, &ep);
            sv=tn->gv("@syntax");
            if( SVCHK('n',0) ) {
                cur=(TreeNode*)SVO;
                reload=removeAll ? true : false;
            } else {
                cur =new TreeNode(2, true);
            }
            cur->setJson(syntaxData, sp, ep);
        }
        if( cur ) {
            for( WBoxNode* bn=cur->First(); bn ;  bn = cur->Next() ) {
                TreeNode* sub=NULL;
                sv=&bn->value;
                if( SVCHK('n',0) ) {
                    sub=(TreeNode*)SVO;
                }
                if( sub==NULL ) {
                    continue;
                }
                if( sub->isNodeFlag(FLAG_CALL) && !reload ) {
                    continue;
                }
                LPCC syntaxCode=cur->getCurCode();
                w->setCharFormat(syntaxCode, sub);
                sub->setNodeFlag(FLAG_CALL);
            }
            QTextCharFormat fmt;
            fmt.setForeground(QBrush(QColor(70,128,56,255)));
            XHighlightData* hd = new XHighlightData(fmt,10);
            hd->exp.setPattern(KR("// [^\\n]*"));
            if( w->xsyntax ) {
                w->xsyntax->xfmts.add(hd);
            }
            w->xsyntax->rehighlight();
            tn->GetVar("@syntax")->setVar('n',0,(LPVOID)cur);
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 24: {	// insert
        QTextCursor c = w->textCursor(); //SVCHK('c',11)? static_cast<QTextCursor>(*((QTextCursor*)SVO)) :
        if( cnt==0 ) {
            c.removeSelectedText();
            w->setTextCursor(c);
            return false;
        }
        bool ok=true;
        int cp = c.hasSelection() ? c.selectionStart(): c.position();
        if( isVarTrue(arrs->get(1)) ) {
            int sp=0,ep=0,fp=-1;
            StrVar* var=getStrVarPointer(arrs->get(0), &sp, &ep);
            XParseVar pv(var,sp,ep);
            if( pv.find("^|",NULL,FIND_FORWORD)!=-1 ) {
                XListArr* arr=tn->addArray("@tapPositions",true);
                LPCC left=NULL;
                XListArr* temp=getListArrTemp();
                QString txt = "";
                tn->setInt("tabPositionsIndex",0);
                int n=0;
                for(; pv.valid() && n<16; n++ ) {
                    pv.findEnd("^|",FIND_FORWORD);
                    left = pv.left();
                    txt+=KR(left);
                    sp=cp+txt.length();
                    if( n==0 )
                        fp=sp;
                    temp->add()->setVar('i',2).addInt(sp).addInt(sp);
                }
                int sz=temp->size();
                tn->setInt("@insertPostion", c.position());
                c.insertText(txt);
                if( fp!=-1 ) {
                    if(sz<3) {
                        sz=0;
                    } else {
                        tn->setInt("tabPositionsIndex",1);
                    }
                    c.setPosition(fp, QTextCursor::MoveAnchor);
                }

                if( sz>1 ) {
                    for(int n=0;n<sz;n++) {
                        arr->add()->add(temp->get(n));
                    }
                    // gtrees.removeArr(temp);
                }
                ok=false;
            }
        }
        if(ok) {
            LPCC str = toString(arrs->get(0));
            tn->setInt("@insertPostion", c.position());
            c.insertText(KR(str));
        }
        if( isVarTrue(arrs->get(2)) ) {
            c.setPosition(cp, QTextCursor::MoveAnchor);
        }
        w->setTextCursor(c);
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 31: {	// sp
        StrVar* sv=NULL;
        QTextCursor c = w->textCursor();
        tn->setInt("@movePos", c.position() );
        int sp=0;
        if( cnt==0 ) {
            sp = c.position();
        } else {
            int cnt=arrs->size();
            sp = c.position();
            for( int n=0; n<cnt; n++ ) {
                sv=arrs->get(n);
                if( isNumberVar(sv) ) {
                    if( n==0 ) {
                        int num=toInteger(sv);
                        if( num<0 ) {
                            sp+=num;
                        } else {
                            sp=num;
                        }
                    } else {
                        sp+= toInteger(sv);
                    }
                    if( sp<0 ) sp=0;
                    c.setPosition(sp);
                } else {
                    int pos=editorMovePos(&c, toString(sv), tn);
                    c.setPosition(pos);
                    sp = pos;
                }
            }
        }
        if( sp<0 ) sp = 0;
        tn->GetVar("@sp")->setVar('0',0).addInt(sp);
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 32: {	// spText
        StrVar* sv=NULL; // tn->gv("@cursor");
        QTextCursor c = w->textCursor();	// SVCHK('c',2)? static_cast<QTextCursor>(*((QTextCursor*)SVO)) :
        int sp=-1, ep=0;
        int cp=c.position();
        sv = tn->gv("@sp");
        if( SVCHK('0',0) ) {
            sp = ep = toInteger(sv);
        }

        if( cnt==0 ) {
            ep=cp;
        } else {
            int cnt=arrs->size();
            if( sp==-1 ) sp=c.position();
            // c.movePosition(getTypeMoveOperation("end"), QTextCursor::MoveAnchor);
            int end = textEndPosition( w->document() );
            c.setPosition(min(sp, end), QTextCursor::MoveAnchor);
            for( int n=0; n<cnt; n++ ) {
                sv=arrs->get(n);
                if( isNumberVar(sv) ) {
                    ep+=toInteger(sv);
                    if( ep<0 )
                        ep=0;
                    else if( ep>end )
                        ep=end;
                    c.setPosition(ep);
                } else {
                    LPCC type=toString(sv);
                    ep=editorMovePos(&c, type, tn);
                    if( ep>end ) ep=end;
                    c.setPosition(ep);
                }
            }
        }

        if( sp!=ep ) {
            c.setPosition(sp, QTextCursor::MoveAnchor);
            c.setPosition(ep, QTextCursor::KeepAnchor);
            rst->set( Q2A(c.selectedText()), -2 );
        } else {
            // qDebug("spText (%d, %d)\n", sp, ep);
            rst->reuse();
        }
    } break;

    case 33: {	// cursorRect
        setVarRectF(rst, w->cursorRect());
    } break;
    case 34: {	// cursorPos
        int cnt = arrs? arrs->size(): 0;
        int x=0, y=0;
        if( cnt==0 ) return false;

        StrVar* sv =arrs->get(0);
        if( SVCHK('i',2) ) {
            x=sv->getInt(4), y=sv->getInt(8);
            sv =arrs->get(1);
        } else {
            x = toInteger(arrs->get(0)), y=toInteger(arrs->get(1));
            sv =arrs->get(2);
        }
        QPoint pt(x,y);
        QTextCursor c = w->cursorForPosition(pt);
        if( c.isNull() ) {
            rst->setVar('3',0);
        } else {
            if( SVCHK('f',1) ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                XFuncNode* fnCur=setFuncNodeParent(fsrc, fn);
                fnCur->GetVar("cursor")->setVar('c',2,(LPVOID)&c);
                fnCur->call();
                fnCur->clearNodeFlag(FLAG_PERSIST);
                funcNodeDelete(fnCur);
                // gfns.deleteFuncNode(fnCur);
            }
            rst->setVar('0',0).addInt(c.position());
        }
    } break;

    case 41: {	// findAll
        XListArr* a = tn->addArray("@finds");
        StrVar* sv = tn->gv("@markTick");
        if( sv ) {
            long dist= GetTickCount() - (long)toUL64(sv);
            if( dist < 800 ) {
                // qDebug("findAll cancle (dist:%d)\n",dist);
                return true;
            }
        }
        a->reuse();
        if( cnt==0 ) {
            w->textCursorPositionChanged();
            return true;
        }
        int skipPos=-1;
        sv = tn->gv("@mouseDownTick");
        if( sv ) {
            long dist= GetTickCount()-(long)toUL64(sv);
            if( dist > 100 ) {
                skipPos=toInteger(tn->gv("@movePos"));
            }
        } else {
            skipPos=toInteger(tn->gv("@movePos"));
        }
        LPCC s = AS(0);
        QString str = KR(s);
        QTextCursor c= w->textCursor();
        tn->GetVar("@movePos")->setVar('0',0).addInt(c.position() );

        int sp=c.position(), ep=-1;
        QTextDocument::FindFlags ff = QTextDocument::FindWholeWords | QTextDocument::FindCaseSensitively;
        sv=arrs->get(1);
        if( isNumberVar(sv) ) {
            // startPos, endPos, skipPos
            sp=toInteger(sv);
            c.setPosition(sp);
            if( isNumberVar(arrs->get(2)) ) {
                ep=toInteger(arrs->get(2));
                if( sp>=ep ) {
                    return true;
                }
                sv=arrs->get(3);
                if( isNumberVar(sv) ) {
                    skipPos=toInteger(sv);
                }
            }
        } else {
            c.movePosition(getTypeMoveOperation("start"), QTextCursor::MoveAnchor);
        }
        // version 1.0.2
        int pos=0;
        while( !c.isNull() && !c.atEnd() ) {
            c = w->document()->find(str, c, ff);
            if( c.isNull() ) break;
            pos=c.position();
            if( skipPos!=-1 && pos==skipPos ) {
                continue;
            }
            if( ep!=-1 ) {
                if(ep<=pos ) break;
            }
            a->add()->setVar('i',2).addInt(c.selectionStart()).addInt(c.selectionEnd());
        }
        w->textCursorPositionChanged();
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 42: {	// insertImage
        // arrs
        QTextCursor c=w->textCursor();
        editorInsertImage(w->document(), c, arrs, rst );
    } break;
    case 43: {	// hitTest
        if( cnt==0 ) return false;
        sv=arrs->get(0);
        if( SVCHK('i',2) ) {
            QPoint pt(sv->getInt(4), sv->getInt(8));
            int pos = w->document()->documentLayout()->hitTest(pt, Qt::ExactHit);
            if(pos < 0) break;

            QTextCursor cursor(w->document());
            cursor.setPosition(pos);
            if(cursor.atEnd()) break;
            cursor.setPosition(pos+1);

            QTextFormat format = cursor.charFormat();
            if( format.isImageFormat() ) {
                QString image = format.toImageFormat().name();
                rst->set("image: ").add(Q2A(image));
            } else {
                QString link=w->anchorAt(pt);
                rst->set(Q2A(link));
            }
        }
    } break;
    case 44: {	// mark
        if( cnt==0 ) {
            return true;
        }
        XListArr* markArr=tn->addArray("@mark");
        if( cnt==0 ) {
            markArr->reuse();
            w->textCursorPositionChanged();
            return true;
        }
        markArr->reuse();
        sv=arrs->get(0);
        if( SVCHK('a',0) ) {
            XListArr* a=(XListArr*)SVO;
            for( int n=0; n<a->size(); n++ ) {
                markArr->add()->add( a->get(n) );
            }
        } else if( SVCHK('i',2) ) {
            markArr->add()->add(sv);
        } else if( isNumberVar(sv) ) {
            int sp=toInteger(sv), ep=toInteger(arrs->get(1));
            if( sp < ep ) {
                markArr->add()->setVar('i',2).addInt(sp).addInt(ep);
            }
        }
        tn->GetVar("@markTick")->setVar('1',0).addUL64(GetTickCount());
        w->viewport()->update();
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 45: {	// clear
        w->clear();
        if( arrs && isVarTrue(arrs->get(0)) ) {
            w->document()->clearUndoRedoStacks();
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 50: {	// toSyntax
        QString str;
        w->xsyntax->asHtml(str);
        rst->add(Q2A(str));
    } break;
    case 52: {	// anchorAt
        if( cnt==0 ) return false;
        StrVar* sv=arrs->get(0);
        rst->reuse();
        if( SVCHK('i',2) ) {
            QPoint pt(sv->getInt(4), sv->getInt(8) );
            QString value=w->anchorAt(pt);
            if( !value.isEmpty() ) {
                rst->add(Q2A(value));
            }
        }
    } break;
    case 53: {	// setCursor
        rst->reuse();
        if( cnt==0 ) {
            rst->setVar('0',0).addInt((int)w->viewport()->cursor().shape());
        } else if( isNumberVar(arrs->get(0)) ) {
            int num = toInteger(arrs->get(0));
            w->viewport()->setCursor( QCursor((Qt::CursorShape)num) );
            rst->setVar('n',0,(LPVOID)tn);
        }
    } break;
    case 54: {	// zoomIn
        int num= cnt==0 ? 2: toInteger(arrs->get(0));
        w->zoomIn(num);
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 55: {	// zoomOut
        int num= cnt==0 ? 2: toInteger(arrs->get(0));
        w->zoomOut(num);
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 56: {	// lineHide
        if(cnt==0) return true;
        if( isNumberVar(arrs->get(0)) ) {
            int num=toInteger(arrs->get(0));
            QTextBlock block=w->document()->findBlockByLineNumber(num);
            if( block.isValid() ) {
                bool visible=cnt==1? false: !isVarTrue(arrs->get(1));
                block.setVisible(visible);
            }
        }
    } break;
    case 57: {	// lineNumber
        if( cnt==0 ) {
            QTextBlock block=w->document()->findBlock(w->textCursor().position());
            if( block.isValid() ) {
                rst->setVar('0',0).addInt(block.blockNumber());
            }
        } else if( isNumberVar(arrs->get(0)) ) {
            QTextBlock block=w->document()->findBlock(toInteger(arrs->get(0)));
            if( block.isValid() ) {
                rst->setVar('0',0).addInt(block.blockNumber());
            }
        } else {
            LPCC ty=AS(0);
            if(ccmp(ty,"start")) {
                if(w->mStartBlock.isValid()) {
                    rst->setVar('0',0).addInt(w->mStartBlock.blockNumber());
                }
            }
            else if(ccmp(ty,"end")) {
                if(w->mEndBlock.isValid()) {
                    rst->setVar('0',0).addInt(w->mEndBlock.blockNumber());
                }
            }
            else if(ccmp(ty,"next")) {
                QTextBlock block=w->mStartBlock.next();
                if(block.isValid()) {
                    w->mStartBlock=block;
                    rst->setVar('0',0).addInt(block.blockNumber());
                }
            }
            else if(ccmp(ty,"prev")) {
                QTextBlock block=w->mStartBlock.previous();
                if(block.isValid()) {
                    w->mStartBlock=block;
                    rst->setVar('0',0).addInt(block.blockNumber());
                }
            }
        }

    } break;
    case 58: {	// lineRect
        int pos=0;
        if( cnt==0 ) {
            pos=w->textCursor().position();
        } else if( isNumberVar(arrs->get(0)) ) {
            pos=toInteger(arrs->get(0));
        }
        QTextBlock block=w->document()->findBlock(pos);
        if( block.isValid() ) {
            if(cnt==2) {
                QRectF rc = w->getBlockRect(block);
                QTextLayout *layout = block.layout();
                int xpos=toInteger(arrs->get(1));
                qreal cx=layout->lineForTextPosition(xpos).cursorToX(xpos);
                rc.setX(rc.x()+cx);
                setVarRectF(rst, rc);
            } else {
                setVarRectF(rst, w->getBlockRect(block));
            }
        }
    } break;
    case 60: {	// lineNumWidth
        if( w->xlinenum && w->xlinenum->isVisible() ) {
            rst->setVar('0',0).addInt( w->xlinenum->width() );
        } else {
            rst->setVar('0',0).addInt( 0 );
        }
    } break;
    case 61: {	// insertCursor
        QTextCursor c=w->textCursor();
        StrVar* sv=arrs->get(0);
        if( SVCHK('f',1) ) {
            int sp=0, ep=0;
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc, fn);
            QTextCharFormat fmt=c.charFormat();
            fnCur->GetVar("@this")->setVar('n',0, (LPVOID)tn);
            fnCur->GetVar("cursor")->setVar('c',2,(LPVOID)&c);
            if( arrs->get(1) ) {
                sv = getStrVarPointer(arrs->get(1),&sp,&ep);
                fnCur->GetVar("data")->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(ep);
            } else {
                fnCur->GetVar("data")->reuse();
            }
            c.beginEditBlock();
            fnCur->call();
            c.endEditBlock();
            c.setCharFormat(fmt);
            w->setTextCursor(c);
            funcNodeDelete(fnCur);
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;    
    case 62: {	// insertHtml
        if( cnt==0 ) return true;
        LPCC html=AS(0);
        if( slen(html)>0) {
            QTextCursor c = w->textCursor();
            QTextCharFormat fmt=c.charFormat();
            c.insertHtml(KR(html));
            c.setCharFormat(fmt);
            w->setTextCursor(c);
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 63: {	// scrollTop
        if( cnt==0 ) {
            return true;
        }
        if( isNumberVar(arrs->get(0)) ) {
            int pos=toInteger(arrs->get(0));
            QTextBlock block=w->document()->findBlock(pos);
            if( block.isValid() ) {
                w->blockScrollTop(block);
            }
        }
        rst->setVar('n',0,(LPVOID)tn);
        // w->setScrollTop();
    } break;
    case 64: {	// scrollPos
        if( cnt==0 ) {
            rst->setVar('0',0).addInt(w->verticalScrollBar()->sliderPosition());
            return true;
        }
        if( isNumberVar(arrs->get(0)) ) {
            int ypos=toInteger(arrs->get(0));
            int xpos=-1;
            if( isNumberVar(arrs->get(1)) ) {
                xpos=toInteger(arrs->get(1));
            }
            if( xpos==-1 ) {
                w->verticalScrollBar()->setSliderPosition(ypos);
            } else {
                w->setScrollPosition(ypos, xpos);
            }
        } else {
            LPCC type=AS(0);
            if( ccmp(type,"end") ) {
                int max=w->verticalScrollBar()->maximum();
                w->verticalScrollBar()->setSliderPosition(max);
            } else if( ccmp(type,"point") ) {
                rst->setVar('i',2).addInt(w->horizontalScrollBar()->sliderPosition()).addInt(w->verticalScrollBar()->sliderPosition());
            }
        }
    } break;
    case 65: {	// frameMargin
         if( cnt==0 ) {
            setVarRect( rst, w->frameGeometry() );
            return true;
        }
        if( isNumberVar(arrs->get(0)) ) {
            int left=toInteger(arrs->get(0));
            int top=toInteger(arrs->get(1));
            int right=toInteger(arrs->get(2));
            int bottom=toInteger(arrs->get(3));
            w->setUndoRedoEnabled(false);
            w->setFrameMargin(left, top, right, bottom);
            w->setUndoRedoEnabled(true);
        }
    } break;

    case 66: {	// update
        w->update();
        w->viewport()->update();
        w->repaint();
    } break;
    case 67: {	// css
        LPCC sty=  cnt==0 ? "a { text-decoration:none; }": AS(0);
        w->document()->setDefaultStyleSheet(KR(sty));
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 68: {	// clearUndo
        w->document()->clearUndoRedoStacks();
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 69: {	// htmlPaste
        if(cnt==0) {
            tn->setBool("pasteStyleUse", true);
        } else {
            tn->setBool("pasteStyleUse", isVarTrue(arrs->get(0)));
        }
    } break;
    case 70: {	// mergeFormat
        if( cnt==0 ) return true;
        QTextCursor cursor(w->document());
        QTextCharFormat fmt;
        textCharFormatApply(fmt, arrs->get(0));
        cursor.mergeCharFormat(fmt);
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 71: {	// html
        if( arrs==NULL ) {
            QString html=w->toHtml();
            rst->set(Q2A(html));
        } else {
            LPCC htmlValue=AS(0);
            w->setHtml( KR(htmlValue) );
        }
    } break;

    case 72: {	// linePos
        int lineNum=w->document()->blockCount();
        if( arrs==NULL ) {
            rst->setVar('0',0).addInt(lineNum );
            return true;
        }
        int num=1;
        if( isNumberVar(arrs->get(0)) ) {
            num=toInteger(arrs->get(0));
            if( num>lineNum ) num=lineNum;
        }
        int pos=0;
        QTextBlock block=w->document()->findBlockByLineNumber(num);
        if( block.isValid() ) {
            pos=block.position();
        }
        rst->setVar('0',0).addInt(pos);
    } break;
    case 73: {	// copy
        QTextCursor c= w->textCursor();
        int pos=c.position();
        if( arrs==NULL ) {
            if( !c.hasSelection() ) {
                w->selectAll();
            }
            w->copy();
        } else {
            sv=arrs->get(0);
            if( isNumberVar(sv) ) {
                // c.movePosition(getTypeMoveOperation("end"), QTextCursor::MoveAnchor);
                int sp=toInteger(sv), end=textEndPosition( w->document() ), ep=end;
                sv=arrs->get(1);
                if( isNumberVar(sv) ) {
                    ep=toInteger(sv);
                }
                if( ep>end ) ep=end;
                if( sp<ep ) {
                    c.setPosition(sp);
                    c.setPosition(ep,QTextCursor::KeepAnchor);
                    w->setTextCursor(c);
                }
            }
            if( c.hasSelection() ) {
                w->copy();
            }
        }
        c.setPosition(pos);
        w->setTextCursor(c);
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 74: {	// paste        
        if( arrs==NULL ) {
            w->paste();
        } else {
            sv=arrs->get(0);
            if( isNumberVar(sv) ) {
                QTextCursor c= w->textCursor();
                int pos=toInteger(sv);
                c.setPosition(pos);
                w->setTextCursor(c);
                w->paste();
            }
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 171: {	// pos
        /*
        static long prevPosTick=0;
        if( GetTickCount()-prevPosTick > 100 ) {
            tn->GetVar("@sel")->reuse();
        }
        */
        StrVar* sv = NULL; //tn->gv("@cursor");;
        QTextCursor c =  w->textCursor(); // SVCHK('c',2)? static_cast<QTextCursor>(*((QTextCursor*)SVO)) :
        int cnt = arrs ? arrs->size(): 0;
        if( cnt==0 ) {
            rst->setVar('0',0).addInt(c.position());
            return true;
        }
        bool ok=false;
        sv=arrs->get(cnt-1);
        if( SVCHK('3',1) ) {
            ok=true;
            cnt-=1;
        }
        for( int n=0;n<cnt;n++ ) {
            sv=arrs->get(n);
            if( isNumberVar(sv) ) {
                int sp = toInteger(sv);
                if( cnt==1 ) {
                    if( sp<0 ) sp=0;
                    // c.movePosition(getTypeMoveOperation("end"), QTextCursor::MoveAnchor);
                    int end=textEndPosition( w->document() );
                    c.setPosition(min(sp,end), QTextCursor::MoveAnchor );
                    w->setTextCursor(c);
                    rst->setVar('n', 0, (LPVOID)tn);
                    return true;
                } else {
                    if( n==0 ) {
                        c.setPosition(sp, QTextCursor::MoveAnchor );
                    } else {
                        int pos=c.position();
                        sp+=pos;
                        if( sp<0 ) sp=0;
                        c.setPosition(sp, QTextCursor::MoveAnchor );
                    }
                }
            } else {
                int pos=editorMovePos(&c, toString(sv), tn);
                c.setPosition(pos, QTextCursor::MoveAnchor);
            }
        }
        if( ok ) {
            w->setTextCursor(c);
        }
        int pos=c.position();
        rst->setVar('0',0).addInt(pos);

        sv = tn->gv("@sp");
        if( sv ) sv->reuse();
        // prevPosTick=GetTickCount();
    } break;
    case 172: {	// movePos
        StrVar* sv=arrs->get(0);
        if( SVCHK('i',2) || SVCHK('i',4) ) {
            w->move(sv->getInt(4), sv->getInt(8));
            if( SVCHK('i',4) ) {
                w->resize(sv->getInt(12), sv->getInt(16));
            }
            return true;
        }
        // sv=tn->gv("@cursor");;
        QTextCursor c = w->textCursor(); // SVCHK('c',2)? static_cast<QTextCursor>(*((QTextCursor*)SVO)) :
        if( arrs==NULL ) {
            StrVar* sv = tn->gv("@movePos");
            if( SVCHK('0',0) ) {
                int num = toInteger(sv);
                c.setPosition(num, QTextCursor::MoveAnchor);
                w->setTextCursor(c);
            }
            return true;
        }
        bool iskeep = isVarTrue(arrs->get(1));
        if( isNumberVar(arrs->get(0))  ) {
            int pos = toInteger(arrs->get(0));
            int end=textEndPosition( w->document() );
            if( pos>end ) pos=end;
            //	 moveoper, iskeep, text
            c.setPosition(pos,iskeep? QTextCursor::KeepAnchor: QTextCursor::MoveAnchor );
            // c.movePosition((QTextCursor::MoveOperation)p, iskeep? QTextCursor::KeepAnchor: QTextCursor::MoveAnchor );
            // rst->setVar('0',0).addInt(c.position());
            // w->setTextCursor(c);
            LPCC txt = AS(2);
            if( slen(txt) ) {
                if( iskeep ) {
                    c.removeSelectedText();
                }
                c.insertText(KR(txt));
            }
            w->setTextCursor(c);
        } else {
            LPCC ty = AS(0);
            int pos= c.position(), ep=-1;
            ty = AS(0);
            if( ccmp(ty,"selectStart") ) {
                ep = c.hasSelection()? c.selectionStart(): pos;
            } else if( ccmp(ty,"selectEnd") ) {
                ep = c.hasSelection()? c.selectionEnd(): pos;
            } else if( ccmp(ty,"cur") ) {
                ep = pos;
            } else {
                c.movePosition(getTypeMoveOperation(ty), iskeep? QTextCursor::KeepAnchor: QTextCursor::MoveAnchor );
            }
            if( ep!=-1 ) {
                c.setPosition(ep,iskeep? QTextCursor::KeepAnchor: QTextCursor::MoveAnchor );
            }
            w->setTextCursor(c);
        }
        rst->setVar('n',0,(LPVOID)tn);
    } break;
    case 173: {	// is lineNumber : NF_LINENUM, enterKey: NF_TABUSE,
        if( arrs==NULL ) {
            rst->set("visible, disable, enable, hidden, active, full, max, min, model, topleval\n\
                     empty, redo, undow, modify, readonly, link, findAll, select, htmlPaste\n\
                     paste, tabWidth, lineHeight, lineNumberUse, search, wrapUse, contextMenu");
            return true;
        }
        LPCC ty=AS(0);
        if( isWidgetCheck(ty, w, rst) ) return true;
        int cnt = arrs->size();

        StrVar* sv=arrs->get(1);
        if( ccmp(ty,"empty") ) rst->setVar('3', w->document()->isEmpty()? 1:0 );
        else if( ccmp(ty,"redo") ) {
            rst->setVar('3', w->document()->isRedoAvailable()? 1:0 );
            if( SVCHK('3',1) ) w->redo();
        }
        else if( ccmp(ty,"undo") ) {
            rst->setVar('3', w->document()->isUndoAvailable()? 1:0 );
            if( SVCHK('3',1) ) w->undo();
        }
        else if( ccmp(ty,"modify") ) {
            if( sv ) {
                w->document()->setModified( isVarTrue(sv) );
            }
            rst->setVar('3', w->document()->isModified()? 1:0 );
        } else if( ccmpl(ty,"readonly") ) {
            if( sv==NULL ) {
                rst->setVar('3', w->isReadOnly()? 1: 0);
            } else {
                w->setReadOnly(isVarTrue(sv));
            }
        } else if( ccmpl(ty,"htmlPaste") ) {
            if( sv==NULL ) {
                rst->setVar('3', w->acceptRichText()? 1: 0);
            } else {
                w->setAcceptRichText( isVarTrue(sv) );
            }
        } else if( ccmp(ty,"link") ) {
            QTextCursor c=w->textCursor();
            int sp=0, ep=0;
            if( sv==NULL ) {
                // c.movePosition(getTypeMoveOperation("end"), QTextCursor::MoveAnchor);
                ep=textEndPosition( w->document() );
            } else {
                if( isNumberVar(sv) ) {
                    sp=toInteger(sv), ep=toInteger(arrs->get(2));
                    if( sp>=ep ) {
                        ep=textEndPosition( w->document() );
                    }
                }
            }
            if( sp<ep ) {
                c.setPosition(sp);
                QTextBlock block=c.block();
                while( block.isValid()) {
                    int bsp=block.position();
                    if( ep <= bsp) break;
                    for (QTextBlock::iterator it = block.begin(); !(it.atEnd()); ++it ) {
                        QTextFragment fragment = it.fragment();
                        if( !fragment.isValid() ) break;
                        QTextCharFormat fmt = it.fragment().charFormat();
                        if( fmt.isAnchor() || fmt.isImageFormat() ) {
                            rst->setVar('3',1);
                            return true;
                        }
                    }
                    block=block.next();
                }
            }
        } else if( ccmp(ty,"findAll") ) {
            XListArr* a=tn->addArray("@finds");
            if( isVarTrue(sv) ) {
                a->reuse();
            }
            rst->setVar('3', a->size()? 1: 0 );
        } else if( ccmp(ty,"select") ) {
            QTextCursor c= w->textCursor();
            rst->setVar('3',c.hasSelection()?1:0);
        } else if( ccmp(ty,"paste") ) {
            rst->setVar('3',w->canPaste() ? 1: 0);
        } else if( ccmp(ty,"search") ) {
            if( sv ) {
                XParseVar pv(sv);
                U32 flag = cnt==3? tn->getU32("@searchflag"): 0;
                while( pv.valid() ) {
                    ty=pv.findEnd(",").v();
                    if( ccmpl(ty,"wholeword") )		{ flag|=EF_WHOLEWORD; }
                    else if( ccmpl(ty,"case") )		{ flag|=EF_CASE; }
                    else if( ccmpl(ty,"regexp") )	{ flag|=EF_REGEXP; }
                    else if( ccmpl(ty,"mark") )		{ flag|=EF_MARK; }
                }
                tn->setU32("@searchflag", flag);
            } else {
                U32 flag=tn->getU32("@searchflag");
                rst->setVar('0',1).addU32(flag);
            }
        } else if( ccmp(ty,"tabWidth") ) {
            if( sv ) {
                if( isNumberVar(sv) ) {
                    int wid=toInteger(sv);
                    if( wid>0 ) w->setTabStopWidth(wid);
                }
            } else {
                rst->setVar('0',0).addInt(w->tabStopWidth());
            }
        } else if( ccmp(ty,"lineHighlight") ) {
            if( sv ) {
                if( isVarTrue(sv) )
                    tn->setNodeFlag(NF_HIGHTLIGHT);
                else
                    tn->clearNodeFlag(NF_HIGHTLIGHT);
                w->update();
            } else {
                rst->setVar('3', tn->isNodeFlag(NF_HIGHTLIGHT)? 1:0 );
            }

        } else if( ccmp(ty,"lineNumberUse") ) {
            if( sv ) {
                if( isVarTrue(sv) )
                    tn->setNodeFlag(NF_LINENUM);
                else
                    tn->clearNodeFlag(NF_LINENUM);
                w->update();
                w->updateLineNumberAreaWidth(0);
            } else {
                rst->setVar('3', tn->isNodeFlag(NF_LINENUM)? 1: 0);
            }
        } else if( ccmp(ty,"tabKeyUse") ) {
            if( sv ) {
                if( isVarTrue(sv) )
                    tn->setNodeFlag(NF_TABUSE);
                else
                    tn->clearNodeFlag(NF_TABUSE);
            } else {
                rst->setVar('3', tn->isNodeFlag(NF_TABUSE)? 1: 0);
            }
        } else if( ccmp(ty,"wrapUse") ) {
            if( sv ) {
                w->setLineWrapMode(isVarTrue(sv)? QTextEdit::WidgetWidth : QTextEdit::NoWrap);
            } else {
                rst->setVar('3', w->lineWrapMode()==QTextEdit::NoWrap? 0: 1);
            }
        } else if( ccmp(ty,"contextMenu") ) {
            if( sv ) {
                LPCC ty = AS(1);
                if( ccmp(ty,"custom") ) {
                    w->setContextMenuPolicy(Qt::CustomContextMenu);
                } else if( ccmp(ty,"default") ) {
                    w->setContextMenuPolicy(Qt::DefaultContextMenu);
                } else if( ccmp(ty,"action") ) {
                    w->setContextMenuPolicy(Qt::ActionsContextMenu);
                } else {
                    w->setContextMenuPolicy(Qt::NoContextMenu);
                }
            }
        } else {
            return false;
        }
    } break;
    case 175:{ // deletePoint
        if( arrs==NULL ) return true;
        StrVar* sv=arrs->get(0);
        LPCC type=AS(1), val=AS(2);
        int lenVal=slen(val);
        int flag=ccmp(type,"lineStart")? 1: ccmp(type,"lineEnd")? 2: ccmp(type,"line")? 3: 0;
        QString text;
        if( lenVal ) text=KR(val);

        QTextCursor cursor=w->textCursor(), c=cursor;
        if( SVCHK('a',0) ) {
            XListArr* deleteArr=(XListArr*)SVO;
            bubbleSort(deleteArr, 1);
            cursor.beginEditBlock();
            int sp=0, ep=0;
            int sz=deleteArr->size();
            for( int n=sz-1; n>=0; n-- ) {
                sv=deleteArr->get(n);
                if( SVCHK('i',2) ) {
                    sp=sv->getInt(4), ep=sv->getInt(8);
                    if( flag ) {
                        if( flag&1 ) {
                            c.setPosition(sp);
                            c.movePosition(getTypeMoveOperation("lineStart"), QTextCursor::MoveAnchor);
                            qDebug("xxxxxxxxxxx %d, %d\n", sp, c.position() );
                            if( sp!=c.position() ) sp=c.position();
                        } else if( flag&2 ) {
                            c.setPosition(ep);
                            c.movePosition(getTypeMoveOperation("lineEnd"), QTextCursor::MoveAnchor);
                            if( ep!=c.position() ) ep=c.position();
                        }
                    }
                    cursor.setPosition(sp);
                    cursor.setPosition(ep,QTextCursor::KeepAnchor);
                    cursor.insertText(text);
                }
            }
            cursor.endEditBlock();
        } else if( SVCHK('i',2) ) {
            int sp=sv->getInt(4), ep=sv->getInt(8);
            cursor.setPosition(sp);
            cursor.setPosition(ep,QTextCursor::KeepAnchor);
            cursor.insertText(text);
        }
    } break;
    case 176:{ // imageUrl
        QTextCursor c=w->textCursor();
        editorInsertImage(w->document(), c, arrs, rst, true );

    } break;
    case 177:{ // lineCount
        rst->setVar('0',0).addInt(w->document()->lineCount());
    } break;
    case 178:{ // align
        if( arrs==NULL ) {
            Qt::Alignment a=w->alignment();
            if (a & Qt::AlignLeft) {
                rst->set("left");
            } else if (a & Qt::AlignHCenter) {
                rst->set("center");
            } else if (a & Qt::AlignRight) {
                rst->set("right");
            } else if (a & Qt::AlignJustify) {
                rst->set("jsutify");
            }
            return true;
        }
        LPCC type=AS(0);
        if( ccmp(type,"left") ) {
            w->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
        } else if( ccmp(type,"right") ) {
            w->setAlignment(Qt::AlignHCenter );
        } else if( ccmp(type,"justify") ) {
            w->setAlignment(Qt::AlignJustify );
        } else if( ccmp(type,"center") ) {
            w->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
        }
    } break;
    case 179:{ // syntaxAt
        QTextCursor c=w->textCursor();
        int pos=-1;
        int sp=0, ep=0,endOfDocument=textEndPosition( w->document() );
        if( endOfDocument==0 ) return true;
        rst->reuse();
        if( arrs==NULL ) {
            QTextBlock last = w->document()->findBlock(endOfDocument);
            pos=c.position();
            c.movePosition(getTypeMoveOperation("lineStart"), QTextCursor::MoveAnchor);
            sp=c.position();
            if( c.block().position()==last.position() ) {
                ep=endOfDocument;
            } else {
                c.movePosition(getTypeMoveOperation("lineEnd"), QTextCursor::MoveAnchor);
                ep=c.position();
            }
        } else {
            StrVar* sv=arrs->get(0);
            if( isNumberVar(sv) ) {
                pos=toInteger(sv);
            } else {
                pos=c.position();
            }
            sv=arrs->get(1);
            if( isNumberVar(sv) ) {
                sp=toInteger(sv);
                sv=arrs->get(2);
                if( isNumberVar(sv) ) {
                    ep=toInteger(sv);
                }
            }
            if( sp<0 || sp>=endOfDocument ) {
                return true;
            }
            if( ep<=0 || ep>endOfDocument ) {
                // c.movePosition(getTypeMoveOperation("end"), QTextCursor::MoveAnchor);
                ep=endOfDocument;
            }
        }
        if( sp<ep ) {
            QTextCursor cursor(w->document());
            cursor.setPosition(sp);
            cursor.setPosition(ep, QTextCursor::KeepAnchor);
            /*
            QTextDocument* doc(new QTextDocument);
            QTextCursor dc(doc);
            dc.insertFragment(cursor.selection());
            dc.select(QTextCursor::Document);
            // Set the default foreground for the inserted characters.
            QTextCharFormat textfmt = dc.charFormat();
            textfmt.setForeground(Qt::gray);
            dc.setCharFormat(textfmt);
            */

            // Apply the additional formats set by the syntax highlighter
            QTextBlock start = w->document()->findBlock(cursor.selectionStart());
            QTextBlock end = w->document()->findBlock(cursor.selectionEnd());
            end = end.next();
            const int selectionStart = cursor.selectionStart();
            bool finish=false;
            for(QTextBlock current = start; current.isValid() ; current = current.next()) {
                const QTextLayout* layout(current.layout());
                foreach(const QTextLayout::FormatRange &range, layout->additionalFormats()) {
                    sp = current.position() + range.start; // - selectionStart;
                    ep = sp + range.length;
                    if( pos!=-1 ) {
                        if( pos<sp ) {
                            finish=true;
                            break;
                        }
                        if( pos>ep ) continue;
                    }
                    if( sp >= endOfDocument ) {
                        finish=true;
                        break;
                    }
                    QColor clr=range.format.foreground().color();
                    /*
                    dc.setPosition(qMax(sp, 0));
                    dc.setPosition(qMin(ep, endOfDocument), QTextCursor::KeepAnchor);
                    dc.setCharFormat(range.format);
                    */
                    if( rst->length() ) rst->add("\n");
                    rst->format(64,"%d,%d,", sp, ep);
                    rst->format(64,"#%02X%02X%02X", clr.red(), clr.green(), clr.blue() );
                    // qDebug()<<"start="<< start <<", end="<<end<<", text="<< dc.selectedText()<<", color"<< range.format.foreground().color()<<"\n";
                }
                if( finish ) break;
            }
            // delete doc;
        }
    } break;
    case 180:{ // highlight
        if( w->xsyntax ) {
            w->xsyntax->rehighlight();
            w->repaint();
        }
    } break;
    case 181:{ // makrdownHtml
        /*
        int sp=0,ep=0;
        if( arrs==NULL ) {
            sv=gvar.reuse();
            sv->set( Q2A(w->toPlainText()) );
            ep=sv->length();
        } else {
            sv=getStrVarPointer(arrs->get(0), &sp, &ep );
        }
        rst->reuse();
        if( sp< ep ) {
            int flags = MKD_TOC|MKD_SAFELINK|MKD_EXTRA_FOOTNOTE;
            char* result=NULL;
            int szdoc = 0;
            MMIOT *doc = mkd_string(sv->get(sp),ep-sp,flags);
            mkd_compile(doc,flags);
            szdoc = mkd_document(doc,&result );
            // mkd_generatehtml(doc,out);
            mkd_cleanup(doc);
            rst->set( result, szdoc );
        }
        */
    } break;

    default: return false;
    }
    return true;
}
bool callTextCursorFunc(LPCC fnm, XFuncNode* fn, XFunc* fc, StrVar* sv, PARR arrs, StrVar* rst) {
    QTextCursor* cursor=static_cast<QTextCursor*>(SVO);
    if( cursor==NULL ) return false;
    GTextEdit* editor=NULL;
    sv=fn->gv("@this");
    if( SVCHK('n',0) ) {
        TreeNode* node=(TreeNode*)SVO;
        editor=qobject_cast<GTextEdit*>(gwidget(node));
    }
    if( editor==NULL ) {
        return false;
    }
    if( fc->xfuncRef==0 ) {
        if( slen(fnm)==0 ) fnm=fc->getFuncName();
        fc->xfuncRef =
            ccmp(fnm,"insert") ? 1 :
            ccmp(fnm,"pos") ? 2 :
            ccmp(fnm,"is") ? 3 :
            ccmp(fnm,"format") ? 4 :
            ccmp(fnm,"align") ? 11 :
            ccmp(fnm,"select") ? 12 :
            ccmp(fnm,"insertList") ? 13 :
            ccmp(fnm,"text") ? 14 :
            ccmp(fnm,"frame") ? 5 :
            ccmp(fnm,"insertHtml") ? 6 :
            ccmp(fnm,"insertImage") ? 7 :
            ccmp(fnm,"valid") ? 90 : 0;
    }
    switch( fc->xfuncRef ) {
    case 1: {	// insert
        if( arrs==NULL ) {
            cursor->insertBlock();
            return true;
        }
        QTextDocument* doc= editor? editor->document(): NULL;
        if( doc ) {
            textCursorInsert(doc, cursor, AS(0), arrs->get(1), rst);
        }
    } break;
    case 2: {	// pos
        if( arrs==NULL ) {
            rst->setVar('0',0).addInt(cursor->position());
        } else if( isNumberVar(arrs->get(0)) ) {
            cursor->setPosition(toInteger(arrs->get(0)) );
        } else {
            LPCC ty=AS(0);
            cursor->movePosition(getTypeMoveOperation(ty), QTextCursor::MoveAnchor);
            rst->setVar('0',0).addInt(cursor->position());
        }
    } break;
    case 3: {	// is
        LPCC type=NULL;
        if( arrs==NULL ) {
            type="lineStart";
        } else {
            type=AS(0);
        }
        if( ccmp(type,"lineStart") ) rst->setVar('3', cursor->atBlockStart()? 1: 0);
        else if( ccmp(type,"lineEnd") ) rst->setVar('3', cursor->atBlockEnd()? 1: 0);
        else if( ccmp(type,"select") ) rst->setVar('3', cursor->hasSelection()? 1: 0);
        else rst->setVar('3',0);
    } break;
    case 4: {	// format
        QTextCharFormat fmt=cursor->charFormat();
        if( arrs==NULL ) {
            QString str=QString();
            textCharFormatText(fmt,str,rst );
        } else {
            textCharFormatApply(fmt, arrs->get(0));
        }
    } break;
    case 11: {	// align
        if( arrs==NULL ) {
            Qt::Alignment a=editor->alignment();
            if (a & Qt::AlignLeft) {
                rst->set("left");
            } else if (a & Qt::AlignHCenter) {
                rst->set("center");
            } else if (a & Qt::AlignRight) {
                rst->set("right");
            } else if (a & Qt::AlignJustify) {
                rst->set("jsutify");
            }
            return true;
        }
        LPCC type=AS(0);
        if( ccmp(type,"left") ) {
            editor->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
        } else if( ccmp(type,"right") ) {
            editor->setAlignment(Qt::AlignHCenter );
        } else if( ccmp(type,"justify") ) {
            editor->setAlignment(Qt::AlignJustify );
        } else if( ccmp(type,"center") ) {
            editor->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
        }
    } break;
    case 12: {	// select
        if( arrs==NULL ) {
            cursor->select(QTextCursor::WordUnderCursor);
        } else {
            if( isNumberVar(arrs->get(0)) ) {
                cursor->movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
                int end=cursor->position();
                if( isNumberVar(arrs->get(1)) ) {
                    int sp=toInteger(arrs->get(0)), ep=toInteger(arrs->get(1));
                    if( sp>=0 && sp<ep && ep<=end ) {
                        cursor->setPosition(sp, QTextCursor::MoveAnchor);
                        cursor->setPosition(ep, QTextCursor::KeepAnchor);
                    }
                } else if( arrs->get(1) ) {
                    LPCC txt=AS(0);
                    if( ccmp(txt,"word") ) {
                        int sp=toInteger(arrs->get(0));
                        if( sp<end ) {
                            cursor->setPosition(sp, QTextCursor::MoveAnchor);
                        }
                        cursor->select(QTextCursor::WordUnderCursor);
                    }
                }
            } else {
                LPCC txt=AS(0);
                if( ccmp(txt,"word") ) {
                    cursor->select(QTextCursor::WordUnderCursor);
                }
            }
        }
    } break;
    case 13: {	// insertList
        if( arrs==NULL ) return true;
        int size=arrs->size();
        qreal marginTop=0;
        LPCC ty=NULL;
        QTextCharFormat fmt=cursor->charFormat();
        int sp=0, ep=0;
        if( size==1 ) {
            ty=AS(1);
            if( cursor->hasSelection() ) {
                sp=cursor->selectionStart(), ep=cursor->selectionEnd();
            }
        } else {
            // insertList("text", "disc", margin);
            LPCC text=AS(0);
            QString str=KR(text);
            ty=AS(1);
            sp=cursor->position(), ep=sp+str.length();
            if( size>2 ) {
                textCharFormatApply(fmt, arrs->get(2));
                cursor->insertText(str, fmt);
                sv=arrs->get(3);
                if( sv && sv->length()) {
                    marginTop=(qreal)toDouble(sv);
                }
            } else {
                cursor->insertText(str);
            }
            cursor->setPosition(sp, QTextCursor::MoveAnchor);
            cursor->setPosition(ep, QTextCursor::KeepAnchor);
        }
        if( sp==ep ) return true;
        QTextListFormat::Style style = QTextListFormat::ListStyleUndefined;
        if( slen(ty) ) {
            if( ccmp(ty,"disc") )  style = QTextListFormat::ListDisc;
            else if( ccmp(ty,"circle") ) style = QTextListFormat::ListCircle;
            else if( ccmp(ty,"square") ) style = QTextListFormat::ListSquare;
            else if( ccmp(ty,"num") ) style = QTextListFormat::ListDecimal;
            else if( ccmp(ty,"lower") ) style = QTextListFormat::ListLowerAlpha;
            else if( ccmp(ty,"upper") ) style = QTextListFormat::ListUpperAlpha;
            else if( ccmp(ty,"roman") ) style = QTextListFormat::ListUpperRoman;
            else if( ccmp(ty,"roman1") ) style = QTextListFormat::ListLowerRoman;
        }

        cursor->beginEditBlock();
        QTextBlockFormat blockFmt = cursor->blockFormat();
        QTextListFormat listFmt;
        if( cursor->currentList() ) {
            listFmt = cursor->currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            if( marginTop ) {
                blockFmt.setTopMargin(marginTop);
            }
            cursor->setBlockFormat(blockFmt);
        }
        listFmt.setStyle(style);
        QTextList* list=cursor->createList(listFmt);
        if( list && marginTop ) {
            for( int num=0,sz=list->count();num<sz;num++ ) {
                list->item(num).blockFormat().setTopMargin(marginTop);
            }
        }
        cursor->endEditBlock();
        cursor->setPosition(ep, QTextCursor::MoveAnchor);
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor->mergeBlockFormat(bfmt);
        cursor->setCharFormat(fmt);
    } break;
    case 14: {	// text
        if( arrs==NULL ) return false;
        // ex) cursor.text("text", "fmt", prevCheck);
        QTextCharFormat prev=cursor->charFormat();
        LPCC text=AS(0);
        sv=arrs->get(1);
        if( sv && sv->length() ) {
            QTextCharFormat fmt;
            textCharFormatApply(fmt, sv);
            cursor->mergeCharFormat(fmt);
            editor->mergeCurrentCharFormat(fmt);
        }
        cursor->insertText(KR(text));
        if( sv && isVarTrue(arrs->get(2)) ) {
            cursor->setCharFormat(prev);
        }

    } break;
    case 5: {	// frame
        QTextCursor c(*cursor);
        QTextFrame *topFrame = cursor->currentFrame();
        QTextFrameFormat bodyFrameFormat;
        LPCC val=NULL;
        sv=arrs->get(1);
        if( SVCHK('n',0) ) {
            TreeNode* cf=(TreeNode*)SVO;
            XParseVar pv;
            sv=cf->gv("width");
            if( sv ) {
                if( isNumberVar(sv) ) {
                    bodyFrameFormat.setWidth(toInteger(sv));
                } else {
                    val=toString(sv);
                    int pos=IndexOf(val,'%');
                    if( pos ) {
                        val=gBuf.add(val, pos);
                        if( isnum(val) ) {
                            int width=atoi(val);
                            bodyFrameFormat.setWidth(QTextLength(QTextLength::PercentageLength, width));
                        }
                    }
                }
            }
            sv=cf->gv("height");
            if( sv ) {
                if( isNumberVar(sv) ) {
                    bodyFrameFormat.setHeight(toInteger(sv));
                } else {
                    val=toString(sv);
                    int pos=IndexOf(val,'%');
                    if( pos ) {
                        val=gBuf.add(val, pos);
                        if( isnum(val) ) {
                            int width=atoi(val);
                            bodyFrameFormat.setHeight(QTextLength(QTextLength::PercentageLength, width));
                        }
                    }
                }
            }
            sv=cf->gv("color");
            if( sv ) {
                bodyFrameFormat.setForeground( QBrush(getQColor(sv)) );
            }
            sv=cf->gv("background");
            if( sv ) {
                bodyFrameFormat.setBackground( QBrush(getQColor(sv)) );
            }
            sv=cf->gv("position");
            if( sv ) {
                val=toString(sv);
                bodyFrameFormat.setPosition( ccmp(val,"left")? QTextFrameFormat::FloatLeft:
                    ccmp(val,"right")? QTextFrameFormat::FloatRight: QTextFrameFormat::InFlow );
            }
            sv=cf->gv("border");
            if( sv ) {
                if( sv->find(",")!=-1 ) {
                    pv.SetVar(sv,0,sv->length() );
                    for( int n=0; n<3 && pv.valid(); n++ ) {
                        val=pv.findEnd(",").v();
                        if( n==0 ) {
                            if( isnum(val) ) bodyFrameFormat.setBorder(atoi(val) );
                        } else if( n==1 ) {
                            rst->set(val);
                            bodyFrameFormat.setBorderBrush( QBrush(getQColor(rst)) );
                        } else if( n==2 ) {
                            bodyFrameFormat.setBorderStyle( ccmp(val,"dash")? QTextFrameFormat::BorderStyle_Dashed :
                                ccmp(val,"dot")? QTextFrameFormat::BorderStyle_Dotted :
                                ccmp(val,"dotDash")? QTextFrameFormat::BorderStyle_DotDash :
                                ccmp(val,"ride")? QTextFrameFormat::BorderStyle_Ridge :
                                ccmp(val,"grove")? QTextFrameFormat::BorderStyle_Groove :
                                ccmp(val,"in")? QTextFrameFormat::BorderStyle_Inset :
                                ccmp(val,"out")? QTextFrameFormat::BorderStyle_Outset :
                                QTextFrameFormat::BorderStyle_Solid );
                        }
                    }
                } else {
                    val=toString(sv);
                    if( isnum(val) ) {
                        bodyFrameFormat.setBorder(atoi(val) );
                    }
                }
            }
            sv=cf->gv("padding");
            if( sv ) {
                val=toString(sv);
                if( isnum(val) ) {
                    bodyFrameFormat.setPadding(atoi(val) );
                }
            }
            sv=cf->gv("margin");
            if( sv ) {
                if( sv->find(",")!=-1 ) {
                    pv.SetVar(sv,0,sv->length() );
                    for( int n=0; n<4 && pv.valid(); n++ ) {
                        val=pv.findEnd(",").v();
                        if( isnum(val) ) {
                            if( n==0 ) bodyFrameFormat.setLeftMargin(atoi(val) );
                            else if( n==1 ) bodyFrameFormat.setTopMargin(atoi(val) );
                            else if( n==2 ) bodyFrameFormat.setRightMargin(atoi(val) );
                            else if( n==3 ) bodyFrameFormat.setBottomMargin(atoi(val) );
                        }
                    }
                } else {
                    val=toString(sv);
                    if( isnum(val) ) {
                        bodyFrameFormat.setMargin(atoi(val) );
                    }
                }
            }
        } else {
            bodyFrameFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
            bodyFrameFormat.setBorder(1);
            bodyFrameFormat.setPadding(8);
        }
        QTextFrame *frame=c.insertFrame(bodyFrameFormat);
        sv=arrs->get(0);
        if( SVCHK('f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)SVO;
            XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc, fn);
            TreeNode* node=editor->config();
            fnCur->GetVar("@this")->setVar('n',0,(LPVOID)node);
            fnCur->GetVar("cursor")->setVar('c',2,(LPVOID)&c);
            fnCur->call();
            funcNodeDelete(fnCur);
        }
        cursor->setPosition( topFrame->lastPosition() );

    } break;
    case 6: {	// insertHtml
        if( arrs==NULL ) return true;
        LPCC html=AS(0);
        QTextCharFormat fmt=cursor->charFormat();
        cursor->insertHtml(KR(html));
        cursor->setCharFormat(fmt);
    } break;
    case 7: {	// insertImage
        if( arrs==NULL ) return true;
        editorInsertImage(editor->document(), *cursor, arrs, rst );
    } break;
    case 90: {	// valid

    } break;
    default: return false;
    }
    return true;
}

void textCharFormatInline(XParseVar& pv, QTextCharFormat& fmt) {
    LPCC key=NULL, val=NULL;
    StrVar* rst=getStrVarTemp();
    while( pv.valid() ) {
        key=pv.NextWord();
        if( pv.ch()!='=') {
            break;
        }
        pv.incr();
        char ch=pv.ch();
        if( (ch=='\'' || ch=='"')  && pv.MatchStringPos(ch) ) {
            val=pv.GetChars(pv.prev, pv.cur);
        } else {
            val=pv.findEnd(" \t\n", FIND_CHARS).v();
        }
        rst->set(val);
        if( ccmp(key,"name") ) {
            fmt.setAnchorName(KR(val));
            fmt.setProperty(100, QVariant(val));
        } else if( ccmp(key,"color") ) {
            fmt.setForeground(getQColor(rst));
            fmt.setProperty(101, QVariant(val));
        } else if( ccmp(key,"weight") ) {
            int num=ccmp(val,"bold")? 75: ccmp(val,"black")? 87: ccmp(val,"light")? 25:ccmp(val,"demi")? 63: 50;
            fmt.setFontWeight(num );
            fmt.setProperty(102, QVariant(num));
        } else if( ccmp(key,"background") ) {
            fmt.setBackground(getQColor(rst));
            fmt.setProperty(103, QVariant(val));
        } else if( ccmp(key,"href") ) {
            fmt.setAnchorHref(KR(val));
        } else if( ccmp(key,"family") ) {
            fmt.setFontFamily(KR(val));
            fmt.setProperty(105, QVariant(val));
        } else if( ccmp(key,"tip") ) {
            fmt.setToolTip(KR(val));
            fmt.setProperty(106, QVariant(val));
        } else if( ccmp(key,"size") ) {
            int fontSize=toInteger(rst);
            fmt.setFontPointSize(fontSize);
            fmt.setProperty(107, QVariant(fontSize));
        } else if( ccmp(key,"underline") ) {
            fmt.setUnderlineStyle(ccmp(val,"dash") ? QTextCharFormat::DashUnderline :
                ccmp(val,"dot") ? QTextCharFormat::DotLine :
                ccmp(val,"wave") ? QTextCharFormat::WaveUnderline : QTextCharFormat::SingleUnderline );
            fmt.setProperty(108, QVariant(val));
        } else if( ccmp(key,"underlineColor") ) {
            fmt.setUnderlineColor(getQColor(rst));
            fmt.setProperty(109, QVariant(val));
        } else if( ccmp(key,"italic") ) {
            bool check=isVarTrue(rst);
            fmt.setFontItalic(check);
            fmt.setProperty(110, QVariant(check));
        } else if( ccmp(key,"overline") ) {
            bool check=isVarTrue(rst);
            fmt.setFontOverline(check);
            fmt.setProperty(111, QVariant(check));
        } else if( ccmp(key,"stricke") ) {
            bool check=isVarTrue(rst);
            fmt.setFontStrikeOut(check);
            fmt.setProperty(112, QVariant(check));
        } else if( ccmp(key,"align") ) {
            fmt.setVerticalAlignment( ccmp(val,"base") ? QTextCharFormat::AlignBaseline :
                ccmp(val,"top") ? QTextCharFormat::AlignTop:
                ccmp(val,"middle") ? QTextCharFormat::AlignMiddle:
                ccmp(val,"bottom") ? QTextCharFormat::AlignBottom: QTextCharFormat::AlignMiddle
            );
            fmt.setProperty(113, QVariant(val));
        }
    }
}
void textCharFormatApply(QTextCharFormat& fmt, StrVar* sv) {
    int sp=0, ep=0;
    sv=getStrVarPointer(sv, &sp, &ep);
    XParseVar pv(sv, sp, ep );
    textCharFormatInline(pv, fmt);
}

void textCharFormatText(QTextCharFormat& fmt, QString& text, StrVar* rst) {
    if( fmt.isAnchor() ) {
        QString str="<#link "+fmt.anchorHref()+"; "+text+"#>";
        rst->add(Q2A(str));
    } else {
        int cnt=fmt.propertyCount();
        if( cnt> 0 ) {
            QVariant var=fmt.property(120);
            if( var.isValid() ) {
                rst->add("<#img src=").add(Q2A(var.toString()) ).add(";#>");
            } else {
                QMapIterator<int,QVariant> iter(fmt.properties());
                QString val="";
                rst->add("<#style");
                while( iter.hasNext() ) {
                    iter.next();
                    val=iter.value().toString();
                    switch(iter.key() ) {
                    case 100: rst->add(" name=").add(Q2A(val)); break;
                    case 101: rst->add(" color=").add(Q2A(val)); break;
                    case 102: rst->add(" weight=").add(Q2A(val)); break;
                    case 103: rst->add(" background=").add(Q2A(val)); break;
                    case 104: rst->add(" href=").add(Q2A(val)); break;
                    case 105: rst->add(" family=").add(Q2A(val)); break;
                    case 106: rst->add(" tip=").add(Q2A(val)); break;
                    case 107: rst->add(" size=").add(Q2A(val)); break;
                    case 108: rst->add(" underline=").add(Q2A(val)); break;
                    case 109: rst->add(" underlineColor=").add(Q2A(val)); break;
                    case 110: rst->add(" italic=").add(Q2A(val)); break;
                    case 111: rst->add(" overline=").add(Q2A(val)); break;
                    case 112: rst->add(" stricke=").add(Q2A(val)); break;
                    case 113: rst->add(" align=").add(Q2A(val)); break;
                    default: break;
                    }
                }
                rst->add(";").add(Q2A(text));
                rst->add("#>");
            }
        } else {
            rst->add(Q2A(text));
        }
    }
}

void textCursorInsert(QTextDocument* doc, QTextCursor* cursor, LPCC text, StrVar* prop, StrVar* rst) {
    if( doc==NULL || cursor==NULL ) return;
    QTextCharFormat fmt=cursor->charFormat();
    if( prop ) {
        int sp=0, ep=0;
        QTextCharFormat tmp(fmt);
        StrVar* sv=getStrVarPointer(prop, &sp, &ep);
        XParseVar pv(sv, sp, ep);
        LPCC type=pv.NextWord();
        if( ccmp(type,"link") ) {
            LPCC href=NULL;
            if( pv.StartWith("href", -1, 1) ) {
                char ch=pv.ch();
                if( ch=='=' ) {
                    ch=pv.incr().ch();
                }
                if( ch=='\'' || ch=='"' ) {
                    if( pv.MatchStringPos(ch) ) {
                        href=pv.v();
                    }
                }
            }
            if( href==NULL ) {
                href=pv.Trim(pv.start, pv.wep);
            }
            LPCC html=rst->fmt("<a href=\"%s\">%s</a>",href,text);
            cursor->insertHtml(KR(html));
        } else if( ccmp(type,"html") ) {
            cursor->insertHtml(KR(text));
        } else if( ccmp(type,"imageUrl") ) {
            XListArr* arr=getListArrTemp();
            arr->add()->add(text);
            arr->add()->setVar('3',1);
            editorInsertImage(doc, *cursor, arr, rst);
        } else if( ccmp(type,"icon") || ccmp(type,"image") ) {
            XListArr* arr=getListArrTemp();
            arr->add()->add(text);
            LPCC width=NULL, height=NULL, align=NULL, val=NULL;
            while( pv.valid() ) {
                LPCC key=pv.NextWord();
                if( pv.ch()!='=') {
                    break;
                }
                pv.incr();
                char ch=pv.ch();
                if( (ch=='\'' || ch=='"')  && pv.MatchStringPos(ch) ) {
                    val=pv.GetChars(pv.prev, pv.cur);
                } else {
                    val=pv.findEnd(" \t\n", FIND_CHARS).v();
                }
                if( val ) {
                    if( ccmp(key,"width") ) width=val;
                    else if( ccmp(key,"height") ) height=val;
                    else if( ccmp(key,"align") ) align=val;
                }
            }
            if( width ) arr->add()->add(width);
            if( height ) {
                if(width==NULL ) arr->add()->add("0");
                arr->add()->add(height);
            }
            if( align) {
                if(width==NULL ) arr->add()->add("0");
                if(height==NULL ) arr->add()->add("0");
                arr->add()->add(align);
            }
            editorInsertImage(doc, *cursor, arr, rst);
        } else if( slen(text) ) {
            pv.start=sp;
            textCharFormatInline(pv, tmp);
            cursor->insertText(KR(text), tmp);
        }
        cursor->setCharFormat(fmt);
    } else {
        cursor->insertText(KR(text), fmt);
    }
}


void editorInsertImage(QTextDocument* doc, QTextCursor& cursor, PARR arrs, StrVar* rst, bool skip) {
    if( arrs==NULL ) return;
    LPCC src=NULL;
    QImage image;
    QPixmap* pix=NULL;
    int idx=2;
    StrVar* sv=arrs->get(0);
    if( SVCHK('i',6) ) {
        pix = (QPixmap*)SVO;
        src=AS(1);
    } else if( SVCHK('i',7) ) {
        image.loadFromData((const uchar *)sv->get(FUNC_HEADER_POS+4), sv->getInt(FUNC_HEADER_POS));
        src=AS(1);
    } else if( SVCHK('n',0) ) {
        TreeNode* nd = (TreeNode*)SVO;
        if( nd && nd->xstat==FNSTATE_DRAW ) {
            sv=nd->gv("@img");
            if( SVCHK('i',9) ) {
                QImage* pImg= (QImage*)SVO;
                image=*pImg;
                src=AS(1);
            }
        }
    } else {
        idx=1;
        src = toString(sv);
        pix = getPixmap(src,true);
        if( pix==NULL ) {
            image = QImageReader(KR(src)).read();
        }
    }
    if( pix ) {
        image=pix->toImage();
    }
    if( !image.isNull() && slen(src) ) {
        QString imgUri=QString("src://%1").arg(src);
        QUrl uri(imgUri);
        doc->addResource(QTextDocument::ImageResource, uri, image);
        if( skip ) {
            rst->set(Q2A(imgUri));
            return;
        }

        QTextImageFormat imageFormat;
        sv=arrs->get(idx++);
        int iw=image.width(), ih=image.height();
        if( isNumberVar(sv)) {
            int wid=toInteger(sv); sv=arrs->get(idx++);
            imageFormat.setWidth(wid? wid: iw);
            if( isNumberVar(sv) ) {
                int hei=toInteger(sv);
                imageFormat.setHeight(hei? hei: ih );
            } else {
                int hei=(wid*ih)/iw;
                imageFormat.setHeight( hei );
            }
        } else {
            imageFormat.setWidth( image.width() );
            imageFormat.setHeight( image.height() );
        }
        imageFormat.setName( uri.toString() );
        imageFormat.setProperty(120, QVariant(uri.toString()));
        sv=arrs->get(idx);
        LPCC align= sv ? toString(sv): NULL;
        if( align ) {
            cursor.insertImage(imageFormat, ccmp(align,"in")? QTextFrameFormat::InFlow: ccmp(align,"right")? QTextFrameFormat::FloatRight : QTextFrameFormat::FloatLeft );
        } else {
            cursor.insertImage(imageFormat);
        }
    }
}

int findEditorCursor(QTextDocument* doc, QTextCursor& newCursor, LPCC val, U32 flag, bool bback, LPCC rep, StrVar* findsVar) {
    if( slen(val)==0 ) return 0;
    if( newCursor.position()<0 ) {
        return 0;
    }
    QString str=KR(val);
    int sp=0, ep=-1, findNum=0;
    int start=0, end=0;
    int curPos= 0;
    if( bback ) {
        flag|=EF_BACK;
    }

    if( flag&EF_SELECTTEXT ) {
        sp=newCursor.selectionStart(), ep=newCursor.selectionEnd();
        start=sp, end=ep;
        curPos=sp;
    } else {
        if( flag&EF_BACK ) {
            newCursor.movePosition(getTypeMoveOperation("prevChar"), QTextCursor::MoveAnchor);
        } else {
            newCursor.movePosition(getTypeMoveOperation("nextChar"), QTextCursor::MoveAnchor);
        }
        curPos=newCursor.position();
        if( flag&EF_MARK || flag&EF_REPLACEALL ) {
            sp=curPos=0;
            flag&=~EF_WRAPEND;
        } else {
            sp=curPos;
        }
        newCursor.movePosition(getTypeMoveOperation("end"), QTextCursor::MoveAnchor);
        ep=newCursor.position();
        start=0, end=ep;

    }
    QTextCursor saveCursor = newCursor;
    QTextDocument::FindFlags ff = 0;

    if( flag ) {
        U32 curFlag = flag &~(EF_REPLACEALL|EF_MARK|EF_SELECTTEXT|EF_BACK);
        if( curFlag ) {
            getCf()->GetVar("@editorSearchFlag")->setVar('0',1).addU32(curFlag);
        }
    } else {
        StrVar* sv = getCf()->gv("@editorSearchFlag");
        if( SVCHK('0',1) ) {
            flag=sv->getU32(4);
        }
    }
    QString strRep;
    bool bReplace=false;
    if( slen(rep) ) {
        bReplace=true;
        strRep=KR(rep);
    } else if( flag&EF_REPLACEALL ) {
        return 0;
    }
    if( flag&EF_CASE ) ff|= QTextDocument::FindCaseSensitively;
    if( flag&EF_WHOLEWORD ) ff|= QTextDocument::FindWholeWords;
    if( flag&EF_REGEXP ) {
        str=str.replace("\\n","\n");
        str=str.replace("\\t","\t");
        if( bReplace ) {
            strRep=strRep.replace("\\n","\n");
            strRep=strRep.replace("\\t","\t");
        }
    }
    if( flag&EF_BACK ) {
        ff|= QTextDocument::FindBackward;
        curPos-=str.length();
        if( curPos<0 ) curPos=0;
    }
    newCursor.setPosition(curPos);

    StrVar* sv = NULL;
    XListArr* finds=NULL;
    if( flag&EF_MARK || flag&EF_REPLACEALL ) {
        sv = findsVar ? findsVar: cfVar("@findsVar");
        if( SVCHK('a',0) ) {
            finds = (XListArr*)SVO;
        } else {
            finds = new XListArr(true);
            sv->setVar('a',0,(LPVOID)finds );
        }
        finds->reuse();
    }

    // if( flag&EF_MARK ) newCursor.setPosition(0);
    int loopNum = 0;
    for( int n=0; n<0xFFFF; n++ ) {
        newCursor = doc->find(str, newCursor, ff);
        if( newCursor.isNull() ) {
            if( (flag&EF_WRAPEND)==0 ) {
                break;
            }
            if( loopNum==0 ) {
                newCursor=saveCursor;
                if( flag&EF_BACK ) {
                    newCursor.setPosition(end);
                    start=curPos;
                } else {
                    newCursor.setPosition(start);
                    end=curPos;
                }
                loopNum++;
            } else {
                break;
            }
        } else {
            int pos=newCursor.position();
            bool ok=false;
            if( flag&EF_BACK ) {
                if( pos>=start) ok=true;
            } else {
                if( pos<=end ) ok=true;
            }
            if( ok ) {
                findNum++;
                if( finds ) {
                    finds->add()->setVar('i',2).addInt(newCursor.selectionStart()).addInt(newCursor.selectionEnd() );
                } else {
                    if( bReplace ) {
                        newCursor.insertText(strRep);
                    }
                    break;
                }
                if( pos==end ) break;
            } else {
                if( (flag&EF_WRAPEND)==0 ) {
                    break;
                }
                if( loopNum==0  ) {
                    newCursor=saveCursor;
                    if( flag&EF_BACK ) {
                        newCursor.setPosition(ep);
                        start=curPos;
                    } else {
                        newCursor.setPosition(sp);
                        end=curPos;
                    }
                    loopNum++;
                } else {
                    break;
                }
            }
        }
    }
    if( finds && flag&EF_REPLACEALL ) {
        int sz=finds->size();
        if( sz>0 ) {
            bubbleSort(finds, 1);
            newCursor=saveCursor;
            newCursor.beginEditBlock();
            int len=slen(rep); // strRep.length();
            for( int n=sz-1; n>=0; n-- ) {
                sv=finds->get(n);
                if( SVCHK('i',2) ) {
                    sp=sv->getInt(4), ep=sv->getInt(8);
                    newCursor.setPosition(sp);
                    newCursor.setPosition(ep,QTextCursor::KeepAnchor);
                    newCursor.insertText(strRep);
                    sv->putInt(8, sp+len);
                }
            }
            newCursor.endEditBlock();
        }
    }
    return findNum;
}
