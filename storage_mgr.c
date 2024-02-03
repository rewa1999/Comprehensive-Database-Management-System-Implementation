#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "storage_mgr.h"


FILE *storageFilePtr;

extern void initStorageManager (void){
    storageFilePtr = NULL;
}

typedef struct {
    void* nullCheckStruct;
} NullChecker;

bool isNull(NullChecker* checker) {
    return checker->nullCheckStruct == NULL;
}


typedef struct {
    int variable;
} CustomChecker;

bool checkValue(CustomChecker checker, int valueToCheck) {
    return checker.variable == valueToCheck;
}

int caseCheck() {
    CustomChecker checker;
    checker.variable = 1;
    int flag;
    int valueToCheck = 1;
    if (checkValue(checker, valueToCheck)) {
        flag=1;
    } else {
        flag=0;
    }
    return 0;
}


RC createPageFile(char *storMFile)
{
    FILE *inputPtr = fopen(storMFile, "wb+");
    int pgPointFlag;
    pgPointFlag = isNull(inputPtr) ? 1 : pgPointFlag;
    caseCheck(pgPointFlag,1) ? RC_FILE_NOT_FOUND : RC_OK;
    SM_PageHandle *targetPagePtr = (SM_PageHandle *)calloc(4096, sizeof(char)); 
    if (fwrite(targetPagePtr, 4096, sizeof(char), inputPtr) >= 0) {
        fclose(inputPtr); 
        free(targetPagePtr);
        return RC_OK;
    }
    else if(fwrite(targetPagePtr, 4096, sizeof(char), inputPtr)<1)
    {
        return RC_WRITE_FAILED;
    }
}


RC openPageFile(char *storMFile, SM_FileHandle *fileHandSM)
{
    storageFilePtr = fopen( storMFile, "r");
    int storFileFlag;
    storFileFlag = (storageFilePtr==NULL)?1:0;    
    switch(storFileFlag) 
    {   
        case 1:
            return RC_FILE_NOT_FOUND;
            break;
    }
    (*fileHandSM).fileName = storMFile; 
    (*fileHandSM).curPagePos = 0; 
    struct stat storageMgrStruct;
    int tgtFile = fstat(fileno(storageFilePtr), &storageMgrStruct);
    switch(tgtFile)
    {
        case -1:
            return RC_READ_NON_EXISTING_PAGE;
        
        default:
            (*fileHandSM).mgmtInfo=storageFilePtr; 
            (*fileHandSM).totalNumPages = storageMgrStruct.st_size / PAGE_SIZE; 
            return RC_OK; 
            break;
    }   
}

RC closePageFile(SM_FileHandle *fileHandSM)
{   
    int filehandFlag=isNull(fileHandSM->mgmtInfo)?1:0;
    switch(filehandFlag)
    {   case 1:
            return RC_FILE_NOT_FOUND;
            break;
        case 0:
            fclose(fileHandSM->mgmtInfo);
            storageFilePtr = NULL;
            return RC_OK; 
            break;   
    }
}

RC destroyPageFile(char *storMFile)
{
    int remStat = remove(storMFile);
    caseCheck(remStat,1) ? RC_DESTROY_ERROR : RC_OK;
}

RC readBlock(int pagePosition, SM_FileHandle *smfilehandle, SM_PageHandle smPageHandle)
{
    int totPgFlag=smfilehandle->totalNumPages;
    if (totPgFlag < pagePosition)
        return RC_READ_NON_EXISTING_PAGE;
    else
        storageFilePtr = fopen(smfilehandle->fileName, "r");
    long offst = pagePosition * PAGE_SIZE;
    int storFilptr=isNull(storageFilePtr)?0:storFilptr;
    storFilptr = caseCheck(storFilptr, 1) ? RC_FILE_NOT_FOUND : fseek(storageFilePtr, offst, SEEK_SET);
    int charSize = sizeof(char);
    switch(storFilptr)
    {
        case 0:
            fread(smPageHandle, charSize, 4096, storageFilePtr);
            printf("Post read pos of the Pointer : %ld\n", ftell(storageFilePtr));
            smfilehandle->curPagePos = ftell(storageFilePtr);
            printf("Pointer successfuly set at %ld\n", ftell(storageFilePtr));
            break;

        default:
            exit(1);
            break;
    }
    int closePg = fclose(storageFilePtr);
    caseCheck(closePg, 1) ? 1 : RC_OK;
}


int getBlockPos(SM_FileHandle * fhandle)
{
    bool condition = true;
    do {
    if ( fhandle) {
        char blockPos[200];
        sprintf(blockPos, "getBlockPos(): Block position is %d",  fhandle->curPagePos);
        return  fhandle->curPagePos;
    } else {
        return 0;
    }
    } while (condition);
}

RC readFirstBlock(SM_FileHandle *fil_Handler, SM_PageHandle sm_pgH)
{
    int constZero = 0;
    return readBlock(constZero, fil_Handler, sm_pgH);
}

RC readPreviousBlock(SM_FileHandle *fil_Handler, SM_PageHandle sm_pgH)
{
    int calByPage = (fil_Handler->curPagePos / 4096) - 1;
    return readBlock(calByPage, fil_Handler, sm_pgH);
}

RC readCurrentBlock(SM_FileHandle *fil_Handler, SM_PageHandle sm_pgH)
{
    int currentTgtPage;
    currentTgtPage = (fil_Handler->curPagePos / 4096);
    return readBlock(currentTgtPage, fil_Handler, sm_pgH);
    int result = readBlock(currentTgtPage, fil_Handler, sm_pgH);
    if (result != 0) {
        int checkpoint;
        checkpoint = 1;
    }
}

RC readNextBlock(SM_FileHandle *fil_Handler, SM_PageHandle sm_pgH)
{
    int nextTgt;
    nextTgt = (fil_Handler->curPagePos / 4096) + 1;
    if (!nextTgt) {
        int lastPage;
        lastPage = 1;
    }
    return readBlock(nextTgt, fil_Handler, sm_pgH);
}


RC readLastBlock(SM_FileHandle *fil_Handler, SM_PageHandle sm_pgH)
{
    int lastPagePtr;
    int lastPagePtrConst = 1;
    int lastPagePtrVar = fil_Handler->totalNumPages;
    lastPagePtr = lastPagePtrVar - lastPagePtrConst;
    return readBlock(lastPagePtr, fil_Handler, sm_pgH);
}

RC writeBlock(int index, SM_FileHandle *fileHand, SM_PageHandle memoryHand)
{   
    FILE *file_ptr_pvt;

    int exists = (access(fileHand->fileName, F_OK) != -1);
    
    int result =caseCheck( exists,0)? RC_FILE_NOT_FOUND: RC_OK;
    
    int ind = (index < 0)?1:0;
    switch(ind)
    {   case 1:
            return RC_READ_NON_EXISTING_PAGE;
            break;
    }
    
    
    file_ptr_pvt = fopen(fileHand->fileName, "r+");
    caseCheck(fseek(file_ptr_pvt, (index * PAGE_SIZE), SEEK_SET),0)?0:RC_WRITE_FAILED;
    
    int chSize = sizeof(char);
    fwrite(memoryHand, chSize, 4096, file_ptr_pvt);
    fileHand->curPagePos = ftell(file_ptr_pvt);
    if(fileHand->curPagePos == ftell(file_ptr_pvt))
    {
        fclose(file_ptr_pvt); 
    }
    
    return RC_OK;
}

RC writeCurrentBlock(SM_FileHandle *fileHand, SM_PageHandle memoryHand)
{
    int check = fileHand->curPagePos;
    switch (check)
    {
        case -1: 
            return RC_PAGE_ERROR;
        default:
            return writeBlock(fileHand->curPagePos, fileHand, memoryHand);
    }
}
RC appendEmptyBlock(SM_FileHandle *file_hand)
{
    char resultBlock[PAGE_SIZE];
    int exists = (access(file_hand->fileName, F_OK) != -1);
    caseCheck(exists,1)? writeBlock(file_hand->totalNumPages, file_hand, resultBlock) :RC_FILE_NOT_FOUND ;
}



RC ensureCapacity(int nPg, SM_FileHandle *fileHand)
{
    int exists = (access(fileHand->fileName, F_OK) != -1);
    int result = caseCheck(exists,1)? 1 : 0;
    char dmy[PAGE_SIZE];
    memset(dmy, 0, PAGE_SIZE);
    int x = 0; 
    int y = 1;
    
    if (!result){
        return 1;
    }
    
    else if (result)
    {
        if (nPg > fileHand->totalNumPages)
        {
             
            while (x < PAGE_SIZE) {
                dmy[x] = 0;
                x+=1;
            }

           
            int remainingPages = nPg - fileHand->totalNumPages;
            if (remainingPages > 0) {
                while (y <= remainingPages) {
                if (fwrite(dmy, PAGE_SIZE, 1, fileHand->fileName) != 1) {
            
                    break; 
                    }
                y++;
                }   
            }
        }
    
        return RC_OK;
    }
}