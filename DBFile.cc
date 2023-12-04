#include "DBFile.h"

DBFile::DBFile() {
}

DBFile::~DBFile() {
}

int DBFile::Create(const char *f_path, fType f_type, void *startup) {

    // Meta data file path.
    PathConfig *pathConfig = PathConfig::GetInstance();
    char *metadataPath = pathConfig->GetMetadataFilePath(f_path);

    // Open file for writing.
    ofstream fOut;
    fOut.open(metadataPath);

    fOut << f_type << "\n";

    switch (f_type) {
        case heap: {
            genericDBFile = new HeapDBFile();
            break;
        }
        case sorted: {
            SortParams *sortParams = (SortParams *)startup;
            fOut << sortParams->runLength << "\n";
            fOut << sortParams->myOrder->ToString() << "\n";

            genericDBFile = new SortedDBFile(sortParams);
            break;
        }
        default: {
            cerr << "Not yet implemented";
            exit(1);
        }
    }

    fOut.close();
    return genericDBFile->Create(f_path);
}

int DBFile::Open(const char *f_path) {

    // Meta data file path.
    PathConfig *pathConfig = PathConfig::GetInstance();
    char *metadataPath = pathConfig->GetMetadataFilePath(f_path);

    ifstream fIn;
    string readLine;

    fIn.open(metadataPath);
    if (!fIn.is_open()) return 0;

    getline(fIn, readLine);

    switch (stoi(readLine)) {
        case heap: {
            genericDBFile = new HeapDBFile();
            break;
        }
        case sorted: {
            SortParams *sortParams = new SortParams();
            sortParams->myOrder = new OrderMaker();

            getline(fIn, readLine);
            sortParams->runLength = stoi(readLine);

            getline(fIn, readLine);
            sortParams->myOrder->FromString(readLine);

            genericDBFile = new SortedDBFile(sortParams);
            break;
        }
        default:
            cerr << "Not yet implemented";
            return 0;
    }

    fIn.close();
    return genericDBFile->Open(f_path);
}

void DBFile::Load(Schema &f_schema, const char *loadpath) {
    genericDBFile->Load(f_schema, loadpath);
}

void DBFile::Add(Record &rec) {
    genericDBFile->Add(rec);
}

void DBFile::MoveFirst() {
    genericDBFile->MoveFirst();
}

int DBFile::GetNext(Record &fetchme) {
    return genericDBFile->GetNext(fetchme);
}

int DBFile::GetNext(Record &fetchme, CNF &cnf, Record &literal) {
    return genericDBFile->GetNext(fetchme, cnf, literal);
}

int DBFile::Close() {
    return genericDBFile->Close();
}
