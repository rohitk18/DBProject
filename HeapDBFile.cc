#include "HeapDBFile.h"

HeapDBFile::HeapDBFile()
{
    currentWritePageNum = 0;
    writeBufferPage.EmptyItOut();
}

HeapDBFile::~HeapDBFile()
{
}

void HeapDBFile::SwitchToWriteMode()
{
    if (isWriteMode)
        return;

    writeBufferPage.EmptyItOut();
    currentWritePageNum = GetLengthInPages() <= 0 ? 0 : GetLengthInPages() - 1;
    GetPageFromDataFile(writeBufferPage, currentWritePageNum);
    isWriteMode = true;
}

void HeapDBFile::SwitchToReadMode()
{
    if (!isWriteMode)
        return;
    AddPageToDataFile(writeBufferPage, currentWritePageNum);
    isWriteMode = false;
}

void HeapDBFile::AddToDBFile(Record &addme)
{
    if (writeBufferPage.Append(&addme))
        return;

    AddPageToDataFile(writeBufferPage, currentWritePageNum++);
    writeBufferPage.Append(&addme);
}

int HeapDBFile::GetNextFromDBFile(Record &fetchme)
{
    return GetRecordFromReadBufferPage(fetchme);
}

int HeapDBFile::GetNextFromDBFile(Record &fetchme, CNF &cnf, Record &literal)
{
    while (GetRecordFromReadBufferPage(fetchme))
        if (comparisonEngine.Compare(&fetchme, &literal, &cnf))
            return 1;

    return 0;
}