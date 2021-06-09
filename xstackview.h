// copyright (c) 2021 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#ifndef XSTACKVIEW_H
#define XSTACKVIEW_H

#include "xdevicetableview.h"

class XStackView : public XDeviceTableView
{
    Q_OBJECT

public:
    struct OPTIONS
    {
        qint64 nStartAddress;
        qint64 nCurrentAddress;
        QString sSignaturesPath;
    };

    explicit XStackView(QWidget *pParent=nullptr);
    void setData(QIODevice *pDevice,OPTIONS options);
    void goToAddress(qint64 nAddress);
    void goToOffset(qint64 nOffset);
    void setSelectionAddress(qint64 nAddress);

private:
    enum COLUMN
    {
        COLUMN_ADDRESS=0,
        COLUMN_VALUE,
        COLUMN_COMMENT
    };

protected:
    virtual OS cursorPositionToOS(CURSOR_POSITION cursorPosition);
    virtual void updateData();
    virtual void paintCell(QPainter *pPainter,qint32 nRow,qint32 nColumn,qint32 nLeft,qint32 nTop,qint32 nWidth,qint32 nHeight);
    virtual void contextMenu(const QPoint &pos);
    virtual void keyPressEvent(QKeyEvent *pEvent);
    virtual qint64 getScrollValue();
    virtual void setScrollValue(qint64 nOffset);
    virtual void adjustColumns();
    virtual void registerShortcuts(bool bState);

private:
    qint32 g_nBytesProLine;
    OPTIONS g_options;
    QByteArray g_baDataBuffer;
    QList<QString> g_listValues;
    QList<QString> g_listAddresses;
    qint32 g_nAddressWidth;
    qint32 g_nDataBlockSize;
};

#endif // XSTACKVIEW_H
