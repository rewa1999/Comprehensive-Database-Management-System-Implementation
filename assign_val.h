#ifndef ASSIGN_VAL_H
#define ASSIGN_VAL_H


typedef struct Page
{


  int boolDirtyBit;
  int clientCount;


  SM_PageHandle storManFileDataHandler;
  PageNumber intPageReferenceNumber;
  int hitNumber;


  int lruPage;
  int lfuPage;


  
} pgFr;

void assign_val(pgFr *fr, pgFr *pgPoint, int ct);

void setPageFrame(BM_BufferPool *const buffPool,int totalPages);

void restoreConstants(int totalPages);

void writeInvoke(BM_BufferPool *const buffPool, pgFr *pagePointer, int poolPtr);


#endif