#ifndef BIGQ_H
#define BIGQ_H

#include <pthread.h>
#include <iostream>
#include <queue>
#include "File.h"
#include "Record.h"
#include "Pipe.h"

using namespace std;

/**
* worker thread Params (members) during run operation
*/
struct WorkerThreadParams {
    Pipe *inputPipe;
    Pipe *outputPipe;
    OrderMaker *sortOrder;
    int runLength;

    File bigQFile;
    char bigQFileName[100];
    int numberOfRuns;

    Page *currentRunPages;
    int currentRunPageNumber;

    bool isOverflowed;
};

// for merging runs
struct PriorityQueueItem {
    Page *page;
    Record *head;

    int currentPageNumber;
    int maxPageNumberOfCurrentRun;  // run can be partially filled
};

struct RecordComparator {
    OrderMaker *sortOrder;

    RecordComparator(OrderMaker *sortorder) {
        this->sortOrder = sortorder;
    }

    bool operator()(Record *lhs, Record *rhs) {
        ComparisonEngine recordCompEngine;
        return recordCompEngine.Compare(lhs, rhs, this->sortOrder) > 0;
    }

    bool operator()(const PriorityQueueItem &lhs, const PriorityQueueItem &rhs) {
        ComparisonEngine queueItemCompEngine;
        return queueItemCompEngine.Compare(lhs.head, rhs.head, this->sortOrder) > 0;
    }
};


void *TPMMS(void *threadParams);

void InitializeWorkerThreadParams(WorkerThreadParams *threadParams);

void GenerateRun(WorkerThreadParams *threadParams);

int AddRecordToCurrentRun(WorkerThreadParams *threadParams, Record *nextRecord);

void CreateRun(WorkerThreadParams *threadParams);

void SortAndStoreCurrentRun(WorkerThreadParams *threadParams);

void LoadCurrentRunPriorityQueue(WorkerThreadParams *threadParams,
                                 priority_queue<Record *, vector<Record *>, RecordComparator> &pq);

void WritePriorityQueueContentToBigQFile(WorkerThreadParams *threadParams,
                                         priority_queue<Record *, vector<Record *>, RecordComparator> &pq);

void ClearCurrentRun(WorkerThreadParams *threadParams);

void MergeRuns(WorkerThreadParams *threadParams);

void LoadMergeRunPriorityQueue(WorkerThreadParams *threadParams,
                               priority_queue<PriorityQueueItem, vector<PriorityQueueItem>, RecordComparator> &pq);

void LoadOutputPipeWithPriorityQueueData(WorkerThreadParams *threadParams,
                                         priority_queue<PriorityQueueItem, vector<PriorityQueueItem>, RecordComparator> &pq);

void CleanUp(WorkerThreadParams *threadParams);

class BigQ {
private:
    pthread_t workerThread;

public:
    BigQ(Pipe &in, Pipe &out, OrderMaker &sortorder, int runlen);

    ~BigQ();
};

#endif