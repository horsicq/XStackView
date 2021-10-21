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
#include "xstackview.h"

XStackView::XStackView(QWidget *pParent) : XDeviceTableView(pParent)
{  
#ifndef Q_OS_WIN64
    g_nBytesProLine=4;
    g_nAddressWidth=8;
#else
    g_nBytesProLine=8;
    g_nAddressWidth=16;
#endif

    addColumn(tr("Address"));
    addColumn(tr("Value"));
    addColumn(tr("Comment"));

    setLastColumnScretch(true);

    setTextFont(getMonoFont(10));
}

void XStackView::setData(QIODevice *pDevice, OPTIONS options)
{
    g_options=options;

    setDevice(pDevice);

    XBinary binary(pDevice,true,options.nStartAddress);
    XBinary::_MEMORY_MAP memoryMap=binary.getMemoryMap();

    setMemoryMap(memoryMap);

    resetCursorData();

    adjustColumns();

    qint64 nTotalLineCount=getDataSize()/g_nBytesProLine;

    if(getDataSize()%g_nBytesProLine==0)
    {
        nTotalLineCount--;
    }

    setTotalLineCount(nTotalLineCount);

    if(options.nCurrentAddress)
    {
        goToAddress(options.nCurrentAddress);
    }

//    setSelection(options.nStartSelectionOffset,options.nSizeOfSelection);
//    setCursorOffset(options.nStartSelectionOffset,COLUMN_HEX);

    reload(true);
}

void XStackView::goToAddress(qint64 nAddress)
{
    _goToOffset(nAddress-g_options.nStartAddress);
}

void XStackView::goToOffset(qint64 nOffset)
{
    _goToOffset(nOffset);
}

void XStackView::setSelectionAddress(qint64 nAddress)
{
    XDeviceTableView::setSelectionAddress(nAddress,g_nBytesProLine);
}

XAbstractTableView::OS XStackView::cursorPositionToOS(CURSOR_POSITION cursorPosition)
{
    OS osResult={};

        osResult.nOffset=-1;

        if((cursorPosition.bIsValid)&&(cursorPosition.ptype==PT_CELL))
        {
            qint64 nBlockOffset=getViewStart()+(cursorPosition.nRow*g_nBytesProLine);

            osResult.nOffset=nBlockOffset;
            osResult.nSize=g_nBytesProLine;
        }

        return osResult;
}

void XStackView::updateData()
{
    if(getDevice())
    {
        // Update cursor position
        qint64 nBlockOffset=getViewStart();
        qint64 nCursorOffset=nBlockOffset;

        if(nCursorOffset>=getDataSize())
        {
            nCursorOffset=getDataSize()-1;
        }

        setCursorOffset(nCursorOffset);

        g_listAddresses.clear();
        g_listValues.clear();

        qint32 nDataBlockSize=g_nBytesProLine*getLinesProPage();

        g_baDataBuffer=read_array(nBlockOffset,nDataBlockSize);

        g_nDataBlockSize=g_baDataBuffer.size();

        XBinary::MODE mode=XBinary::getWidthModeFromByteSize(g_nAddressWidth);

        if(g_nDataBlockSize)
        {
            char *pData=g_baDataBuffer.data();

            for(qint32 i=0;i<g_nDataBlockSize;i+=g_nBytesProLine)
            {
                QString sAddress=XBinary::valueToHex(mode,i+g_options.nStartAddress+nBlockOffset);
                QString sValue=XBinary::valueToHex(mode,XBinary::_read_value(mode,pData+i));

                g_listAddresses.append(sAddress);
                g_listValues.append(sValue);
            }
        }
        else
        {
            g_baDataBuffer.clear();
            g_listValues.clear();
        }

        setCurrentBlock(nBlockOffset,g_nDataBlockSize);
    }
}

void XStackView::paintCell(QPainter *pPainter, qint32 nRow, qint32 nColumn, qint32 nLeft, qint32 nTop, qint32 nWidth, qint32 nHeight)
{
    if(nColumn==COLUMN_ADDRESS)
    {
        if(nRow<g_listAddresses.count())
        {
            pPainter->drawText(nLeft+getCharWidth(),nTop+nHeight,g_listAddresses.at(nRow)); // TODO Text Optional
        }
    }
    else if(nColumn==COLUMN_VALUE)
    {
        if(nRow<g_listValues.count())
        {
            pPainter->drawText(nLeft+getCharWidth(),nTop+nHeight,g_listValues.at(nRow)); // TODO Text Optional
        }
    }
}

void XStackView::contextMenu(const QPoint &pos)
{
    // TODO
}

void XStackView::keyPressEvent(QKeyEvent *pEvent)
{

}

qint64 XStackView::getScrollValue()
{
    qint64 nResult=0;

    qint32 nValue=verticalScrollBar()->value();

    qint64 nMaxValue=getMaxScrollValue()*g_nBytesProLine;

    if(getDataSize()>nMaxValue)
    {
        if(nValue==getMaxScrollValue())
        {
            nResult=getDataSize()-g_nBytesProLine;
        }
        else
        {
            nResult=((double)nValue/(double)getMaxScrollValue())*getDataSize();
        }
    }
    else
    {
        nResult=(qint64)nValue*g_nBytesProLine;
    }

    return nResult;
}

void XStackView::setScrollValue(qint64 nOffset)
{
    setViewStart(nOffset);

    qint32 nValue=0;

    if(getDataSize()>(getMaxScrollValue()*g_nBytesProLine))
    {
        if(nOffset==getDataSize()-g_nBytesProLine)
        {
            nValue=getMaxScrollValue();
        }
        else
        {
            nValue=((double)(nOffset)/((double)getDataSize()))*(double)getMaxScrollValue();
        }
    }
    else
    {
        nValue=(nOffset)/g_nBytesProLine;
    }

    verticalScrollBar()->setValue(nValue);
}

void XStackView::adjustColumns()
{
    const QFontMetricsF fm(getTextFont());

#ifndef Q_OS_WIN64
    setColumnWidth(COLUMN_ADDRESS,2*getCharWidth()+fm.boundingRect("00000000").width());
    setColumnWidth(COLUMN_VALUE,2*getCharWidth()+fm.boundingRect("00000000").width());
#else
    setColumnWidth(COLUMN_ADDRESS,2*getCharWidth()+fm.boundingRect("0000000000000000").width());
    setColumnWidth(COLUMN_VALUE,2*getCharWidth()+fm.boundingRect("0000000000000000").width());
#endif
}

void XStackView::registerShortcuts(bool bState)
{
    Q_UNUSED(bState)
}
