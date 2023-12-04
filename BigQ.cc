#include "BigQ.h"

BigQ::BigQ(Pipe &in, Pipe &out, OrderMaker &sortOrder, int runlen) {
    // Initialize worker thread Params object.
    WorkerThreadParams *threadParams = new WorkerThreadParams();
    threadParams->inputPipe = &in;
    threadParams->outputPipe = &out;
    threadParams->sortOrder = &sortOrder;
    threadParams->runLength = runlen;

    // Create the worker thread and the params object.
    pthread_create(&workerThread, NULL, TPMMS, (void *) threadParams);
}

BigQ::~BigQ() {
}

// Two-Pass Multiway Merge Sort
void *TPMMS(void *threadParams) {
    WorkerThreadParams *workerThreadParams = (WorkerThreadParams *) threadParams;
    InitializeWorkerThreadParams(workerThreadParams);

    // Phase-1 (Generate sorted runs of runLength pages)
    GenerateRun(workerThreadParams);
 
    // Now records are sorted in individual run, so each run is fed to a priority queue which compares records removed from each run; This operation will sort all the runs and flushed to the output pipe
    // Phase-2 (Merge sorted runs)
    MergeRuns(workerThreadParams);

    CleanUp(workerThreadParams);
}

void InitializeWorkerThreadParams(WorkerThreadParams *threadParams) {
    // Create buffer page array to store current runs' records
    Page *currentRunPages = new(std::nothrow) Page[threadParams->runLength];
    if (currentRunPages == NULL) {
        cerr << "ERROR : Not enough memory. EXIT !!!\n";
        exit(1);
    }

    threadParams->currentRunPages = currentRunPages;
    threadParams->currentRunPageNumber = 0;
    threadParams->numberOfRuns = 0;
    threadParams->isOverflowed = false;

    // Create temporary file for storing runs.
    sprintf(threadParams->bigQFileName, "%d.bin", pthread_self());
    threadParams->bigQFile.Open(0, threadParams->bigQFileName);
}

/*
* generate runs by flushing records from input pipe
*/
void GenerateRun(WorkerThreadParams *threadParams)
{
    Record *nextRecord = new Record();

    while (threadParams->inputPipe->Remove(nextRecord)) {
        if (!AddRecordToCurrentRun(threadParams, nextRecord)) {
            CreateRun(threadParams);
            AddRecordToCurrentRun(threadParams, nextRecord);
        }
    }
    do {
        CreateRun(threadParams);
    } while (threadParams->isOverflowed);
}

/*
 * Add record to pages; if added, return 1 else 0 that's when all pages are filled with records
 */
int AddRecordToCurrentRun(WorkerThreadParams *threadParams, Record *nextRecord) {
    Page *currentRunPage = &threadParams->currentRunPages[threadParams->currentRunPageNumber];
    if (!currentRunPage->Append(nextRecord)) {
        if (threadParams->currentRunPageNumber + 1 == threadParams->runLength) {
            return 0;
        } else {
            currentRunPage = &threadParams->currentRunPages[++threadParams->currentRunPageNumber];
        }
        currentRunPage->Append(nextRecord);
    }
    return 1;
}

void CreateRun(WorkerThreadParams *threadParams) {
    SortAndStoreCurrentRun(threadParams);
    ClearCurrentRun(threadParams);
}

/*
* Sorting all current run's records and store in a temporary file
*/
void SortAndStoreCurrentRun(WorkerThreadParams *threadParams) {
    // Using priority queue for in memory sort.
    priority_queue<Record *, vector<Record *>, RecordComparator> pq(threadParams->sortOrder);
    LoadCurrentRunPriorityQueue(threadParams, pq);

    // Get the records from the priority queue and add sorted pages of records in the BigQFile.
    WritePriorityQueueContentToBigQFile(threadParams, pq);
}

/*
* Transfer all the current run's records to the priority queue.
*/
void LoadCurrentRunPriorityQueue(WorkerThreadParams *threadParams,
                                 priority_queue<Record *, vector<Record *>, RecordComparator> &pq) {
    for (int i = 0; i <= threadParams->currentRunPageNumber; i++) {
        Page *currentRunPage = &threadParams->currentRunPages[i];
        Record *temp = new Record();
        while (currentRunPage->GetFirst(temp)) {
            pq.push(temp);
            temp = new Record();
        }
        currentRunPage->EmptyItOut();
    }
}

/*
* Write records from priority queue based on the comparator
*/
void WritePriorityQueueContentToBigQFile(WorkerThreadParams *threadParams,
                                         priority_queue<Record *, vector<Record *>, RecordComparator> &pq) {
    threadParams->isOverflowed = false;
    Page *bufferPage = new Page();
    // Get next empty page offset of BigQFile.
    off_t currentPageIndexOfBigQFile =
            threadParams->bigQFile.GetLength() - 2 < 0 ? 0 : threadParams->bigQFile.GetLength() - 1;
    off_t maxRunPageNumber = currentPageIndexOfBigQFile + threadParams->runLength - 1;
    int currentRunPageNumberOfOverflowRecords = 0;

    while (!pq.empty()) {
        // Overflow condition
        if (currentPageIndexOfBigQFile > maxRunPageNumber) {
            if (!(&threadParams->currentRunPages[currentRunPageNumberOfOverflowRecords])->Append(pq.top())) {
                currentRunPageNumberOfOverflowRecords++;
            } else {
                pq.pop();
            }
        }
            // If the current buffer is full, write this buffer to temp file
        else if (!bufferPage->Append(pq.top())) {
            threadParams->bigQFile.AddPage(bufferPage, currentPageIndexOfBigQFile++);
            bufferPage->EmptyItOut();
        } else {
            pq.pop();
        }
    }

    // If data is not over flow, store the last page in the BigQFile.
    if (currentPageIndexOfBigQFile <= maxRunPageNumber) {
        threadParams->bigQFile.AddPage(bufferPage, currentPageIndexOfBigQFile);
    } else {
        threadParams->isOverflowed = true;
    }
}

void ClearCurrentRun(WorkerThreadParams *threadParams) {
    threadParams->numberOfRuns++;
    threadParams->currentRunPageNumber = 0;
}

void MergeRuns(struct WorkerThreadParams *threadParams) {
    priority_queue<PriorityQueueItem, vector<PriorityQueueItem>, RecordComparator> pq(threadParams->sortOrder);

    LoadMergeRunPriorityQueue(threadParams, pq);

    LoadOutputPipeWithPriorityQueueData(threadParams, pq);
}

/*
* Each run is gathered from bigq file by fetching the first page and added to priority queue
*/
void LoadMergeRunPriorityQueue(WorkerThreadParams *threadParams,
                               priority_queue<PriorityQueueItem,
                                       vector<PriorityQueueItem>,
                                       RecordComparator> &pq) {
    PriorityQueueItem *priorityQueueItem = new PriorityQueueItem[threadParams->numberOfRuns];
    for (int i = 0; i < threadParams->numberOfRuns; i++) {
        priorityQueueItem[i].currentPageNumber = i * threadParams->runLength;
        priorityQueueItem[i].maxPageNumberOfCurrentRun =
                std::min((off_t) priorityQueueItem[i].currentPageNumber + threadParams->runLength - 1,
                         threadParams->bigQFile.GetLength() - 2);

        priorityQueueItem[i].page = new Page();
        threadParams->bigQFile.GetPage(priorityQueueItem[i].page, priorityQueueItem[i].currentPageNumber);
        priorityQueueItem[i].head = new Record();
        priorityQueueItem[i].page->GetFirst(priorityQueueItem[i].head);
        pq.push(priorityQueueItem[i]);
    }
}

/*
* Record is flushed from priority queue using a comparator; If a page has emptied its records, it's corresponding run will push the next page to priority queue
*/
void LoadOutputPipeWithPriorityQueueData(WorkerThreadParams *threadParams,
                                         priority_queue<PriorityQueueItem,
                                                 vector<PriorityQueueItem>, RecordComparator> &pq) {
    while (!pq.empty()) {
        PriorityQueueItem item = pq.top();
        pq.pop();
        threadParams->outputPipe->Insert(item.head);
        if (!item.page->GetFirst(item.head)) {
            if (item.currentPageNumber + 1 <= item.maxPageNumberOfCurrentRun) {
                item.currentPageNumber++;
                threadParams->bigQFile.GetPage(item.page, item.currentPageNumber);
                item.page->GetFirst(item.head);
                pq.push(item);
            }
        } else {
            pq.push(item);
        }
    }
}

void CleanUp(WorkerThreadParams *threadParams) {
    // Close and delete the bigq file.
    threadParams->bigQFile.Close();
    remove(threadParams->bigQFileName);

    delete[] threadParams->currentRunPages;

    // Shut down output pipe.
    if (threadParams->outputPipe) {
        threadParams->outputPipe->ShutDown();
    }
}