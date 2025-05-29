/* Copyright (c) 2021-2025 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef XSTACKVIEW_H
#define XSTACKVIEW_H

#include "xdevicetableeditview.h"

// TODO header clickable Address -> Offset -> ESP ->EBP
// TODO show comments

class XStackView : public XDeviceTableEditView {
    Q_OBJECT

public:
    struct OPTIONS {
        XADDR nStartAddress;
        XADDR nCurrentAddress;
        XADDR nCurrentStackPointer;
    };

    explicit XStackView(QWidget *pParent = nullptr);
    void setData(QIODevice *pDevice, const OPTIONS &options, bool bReload = true);
    void goToAddress(qint64 nAddress);
    void goToOffset(qint64 nOffset);
    void setSelectionAddress(qint64 nAddress);

    void _adjustView();
    void adjustView();

    void setCurrentStackPointer(XADDR nAddress);

private:
    enum COLUMN {
        COLUMN_ADDRESS = 0,
        COLUMN_VALUE,
        COLUMN_COMMENT
    };

    struct RECORD {
        qint64 nOffset;
        QString sAddress;
        XADDR nAddress;
        QString sValue;
        QString sComment;
    };

    struct TEXT_OPTION {
        bool bSelected;
        bool bCurrentSP;
        //        bool bCursor;
        bool bIsReplaced;
    };

    enum MODE_COMMENT {
        MODE_COMMENT_GENERAL = 0,
        MODE_COMMENT_ADDRESS,
        MODE_COMMENT_ANSI,
        MODE_COMMENT_UNICODE,
        MODE_COMMENT_UTF8
    };

    void drawText(QPainter *pPainter, qint32 nLeft, qint32 nTop, qint32 nWidth, qint32 nHeight, const QString &sText, TEXT_OPTION *pTextOption);

protected:
    virtual OS cursorPositionToOS(CURSOR_POSITION cursorPosition);
    virtual void updateData();
    virtual void paintCell(QPainter *pPainter, qint32 nRow, qint32 nColumn, qint32 nLeft, qint32 nTop, qint32 nWidth, qint32 nHeight);
    virtual void contextMenu(const QPoint &pos);
    virtual void keyPressEvent(QKeyEvent *pEvent);
    virtual qint64 getCurrentViewPosFromScroll();
    virtual void setCurrentViewPosToScroll(qint64 nOffset);
    virtual void adjustColumns();
    virtual void registerShortcuts(bool bState);
    virtual void _headerClicked(qint32 nColumn);
    virtual void _cellDoubleClicked(qint32 nRow, qint32 nColumn);
    virtual qint64 getRecordSize(qint64 nOffset);
    virtual void adjustScrollCount();

private:
    qint32 g_nBytesProLine;
    OPTIONS g_options;
    QByteArray g_baDataBuffer;
    QList<RECORD> g_listRecords;
    qint32 g_nAddressWidth;
    qint32 g_nDataBlockSize;
    MODE_COMMENT g_modeComment;
    bool g_bIsAddressColon;
    XADDR g_nCurrentStackPointer;
};

#endif  // XSTACKVIEW_H
