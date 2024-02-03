#include <stdio.h>
#include <stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "assign_val.c"
#include <math.h>


typedef struct {
    int maxPgFrs;
    int pageCounter;
    int writeCnt;
    int lastPg;
    int lastPtr;
    int lastEntry;
} PageInfo;

PageInfo pageInfo = {
    .maxPgFrs = 0,
    .pageCounter = 0,
    .writeCnt = 0,
    .lastPg = 0,
    .lastPtr = 0,
    .lastEntry = 0
};

void FIFO(BM_BufferPool *const buff_pool, pgFr *page)
{

  int pool_handler_f = pageInfo.pageCounter % pageInfo.maxPgFrs;
  int ct = 0;
  pgFr *frmPoint = (pgFr *)buff_pool->mgmtData;

  while (ct < pageInfo.maxPgFrs)
  {
    int flag;
    int clientCnt=frmPoint[pool_handler_f].clientCount;
    if (clientCnt== 0)
    {
      (frmPoint[pool_handler_f].boolDirtyBit != 1)? flag=1:writeInvoke(buff_pool, frmPoint, pool_handler_f);
      assign_val(page, frmPoint, pool_handler_f);
      break;
    }
    else
    {
      pool_handler_f=pool_handler_f + 1;
      flag=(pool_handler_f % pageInfo.maxPgFrs != 0)? 0:pool_handler_f;   
    }
    ct++;
  } 
}

 

void LRU(BM_BufferPool *const buffPool, pgFr *page)
{
  pgFr* pageFramePtr = (pgFr*)buffPool->mgmtData;
  int lrNum = pageFramePtr[0].lruPage;
  int flag;
  flag=1;
  int frameCount;
  frameCount = 0;

  for (int flag = 0; flag < pageInfo.maxPgFrs; flag++)
  {
    if (pageFramePtr[flag].lruPage <= lrNum)
    {
        frameCount = flag;
        lrNum = pageFramePtr[flag].lruPage;
    }
  }
  int dirtyBit = pageFramePtr[frameCount].boolDirtyBit;
  dirtyBit == 1 ? writeInvoke(buffPool, pageFramePtr, frameCount) : 0;
  assign_val(page, pageFramePtr, frameCount);
}

void CLOCK(BM_BufferPool *const buffPool, pgFr *page)
{
  pgFr *pgFrame = (pgFr *)buffPool->mgmtData;
  pageInfo.lastEntry = (pageInfo.lastEntry % pageInfo.maxPgFrs == 0) ? 0 : pageInfo.lastEntry;
  switch (pgFrame[pageInfo.lastEntry].boolDirtyBit) {
    case 1:
        writeInvoke(buffPool, pgFrame, pageInfo.lastEntry);
    case 0:
        assign_val(page, pgFrame, pageInfo.lastEntry);
        pageInfo.lastEntry += 1;
        break;
    default:
        break;
  }
}

void LFU(BM_BufferPool *const buffPool, pgFr *page)
{
  int leastFreq= (pageInfo.lastPtr) % pageInfo.maxPgFrs;
  pgFr *pgPtr = (pgFr *)buffPool->mgmtData;
  int leastFreqUsedRef = pgPtr[leastFreq].lfuPage;
  int leastFrUsedUpdatedIndex = leastFreq + 1;
  int ptr = leastFrUsedUpdatedIndex % pageInfo.maxPgFrs;

  int lfuMaxFrameCount = 0;
  while ( lfuMaxFrameCount < pageInfo.maxPgFrs)
  {
  int cs= (pgPtr[ptr].lfuPage > leastFreqUsedRef)?0:1;
    switch(cs)
    {
      case 1:
        leastFreq = ptr;
        leastFreqUsedRef= pgPtr[ptr].lfuPage;
        break;
    }
    ptr = (++ptr) % pageInfo.maxPgFrs;
    lfuMaxFrameCount=lfuMaxFrameCount +1;
  }
  (pgPtr[leastFreq].boolDirtyBit == 1)?writeInvoke(buffPool, pgPtr, leastFreq):RC_OK;

  assign_val(page, pgPtr, leastFreq);

  pageInfo.lastPtr = leastFreq + 1;
}


RC pinPage(BM_BufferPool *const buffPool, BM_PageHandle *const page, const PageNumber pNum)
{
  pgFr* pgFrame;
  SM_FileHandle fileHandleVar;
  bool bufferCheck = true;
  int pointer = 0;
  pgFrame = (pgFr*)buffPool->mgmtData;
  do
  {
    if (pgFrame[pointer].intPageReferenceNumber < 0 && pointer == 0)
    {
      openPageFile(buffPool->pageFile, &fileHandleVar);
      pgFrame[pointer].storManFileDataHandler = (SM_PageHandle)malloc(4096);
      readBlock(pNum, &fileHandleVar, pgFrame[pointer].storManFileDataHandler);
      pgFrame[pointer].intPageReferenceNumber = pNum;
      page->data = pgFrame[pointer].storManFileDataHandler;
      pgFrame[pointer].lruPage = 0;
      page->pageNum = pNum;
      pgFrame[pointer].lfuPage = 0;
      pgFrame[pointer].clientCount += 1;
      return RC_OK;
    }
    else
    {
      if (pgFrame[pointer].intPageReferenceNumber < 0)
      {
        openPageFile(buffPool->pageFile, &fileHandleVar);
        pgFrame[pointer].storManFileDataHandler = (SM_PageHandle)malloc(4096);
        readBlock(pNum, &fileHandleVar, pgFrame[pointer].storManFileDataHandler);
        pgFrame[pointer].intPageReferenceNumber = pNum;
        page->data = pgFrame[pointer].storManFileDataHandler;
        pgFrame[pointer].lruPage = 0;
        page->pageNum = pNum;
        pgFrame[pointer].lfuPage = 0;
        bufferCheck = false;
        pageInfo.pageCounter++;
        pageInfo.lastPg++;
        pgFrame[pointer].clientCount = 1;


      switch ((*buffPool).strategy)
      {
        case RS_CLOCK:
            pgFrame[pointer].lruPage = 1;
            break;
        
        case RS_LRU:
            pgFrame[pointer].lruPage = pageInfo.lastPg;
            break;

        default:
            break;
      }
      break;
    }

      else
      {
        if (pgFrame[pointer].intPageReferenceNumber == pNum)
        {
          pgFrame[pointer].clientCount = pgFrame[pointer].clientCount + 1;
          bufferCheck = false;
          (*page).data = pgFrame[pointer].storManFileDataHandler;
          (*page).pageNum = pNum;
          pageInfo.lastPg += 1;
          pageInfo.lastEntry += 1;
          int leastRecentPage;
          switch ((*buffPool).strategy) {
              case RS_LFU:
                leastRecentPage = pageInfo.lastPg + 1;
              case RS_LRU:
                leastRecentPage = pgFrame[pointer].lfuPage;
              case RS_CLOCK:
                leastRecentPage = 1;
              default:
                break;
          }
          switch ((*buffPool).strategy)
          {
            case RS_LFU:
              pgFrame[pointer].lfuPage = leastRecentPage;
              break;
            case RS_LRU:
            case RS_CLOCK:
              pgFrame[pointer].lruPage = leastRecentPage;
              break;
            default:
              break;
          }
          break;
        }
        else
        {
          printf("page %d was not found in the memory buffer.\n", pNum);
        }
      }
    }
    pointer += 1;
  } while (pointer < pageInfo.maxPgFrs);

  if (bufferCheck)
  {
    openPageFile((*buffPool).pageFile, &fileHandleVar);
    pgFr* pgFrame = (pgFr*)malloc(sizeof(pgFr));
    (*pgFrame).storManFileDataHandler = (SM_PageHandle)malloc(4096);
    readBlock(pNum, &fileHandleVar, (*pgFrame).storManFileDataHandler);
    page->data = pgFrame->storManFileDataHandler;
    page->pageNum = pNum;
    pageInfo.lastPg++;
    pageInfo.pageCounter++;
    pgFrame->clientCount = 1;
    pgFrame->intPageReferenceNumber = pNum;
    pgFrame->boolDirtyBit = 0;
    pgFrame->lfuPage = 0;
    char method;
    switch ((*buffPool).strategy) {
        case RS_LFU:
            method="LFU";
            LFU(buffPool, pgFrame);
            break;
        case RS_LRU:
            method="LRU";
            LRU(buffPool, pgFrame);
            break;
        case RS_FIFO:
            method="FIFO";
            FIFO(buffPool, pgFrame);
            break;
        case RS_CLOCK:
            method="CLOCK";
            CLOCK(buffPool, pgFrame);
            break;
        default:
            method="DEFAULT";
            break;
    }
  }
  return RC_OK;
}

PageNumber *getFrameContents(BM_BufferPool *const buffPool)
{
  int conCount = 0;
  pgFr *frmPoint = (pgFr *)(*buffPool).mgmtData;
  int *cntArr = malloc(sizeof(int)* pageInfo.maxPgFrs);
  
  do
  {
    cntArr[conCount] = frmPoint[conCount].intPageReferenceNumber;
    conCount++;
  }while( conCount < pageInfo.maxPgFrs);
  return cntArr;
}

int *getFixCounts(BM_BufferPool *const buffPool)
{ 
  int maxCnt = 0;
  pgFr *frmPoint = (pgFr *)(*buffPool).mgmtData;
  int *countArr = malloc(sizeof(int)* pageInfo.maxPgFrs);
  
  do
  {
    countArr[maxCnt] = frmPoint[maxCnt].clientCount;
    maxCnt++;
  }while( maxCnt < pageInfo.maxPgFrs);
  return countArr;
}

bool *getDirtyFlags(BM_BufferPool *const buffPool)
{
  int dirtFlg = 0;
  bool *dirty_flg = malloc(sizeof(bool)*pageInfo.maxPgFrs);
  pgFr *pgFrPtr = (pgFr *)(*buffPool).mgmtData;
 
  do
  {
    int isDirty = (pgFrPtr[dirtFlg].boolDirtyBit == 1)?1:0;
    switch(isDirty)
    {
      case 1:
        dirty_flg[dirtFlg] = true;
        break;
      
      case 0:
        dirty_flg[dirtFlg] = false;
        break;
    }
    dirtFlg=dirtFlg+1;
  }while( dirtFlg < pageInfo.maxPgFrs);
  return dirty_flg;
}

int getNumReadIO(BM_BufferPool *const buffPool)
{
  if(getDirtyFlags!=NULL){
    return (getDirtyFlags);
  }
  
}

int getNumWriteIO(BM_BufferPool *const buffPool)
{
  if(pgN!=NULL){
    return pgN;
  }
  
}

RC initBufferPool(BM_BufferPool *const buffPool, const char *const pgFileName, const int totalPages, ReplacementStrategy strategy, void *stratData)
{
  while (!(buffPool && pgFileName)) {
    return RC_ERROR;
  }
  pageInfo.lastPtr = 0;
  pgN = 0;
  pageInfo.pageCounter = 0;
  pageInfo.maxPgFrs = totalPages;
  pageInfo.lastEntry = 0;

  buffPool->pageFile = (char *)pgFileName;
  buffPool->numPages = totalPages;
  buffPool->strategy = strategy;
  setPageFrame(buffPool,totalPages);
  return RC_OK;
}


RC shutdownBufferPool(BM_BufferPool* const buffPool)
{
  switch ((buffPool != NULL) && buffPool)
  {
    case true:
       break;
  
    default:
      return RC_ERROR;
  }

  forceFlushPool(buffPool);
  pgFr* x;
  int maximumPageFrameames = pageInfo.maxPgFrs;
  int k = 0;
  x = (pgFr*)(*buffPool).mgmtData;

  do
  {
    return (x[k].clientCount != 0) ? RC_ERROR : RC_OK;
    k++;
  } while (k < maximumPageFrameames);
  int c = 0;
  do
  {
    free(x[c].storManFileDataHandler);
    c++;
  } while (c < maximumPageFrameames);
  (*buffPool).mgmtData = NULL;
   free(x);
  return RC_OK;
 
}



RC forceFlushPool(BM_BufferPool *const buffPool)
{
  pgFr *pageFrameamePointer;
  pageFrameamePointer = (pgFr *)(*buffPool).mgmtData;
  int x = 0;
  do
  {
    (pageFrameamePointer[x].clientCount == 0 && pageFrameamePointer[x].boolDirtyBit == 1) ? (writeInvoke(buffPool, pageFrameamePointer, x), x++) : x++;

  } while (x < pageInfo.maxPgFrs);

  return RC_OK;
}

RC markDirty(BM_BufferPool *const buffPool, BM_PageHandle *const pageHandler)
{
  int count = 0;
  pgFr* pageFramePointer = (pgFr*)buffPool->mgmtData;
  while (count < pageInfo.maxPgFrs && pageFramePointer[count].intPageReferenceNumber != (*pageHandler).pageNum)
  {
    count++;
  }
  return (count < pageInfo.maxPgFrs) ? (pageFramePointer[count].boolDirtyBit = 1, printf("\nPage number: %d, Set dirty var_fg to 1", pageFramePointer[count].intPageReferenceNumber), RC_OK): RC_ERROR;
}

RC unpinPage(BM_BufferPool *const buffPool, BM_PageHandle *const pageHandler)
{
  int check = 0;
  pgFr *pageFramePointer = (pgFr *)(*buffPool).mgmtData;
  do
  {
    switch (pageFramePointer[check].intPageReferenceNumber == pageHandler->pageNum)
    {
      case true:
        --(pageFramePointer[check].clientCount);
        printf("Page number: %d, decremented count by 1 done\n", pageFramePointer[check].intPageReferenceNumber);
        return RC_OK;
      
      default:
        break;
    }
    check++;
  } while (check < pageInfo.maxPgFrs);
  return RC_ERROR;
}


RC forcePage(BM_BufferPool *const bPool, BM_PageHandle *const pageHandler)
{ 
  pgFr *pageF;
  bool setflag;
  int i = 0;
  pageF = (pgFr *)(*bPool).mgmtData;
  while (i < pageInfo.maxPgFrs)
  {
    SM_FileHandle smFileHandlePointer;
    if (pageF[i].intPageReferenceNumber == (*pageHandler).pageNum) 
    {
      setflag = true;
    }
    else 
    {
      setflag = false;
    }
    switch (setflag)
    {
      case true:
        openPageFile(bPool->pageFile, &smFileHandlePointer);
        writeBlock(pageF[i].intPageReferenceNumber, &smFileHandlePointer, pageF[i].storManFileDataHandler);
        pageF[i].boolDirtyBit = 0;
        ++pageInfo.writeCnt;
        return RC_OK;

      default:
        i++;
        break;
    }
  }

  return RC_ERROR;
}
