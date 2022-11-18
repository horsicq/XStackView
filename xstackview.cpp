/* Copyright (c) 2021-2022 hors<horsicq@gmail.com>
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
#include "xstackview.h"

XStackView::XStackView(QWidget *pParent)
    : XDeviceTableEditView(pParent)
{
    // TODO Check
    if (sizeof(void *) == 8) {
        g_nBytesProLine = 8;
        g_nAddressWidth = 16;
    } else {
        g_nBytesProLine = 4;
        g_nAddressWidth = 8;
    }

    g_modeComment = MODE_COMMENT_GENERAL;
    g_bIsAddressColon = false;
    g_nCurrentStackPointer = -1;

    addColumn(tr("Address"), 0, true);
    addColumn(tr("Value"));
    addColumn(tr("Comment"), 0, true);

    //    setLastColumnStretch(true);

    setTextFont(getMonoFont(10));
}

void XStackView::setData(QIODevice *pDevice, OPTIONS options, bool bReload)
{
    g_options = options;

    setDevice(pDevice);

    XBinary binary(pDevice, true, options.nStartAddress);
    XBinary::_MEMORY_MAP memoryMap = binary.getMemoryMap();

    setMemoryMap(memoryMap);

    resetCursorData();

    adjustColumns();

    qint64 nTotalLineCount = getDataSize() / g_nBytesProLine;

    if (getDataSize() % g_nBytesProLine == 0) {
        nTotalLineCount--;
    }

    setTotalLineCount(nTotalLineCount);

    if (options.nCurrentAddress) {
        goToAddress(options.nCurrentAddress);
    }

    setCurrentStackPointer(options.nCurrentStackPointer);

    //    setSelection(options.nStartSelectionOffset,options.nSizeOfSelection);
    //    setCursorOffset(options.nStartSelectionOffset,COLUMN_HEX);

    _adjustView();

    if (bReload) {
        reload(true);
    }
}

void XStackView::goToAddress(qint64 nAddress)
{
    _goToOffset(nAddress - g_options.nStartAddress);
}

void XStackView::goToOffset(qint64 nOffset)
{
    _goToOffset(nOffset);
}

void XStackView::setSelectionAddress(qint64 nAddress)
{
    XDeviceTableView::setSelectionAddress(nAddress, g_nBytesProLine);
}

void XStackView::_adjustView()
{
    QFont _font;
    QString sFont = getGlobalOptions()->getValue(XOptions::ID_STACK_FONT).toString();

    if ((sFont != "") && _font.fromString(sFont)) {
        setTextFont(_font);
    }
    // mb TODO errorString signal if invalid font
    // TODO Check
    g_bIsAddressColon = getGlobalOptions()->getValue(XOptions::ID_STACK_ADDRESSCOLON).toBool();
}

void XStackView::adjustView()
{
    _adjustView();

    if (getDevice()) {
        reload(true);
    }
}

void XStackView::setCurrentStackPointer(XADDR nAddress)
{
    g_nCurrentStackPointer = nAddress;
}

void XStackView::drawText(QPainter *pPainter, qint32 nLeft, qint32 nTop, qint32 nWidth, qint32 nHeight, QString sText, TEXT_OPTION *pTextOption)
{
    QRect rectText;

    rectText.setLeft(nLeft + getCharWidth());
    rectText.setTop(nTop + getLineDelta());
    rectText.setWidth(nWidth);
    rectText.setHeight(nHeight - getLineDelta());

    bool bSave = false;

    if ((pTextOption->bCursor) || (pTextOption->bCurrentSP)) {
        bSave = true;
    }

    if (bSave) {
        pPainter->save();
    }

    if ((pTextOption->bSelected) && (!pTextOption->bCursor) && (!pTextOption->bCurrentSP)) {
        pPainter->fillRect(nLeft, nTop, nWidth, nHeight, viewport()->palette().color(QPalette::Highlight));
    }

    if (pTextOption->bIsReplaced) {
        pPainter->fillRect(nLeft, nTop, nWidth, nHeight, QColor(Qt::red));
    } else if ((pTextOption->bCursor) || (pTextOption->bCurrentSP)) {
        pPainter->fillRect(nLeft, nTop, nWidth, nHeight, viewport()->palette().color(QPalette::WindowText));
        pPainter->setPen(viewport()->palette().color(QPalette::Base));
    }

    QTextOption textOption;
    textOption.setWrapMode(QTextOption::NoWrap);

    pPainter->drawText(rectText, sText, textOption);

    if (bSave) {
        pPainter->restore();
    }
}

XAbstractTableView::OS XStackView::cursorPositionToOS(CURSOR_POSITION cursorPosition)
{
    OS osResult = {};

    osResult.nOffset = -1;

    if ((cursorPosition.bIsValid) && (cursorPosition.ptype == PT_CELL)) {
        qint64 nBlockOffset = getViewStart() + (cursorPosition.nRow * g_nBytesProLine);

        osResult.nOffset = nBlockOffset;
        osResult.nSize = g_nBytesProLine;
    }

    return osResult;
}

void XStackView::updateData()
{
    if (getDevice()) {
        if (getXInfoDB()) {
            QList<XBinary::MEMORY_REPLACE> listMR = getXInfoDB()->getMemoryReplaces(getMemoryMap()->nModuleAddress, getMemoryMap()->nImageSize);

            setMemoryReplaces(listMR);
        }

        qint64 nBlockOffset = getViewStart();
        // Update cursor position
        //        qint64 nCursorOffset=nBlockOffset;

        //        if(nCursorOffset>=getDataSize())
        //        {
        //            nCursorOffset=getDataSize()-1;
        //        }

        //        setCursorOffset(nCursorOffset);

        g_listRecords.clear();

        qint32 nDataBlockSize = g_nBytesProLine * getLinesProPage();

        g_baDataBuffer = read_array(nBlockOffset, nDataBlockSize);

        g_nDataBlockSize = g_baDataBuffer.size();

        XBinary::MODE mode = XBinary::getWidthModeFromByteSize(g_nAddressWidth);

        if (g_nDataBlockSize) {
            XInfoDB::RI_TYPE riType = XInfoDB::RI_TYPE_GENERAL;

            if (g_modeComment == MODE_COMMENT_GENERAL)
                riType = XInfoDB::RI_TYPE_GENERAL;
            else if (g_modeComment == MODE_COMMENT_ADDRESS)
                riType = XInfoDB::RI_TYPE_ADDRESS;
            else if (g_modeComment == MODE_COMMENT_ANSI)
                riType = XInfoDB::RI_TYPE_ANSI;
            else if (g_modeComment == MODE_COMMENT_UNICODE)
                riType = XInfoDB::RI_TYPE_UNICODE;
            else if (g_modeComment == MODE_COMMENT_UTF8)
                riType = XInfoDB::RI_TYPE_UTF8;

            char *pData = g_baDataBuffer.data();

            for (qint32 i = 0; i < g_nDataBlockSize; i += g_nBytesProLine) {
                RECORD record = {};
                record.nOffset = i + nBlockOffset;
                record.nAddress = i + g_options.nStartAddress + nBlockOffset;

                qint64 nCurrentAddress = 0;

                if (getAddressMode() == MODE_ADDRESS) {
                    nCurrentAddress = record.nAddress;
                } else if (getAddressMode() == MODE_OFFSET) {
                    nCurrentAddress = i + nBlockOffset;
                }

                if (g_bIsAddressColon) {
                    record.sAddress = XBinary::valueToHexColon(mode, nCurrentAddress);  // TODO Settings
                } else {
                    record.sAddress = XBinary::valueToHex(mode, nCurrentAddress);
                }

                quint64 nValue = XBinary::_read_value(mode, pData + i);

                record.sComment = XInfoDB::recordInfoToString(getXInfoDB()->getRecordInfoCache(nValue), riType);

                record.sValue = XBinary::valueToHex(mode, nValue);

                g_listRecords.append(record);

                // TODO Comments
                // TODO Breakpoints
            }
        } else {
            g_baDataBuffer.clear();
        }

        setCurrentBlock(nBlockOffset, g_nDataBlockSize);
    }
}

void XStackView::paintCell(QPainter *pPainter, qint32 nRow, qint32 nColumn, qint32 nLeft, qint32 nTop, qint32 nWidth, qint32 nHeight)
{
    Q_UNUSED(nWidth)

    qint32 nNumberOfRows = g_listRecords.count();

    qint64 nCursorOffset = getState().nCursorOffset;

    if (nRow < nNumberOfRows) {
        qint64 nOffset = g_listRecords.at(nRow).nOffset;
        qint64 nAddress = g_listRecords.at(nRow).nAddress;

        // TODO replaced !!!
        TEXT_OPTION textOption = {};
        textOption.bSelected = isOffsetSelected(nOffset);
        textOption.bCurrentSP = ((g_nCurrentStackPointer != -1) && (nAddress == g_nCurrentStackPointer) && (nColumn == COLUMN_ADDRESS));
        textOption.bCursor = (nOffset == nCursorOffset) && (nColumn == COLUMN_VALUE);
        //        textOption.bIsReplaced=((g_listRecords.at(nRow).bIsReplaced)&&(nColumn==COLUMN_ADDRESS));

        if (nColumn == COLUMN_ADDRESS) {
            drawText(pPainter, nLeft, nTop, nWidth, nHeight, g_listRecords.at(nRow).sAddress, &textOption);
        } else if (nColumn == COLUMN_VALUE) {
            drawText(pPainter, nLeft, nTop, nWidth, nHeight, g_listRecords.at(nRow).sValue, &textOption);
        } else if (nColumn == COLUMN_COMMENT) {
            drawText(pPainter, nLeft, nTop, nWidth, nHeight, g_listRecords.at(nRow).sComment, &textOption);
        }
    }
}

void XStackView::contextMenu(const QPoint &pos)
{
    Q_UNUSED(pos)
    // TODO
}

void XStackView::keyPressEvent(QKeyEvent *pEvent)
{
    XAbstractTableView::keyPressEvent(pEvent);
}

qint64 XStackView::getScrollValue()
{
    qint64 nResult = 0;

    qint32 nValue = verticalScrollBar()->value();

    qint64 nMaxValue = getMaxScrollValue() * g_nBytesProLine;

    if (getDataSize() > nMaxValue) {
        if (nValue == getMaxScrollValue()) {
            nResult = getDataSize() - g_nBytesProLine;
        } else {
            nResult = ((double)nValue / (double)getMaxScrollValue()) * getDataSize();
        }
    } else {
        nResult = (qint64)nValue * g_nBytesProLine;
    }

    return nResult;
}

void XStackView::setScrollValue(qint64 nOffset)
{
    setViewStart(nOffset);

    qint32 nValue = 0;

    if (getDataSize() > (getMaxScrollValue() * g_nBytesProLine)) {
        if (nOffset == getDataSize() - g_nBytesProLine) {
            nValue = getMaxScrollValue();
        } else {
            nValue = ((double)(nOffset) / ((double)getDataSize())) * (double)getMaxScrollValue();
        }
    } else {
        nValue = (nOffset) / g_nBytesProLine;
    }

    verticalScrollBar()->setValue(nValue);

    // adjust(true);
}

void XStackView::adjustColumns()
{
    const QFontMetricsF fm(getTextFont());

    // TODO option
    if (sizeof(void *) == 8) {
        setColumnWidth(COLUMN_ADDRESS, 2 * getCharWidth() + fm.boundingRect("00000000:00000000").width());
        setColumnWidth(COLUMN_VALUE, 2 * getCharWidth() + fm.boundingRect("00000000:00000000").width());
    } else {
        setColumnWidth(COLUMN_ADDRESS, 2 * getCharWidth() + fm.boundingRect("0000:0000").width());
        setColumnWidth(COLUMN_VALUE, 2 * getCharWidth() + fm.boundingRect("0000:0000").width());
    }

    setColumnWidth(COLUMN_COMMENT, 60 * getCharWidth());
}

void XStackView::registerShortcuts(bool bState)
{
    Q_UNUSED(bState)
}

void XStackView::_headerClicked(qint32 nColumn)
{
    if (nColumn == COLUMN_ADDRESS) {
        if (getAddressMode() == MODE_ADDRESS) {
            setColumnTitle(COLUMN_ADDRESS, tr("Offset"));
            setAddressMode(MODE_OFFSET);
        } else if (getAddressMode() == MODE_OFFSET) {
            setColumnTitle(COLUMN_ADDRESS, tr("Address"));
            setAddressMode(MODE_ADDRESS);
        }

        adjust(true);
    } else if (nColumn == COLUMN_COMMENT) {
        if (g_modeComment == MODE_COMMENT_GENERAL) {
            setColumnTitle(COLUMN_COMMENT, tr("Address"));
            g_modeComment = MODE_COMMENT_ADDRESS;
        } else if (g_modeComment == MODE_COMMENT_ADDRESS) {
            setColumnTitle(COLUMN_COMMENT, QString("Ansi"));
            g_modeComment = MODE_COMMENT_ANSI;
        } else if (g_modeComment == MODE_COMMENT_ANSI) {
            setColumnTitle(COLUMN_COMMENT, QString("Unicode"));
            g_modeComment = MODE_COMMENT_UNICODE;
        } else if (g_modeComment == MODE_COMMENT_UNICODE) {
            setColumnTitle(COLUMN_COMMENT, QString("UTF8"));
            g_modeComment = MODE_COMMENT_UTF8;
        } else if (g_modeComment == MODE_COMMENT_UTF8) {
            setColumnTitle(COLUMN_COMMENT, tr("Comment"));
            g_modeComment = MODE_COMMENT_GENERAL;
        }

        adjust(true);
    }
}

void XStackView::_cellDoubleClicked(qint32 nRow, qint32 nColumn)
{
    Q_UNUSED(nRow)
    Q_UNUSED(nColumn)
    // TODO Edit
}

qint64 XStackView::getRecordSize(qint64 nOffset)
{
    Q_UNUSED(nOffset)

    return g_nBytesProLine;
}
