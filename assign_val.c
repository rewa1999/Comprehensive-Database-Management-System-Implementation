#include "assign_val.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int augCall=0;

void assign_val(pgFr *fr, pgFr *pgPoint, int ct)
{
  bool var_fg = TRUE;
  int flag=(var_fg==TRUE)? 1:0;
  switch (flag)
  {
    case 1:
      pgPoint[ct].boolDirtyBit = (*fr).boolDirtyBit;
      pgPoint[ct].lruPage = (*fr).lruPage;

      var_fg = FALSE;
      pgPoint[ct].clientCount = (*fr).clientCount;

      pgPoint[ct].intPageReferenceNumber = (*fr).intPageReferenceNumber;
      augCall++;
      pgPoint[ct].storManFileDataHandler = (*fr).storManFileDataHandler;
      var_fg = TRUE;
      break;
   

  }
}

void setPageFrame(BM_BufferPool *const buffPool,int totalPages)
{
   pgFr *pframe = calloc(sizeof(pgFr), totalPages);

  int x = 0;
  do
  {
    memset(&pframe[x].storManFileDataHandler, 0, sizeof(pframe[x].storManFileDataHandler));
    memset(&pframe[x].lruPage, 0, sizeof(pframe[x].lruPage));
    memset(&pframe[x].intPageReferenceNumber, -1, sizeof(pframe[x].intPageReferenceNumber));
    memset(&pframe[x].lfuPage, 0, sizeof(pframe[x].lfuPage));
    memset(&pframe[x].boolDirtyBit, 0, sizeof(pframe[x].boolDirtyBit));
    memset(&pframe[x].clientCount, 0, sizeof(pframe[x].clientCount));
    x++;
  }while( x < totalPages);
  (*buffPool).mgmtData = pframe;

}

int pgN;
pgN = 0;
void writeInvoke(BM_BufferPool *const buffPool, pgFr *pagePointer, int poolPtr)
{
  SM_FileHandle fileHandler;
  auto pageFile = (*buffPool).pageFile;
  openPageFile(pageFile, &fileHandler);
  int pgRefNum = pagePointer[poolPtr].intPageReferenceNumber;
  auto dataHandler = pagePointer[poolPtr].storManFileDataHandler;
  writeBlock(pgRefNum, &fileHandler, dataHandler); 
  pagePointer[poolPtr].boolDirtyBit = 0;
  ++pgN;
}