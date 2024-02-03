#include <string.h>
#include "storage_mgr.h"
#include "record_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#include "buffer_mgr.h"

typedef struct BM_PageHandle BM_PageHandle;
typedef struct BM_BufferPool BM_BufferPool;
typedef struct Expr Expr;
typedef struct RID RID;

typedef struct RecordManager
{
    BM_PageHandle pageHandler;
    int numOfTuples;
    RID recordNum;
    int pageID;
    BM_BufferPool buffMgr;
    int numOfTuplesScanned;
    Expr *check;
} record_mgr;

record_mgr *recordManager;
int x = 15;

void assignRecord(record_mgr *recMan, BM_PageHandle **pageinfo, BM_BufferPool **buffMgr)
{
  *pageinfo = &recMan->pageHandler;
  *buffMgr = &recMan->buffMgr;
  
}

void updatePage(RID *rec_pid, RM_TableData *tbData, Record *rec, BM_BufferPool *buffMgr, BM_PageHandle *pageinfo, char *val, char *info) {
    markDirty(buffMgr, pageinfo);
    int recordOffset = rec_pid->slot * getRecordSize(tbData->schema);
    val = info + recordOffset;
    *val = '#';
    val++;
    memmove(val, rec->data + 1, getRecordSize(tbData->schema) - 1);
    unpinPage(buffMgr, pageinfo);
	bool success = TRUE;
	if (unpinPage(buffMgr, pageinfo) != RC_OK) 
	{
        success = false;
		return;
    }
    recordManager->numOfTuples = recordManager->numOfTuples + 1;
    pinPage(buffMgr, pageinfo, 0);
}



int checkCase(int r_capacity, char *data)
{
 int pivot;
 char bytes;
 while (pivot < (PAGE_SIZE / r_capacity))
 {
	char bytes = data[pivot * r_capacity];
	if (bytes != '#')
		return pivot;
		pivot++;
 }
 return -1;
}

extern RC initRecordManager(void *mgmtData)
{
 bool check = TRUE;
 if(check)
 {
	initStorageManager();
 	return RC_OK;
 }
 else
	return RC_ERROR;
}

extern RC shutdownRecordManager()
{
 bool check = TRUE;
 if(check)
 {
	free(recordManager);
 	return RC_OK;
 }
 else
	return RC_ERROR;
}

void pageOperation(char *name, SM_FileHandle FileHandlerVar, char *data)
{
 bool check = TRUE;
 if(check)
 {
	createPageFile(name);
	openPageFile(name, &FileHandlerVar);
 }
 else
	return RC_ERROR;

if(check)
 {
	writeBlock(0, &FileHandlerVar, data);
	closePageFile(&FileHandlerVar);
 }
 else
	return RC_ERROR;

}

extern RC createTable(char *name, Schema *schema)
{
 bool chk=true;
 if(chk)
 {
	recordManager = (record_mgr *)malloc(sizeof(record_mgr));
 	initBufferPool(&recordManager->buffMgr, name, 100, RS_LRU, NULL);
 }
 else
 {
	return RC_ERROR;
 }
 
 SM_FileHandle fileHandl;
 char pgSiz[4096];
 char *bufferManHand = pgSiz;

 for(int c=0; c<2; c++)
 {
		*(int *)bufferManHand = c;
		bufferManHand += sizeof(int);
		
 }


 if(schema!=NULL)
 {
	*(int *)bufferManHand = schema->numAttr;
 	bufferManHand = bufferManHand + sizeof(int);
 	*(int *)bufferManHand = schema->keySize;
 	bufferManHand = bufferManHand + sizeof(int);
 }
 else
 {
	return RC_ERROR;
 }

 int i=0;
 do 
 {
		strncpy(bufferManHand, schema->attrNames[i], 15);
		bufferManHand += x;
		*(int *)bufferManHand = (int)(*schema).dataTypes[i];
		bufferManHand += sizeof(int);
		*(int *)bufferManHand = (int)(*schema).typeLength[i];
		bufferManHand += sizeof(int);
		i++;
 }
 while (i < (*schema).numAttr);
 bufferManHand -= sizeof(int);
 createPageFile(name);
 openPageFile(name, &fileHandl);
 writeBlock(0, &fileHandl, pgSiz);
 closePageFile(&fileHandl);
 return RC_OK;
}

extern RC openTable(RM_TableData *rmtable, char *name)
{
 rmtable->mgmtData = recordManager;
 rmtable->name = name;
 BM_BufferPool *buffPoolPtr = &(recordManager->buffMgr);
 BM_PageHandle *pageHandlePtr = &(recordManager->pageHandler);
 (pinPage(buffPoolPtr, pageHandlePtr, 0) != RC_OK) ? RC_ERROR : RC_OK;
 char* pgHandle = (char*)recordManager->pageHandler.data;
 recordManager->numOfTuples = *(int*)pgHandle;
 pgHandle += sizeof(int);
 recordManager->pageID = *(int*)pgHandle;
 pgHandle += sizeof(int);
 int counter = *(int*)pgHandle;
 pgHandle += sizeof(int);
 Schema* schema = (Schema*)malloc(sizeof(Schema));
 schema->numAttr = counter;
 schema->attrNames = (char**)malloc(sizeof(char*) * counter);
 1 ? (schema->dataTypes = (DataType*)malloc(sizeof(DataType) * counter),
     schema->typeLength = (int*)malloc(sizeof(int) * counter))
   : NULL;
 for (int check = 0; check < counter; check++)
    schema->attrNames[check] = (char*)malloc(x);
 int index = 0;
 while (index < schema->numAttr)
 {
    strncpy(schema->attrNames[index], pgHandle, x);
    pgHandle += x;
    schema->dataTypes[index] = *(int*)pgHandle;
    pgHandle += sizeof(int);
    schema->typeLength[index] = *(int*)pgHandle;
    pgHandle += sizeof(int);
    index++;
 }
 rmtable->schema = schema;
 forcePage(buffPoolPtr, pageHandlePtr);
}

extern RC closeTable(RM_TableData *rtable)
{
	switch (rtable != NULL) {
    case 1:

        shutdownBufferPool(&recordManager->buffMgr);
        rtable->mgmtData = NULL;
        free(rtable->schema);
        return RC_OK;
    case 0:
        return RC_ERROR;

	}
}

RC deleteTable(char *name)
{
	destroyPageFile(name);
}

RC insertRecord(RM_TableData *tbData, Record *rec)
{
	char *info, *val;
	record_mgr *recMan = tbData->mgmtData;
	if(tbData == NULL)
	{
		RC_ERROR;
	}
	RID *rec_pid = &rec->id;
	BM_PageHandle *pageinfo;
	BM_BufferPool *buffMgr;
	rec_pid->page = recMan->pageID;
	int slotNum = -1;
	while (slotNum == -1) {
		assignRecord(recMan, &pageinfo, &buffMgr);
		pinPage(buffMgr, pageinfo, rec_pid->page);
		info = pageinfo->data;
		slotNum = checkCase(getRecordSize(tbData->schema), info);
		if (slotNum == -1) {
			unpinPage(buffMgr, pageinfo);
			rec_pid->page++;
		}
	}
	rec_pid->slot = slotNum;
	rec_pid->slot = checkCase(getRecordSize(tbData->schema), info);
	if (rec_pid->slot == -1)
	{
		while(rec_pid->slot == -1)
 		{
			unpinPage(buffMgr, pageinfo);
			if (unpinPage(buffMgr, pageinfo))
			{
				RC_OK;
			}
			rec_pid->page = rec_pid->page + 1;
			pinPage(buffMgr, pageinfo, rec_pid->page);
			info = recordManager->pageHandler.data;
			if (pinPage(buffMgr, pageinfo, rec_pid->page))
			{
				RC_OK;
			}
			rec_pid->slot = checkCase(getRecordSize(tbData->schema), info);
 		}
	}
	
	updatePage(rec_pid, tbData, rec, &buffMgr, &pageinfo, val, info);
	return RC_OK;
}



extern int getNumTuples(RM_TableData *rel)
{
 if(rel == NULL)
 {
	RC_ERROR;
 }
 return ((record_mgr *)rel->mgmtData)->numOfTuples;
}

int getNumSlotsPerPage(BM_BufferPool* buffMgr) {
    return 100;
}

RC deleteRecord(RM_TableData *tbData, RID rID)
{
    record_mgr* recordMgrPtr = tbData->mgmtData;
    BM_PageHandle* pageAddress = &recordMgrPtr->pageHandler;
    BM_BufferPool* buffMgrPtr = &recordMgrPtr->buffMgr;
    RC rc = pinPage(buffMgrPtr, pageAddress, rID.page);
    if (rc != RC_OK) {
        return rc; 
    }
    recordMgrPtr->pageID = rID.page;
    char* check = recordMgrPtr->pageHandler.data;
    check += (rID.slot * getRecordSize(tbData->schema));
    if (rID.slot >= getNumSlotsPerPage(buffMgrPtr)) {
        unpinPage(buffMgrPtr, pageAddress);
        return RC_RM_NO_MORE_TUPLES; 
    }
    markDirty(buffMgrPtr, pageAddress);
	bool success = TRUE;
    unpinPage(buffMgrPtr, pageAddress);
	if (unpinPage(buffMgrPtr, pageAddress) != RC_OK) 
	{
        success = false;
		return;
    }
    *check = '*';
    return RC_OK;
}

RC updateRecord(RM_TableData *tbData, Record *record)
{
 	RID recordID = record->id;
	record_mgr *recordMgr = tbData->mgmtData;
	if (tbData == NULL || record == NULL) 
	{
		RC_ERROR;
	}
	
	BM_PageHandle *const page = &recordMgr->pageHandler;
	BM_BufferPool *const bufferPtr = &recordMgr->buffMgr;
	for (PageNumber page_num = 0; page_num < recordID.page; page_num++) {
		RC rc = pinPage(bufferPtr, page, page_num);
		if (rc != RC_OK) {
			break;
		}
	}

    RC rc = pinPage(bufferPtr, page, recordID.page);
    if (rc != RC_OK) {
        return rc;
    }
    int recordOffset = recordID.slot * getRecordSize(tbData->schema);
    char *tempBuffer = (char *)malloc(getRecordSize(tbData->schema));
    if (tempBuffer == NULL) {
        unpinPage(bufferPtr, page);
        return RC_ERROR;
    }
    markDirty(bufferPtr, page);
    memcpy(tempBuffer, page->data + recordOffset, getRecordSize(tbData->schema));
    memcpy(tempBuffer + 1, record->data + 1, getRecordSize(tbData->schema) - 1);
    memcpy(page->data + recordOffset, tempBuffer, getRecordSize(tbData->schema));
    free(tempBuffer);
    unpinPage(bufferPtr, page);
    return RC_OK;
}

RC getRecord(RM_TableData *tbData, RID rID, Record *recordInfo)
{
 BM_PageHandle *pageinfo;
 BM_BufferPool *buffMgr;

 record_mgr *recordMan = tbData->mgmtData;
 assignRecord(recordMan,&pageinfo,&buffMgr);
 pinPage(buffMgr, pageinfo, rID.page);
 char *info = recordMan->pageHandler.data;
 int recordSize = (getRecordSize(tbData->schema));
 info = info + (rID.slot * recordSize);
 recordInfo->id = rID;
 char *data1 = recordInfo->data;
 info=info+1;
 memcpy(++data1, info, recordSize-1 );
 unpinPage(buffMgr, pageinfo);
 return RC_OK;
}


RC startScan(RM_TableData *tbData, RM_ScanHandle *scanHandler, Expr *expr)
{
 record_mgr *record;
 if (expr == NULL || scanHandler == NULL || tbData == NULL)
        return RC_ERROR;
 openTable(tbData, "Scanning table");
 record_mgr *recordCheck = (record_mgr *)malloc(sizeof(record_mgr));
 if (recordCheck == NULL) 
    return RC_ERROR;
 do {
    recordCheck->numOfTuplesScanned = 0;
    recordCheck->recordNum.page = 1;
    recordCheck->recordNum.slot = 0;
    recordCheck->check = expr;
    scanHandler->mgmtData = recordCheck;
    record_mgr *record = tbData->mgmtData;
    record->numOfTuples = x;
    scanHandler->rel = tbData;
 } while (0);  
 return RC_OK;
}

RC next(RM_ScanHandle *scanHandler, Record *record)
{
 Schema *tableInformation;
 record_mgr *searchCondition;
 record_mgr *searchRecord;
 assignNext(scanHandler,&searchCondition,&searchRecord,&tableInformation); 
 Value *Output;
 Output = (Value *)malloc(sizeof(Value));
 int numflag= ((*searchRecord).numOfTuples == 0)?1:0;
 int chkflag=((*searchCondition).check == NULL)?1:0;
 switch(numflag)
 {
	case 1:
		return RC_RM_NO_MORE_TUPLES;
		break;

 }

 switch(chkflag)
 {
	case 1:
		return RC_ERROR;
		break;

 }	
 bool success = true;
 while (searchCondition->numOfTuplesScanned <= searchRecord->numOfTuples)
 {		
	int flag= ((*searchCondition).numOfTuplesScanned >= 0)?1:0;
	switch(flag)
	{
		case 1:
			(*searchCondition).recordNum.slot++;
			if ((*searchCondition).recordNum.slot >= (4096 / getRecordSize(tableInformation)))
			{
				(*searchCondition).recordNum.slot = 0;
				(*searchCondition).recordNum.page++;
			}
			break;
		case 2:
			(*searchCondition).recordNum.slot = 0;
			(*searchCondition).recordNum.page = 1;
			break;
		
	}	
	BM_BufferPool *const buffManager = &searchRecord->buffMgr;
	BM_PageHandle *const pginfo = &searchCondition->pageHandler;

	for (PageNumber page = 0; page < record->id.page; page++) {
		pinPage(buffManager, pginfo, page);
	}
    if(pinPage(buffManager, pginfo, record->id.page) != RC_OK) 
	{
    	success = false;
        return;
    }
    char *pageData = pginfo->data;
    size_t recordOffset = record->id.slot * getRecordSize(tableInformation);

    if (recordOffset >= PAGE_SIZE || recordOffset + getRecordSize(tableInformation) > PAGE_SIZE) 
	{
    	unpinPage(buffManager, pginfo);
    	success = false;
    	return;
    }

 	record->id.slot = record->id.slot;
    record->data = pageData + recordOffset;

   
    if (unpinPage(buffManager, pginfo) != RC_OK) 
	{
        success = false;
		return;
    }
		int recSize=getRecordSize(tableInformation);
		char *data = (*searchCondition).pageHandler.data;

		data = data + (*searchCondition).recordNum.slot * recSize;

		char *dp = record->data;
		*dp ='*';
	
		memcpy(++dp, data + 1, recSize-1);
		searchCondition->numOfTuplesScanned++;
		
		unpinPage(buffManager, pginfo);
		if (unpinPage(buffManager, pginfo) != RC_OK) 
		{
        	success = false;
			return;
    	}	
		return RC_OK;
 	}
	BM_BufferPool *const buffManager = &searchRecord->buffMgr;
	BM_PageHandle *const pginfo = &searchCondition->pageHandler;
	unpinPage(buffManager, pginfo);
	if (unpinPage(buffManager, pginfo) != RC_OK) 
			{
				success = false;
				return;
			}	
	return RC_RM_NO_MORE_TUPLES;
}

void updateSearchCond(record_mgr *searchCondition)
{
	bool success=true;

	if(searchCondition!=NULL)
	{
		(*searchCondition).recordNum.slot = 0;
 		(*searchCondition).recordNum.page = 1;
 		(*searchCondition).numOfTuplesScanned = 0;
	}
	else
	{
		success=false;
		return;
	}
}

void assignNext(RM_ScanHandle *scanHandlerObj, record_mgr **searchCondition, record_mgr **searchRecord, Schema **tableInformation) {
        *searchCondition = scanHandlerObj->mgmtData;
        *searchRecord = scanHandlerObj->rel->mgmtData;
        *tableInformation = scanHandlerObj->rel->schema;
}


RC closeScan(RM_ScanHandle *scanHandler)
{
	bool check = TRUE;
	if(check)
	{
		free(scanHandler->mgmtData);
		return RC_OK;
	}
	else
	return RC_ERROR;
}

typedef int (*DataHandler)(Schema *schema, int pointerValue);
int handleFloat(Schema *schema, int pointerValue) {
    return sizeof(float);
}
int handleInt(Schema *schema, int pointerValue) {
    return sizeof(int);
}
int handleString(Schema *schema, int pointerValue) {
    return schema->typeLength[pointerValue];
}
int handleBool(Schema *schema, int pointerValue) {
    return sizeof(bool);
}

int getRecordSize(Schema *schema) {
    int memorySize = 0;
    DataHandler handlers[] = {handleFloat, handleInt, handleString, handleBool};
    for (int pointerValue = 0; pointerValue < schema->numAttr; pointerValue++) {
        int dataTypeIndex = schema->dataTypes[pointerValue];
        memorySize += handlers[dataTypeIndex](schema, pointerValue);
    }
    memorySize++; 
    return memorySize;
}

RC freeSchema(Schema *schema)
{
 	bool check = TRUE;
	if(check)
	{
		free(schema);
		return RC_OK;
	}
	else
	return RC_ERROR;
}

Schema *createSchema(int attrCount, char **attrVar, DataType *type, int *length, int ptrSize, int *checkVar) {
	if(attrCount == NULL || ptrSize == NULL)
	{
		RC_ERROR;
	}
    Schema *schema = (Schema *)malloc(sizeof(Schema));
    schema->keyAttrs = (int *)malloc(ptrSize * sizeof(int));
    if (schema->keyAttrs == NULL) {
        free(schema);
        return NULL;
    }
    for (int i = 0; i < ptrSize; i++) {
        schema->keyAttrs[i] = checkVar[i];
    }
    schema->dataTypes = (DataType *)malloc(attrCount * sizeof(DataType));
    if (schema->dataTypes == NULL) {
        free(schema->keyAttrs);
        free(schema);
        return NULL;
    }
    for (int i = 0; i < attrCount; i++) {
        schema->dataTypes[i] = type[i];
    }
    schema->typeLength = (int *)malloc(attrCount * sizeof(int));
    if (schema->typeLength == NULL) {
        free(schema->dataTypes);
        free(schema->keyAttrs);
        free(schema);
        return NULL;
    }
    for (int i = 0; i < attrCount; i++) {
        schema->typeLength[i] = length[i];
    }
    schema->attrNames = (char **)malloc(attrCount * sizeof(char *));
    if (schema->attrNames == NULL) {
        free(schema->typeLength);
        free(schema->dataTypes);
        free(schema->keyAttrs);
        free(schema);
        return NULL;
    }
    for (int i = 0; i < attrCount; i++) {
        int len = strlen(attrVar[i]) + 1;
        schema->attrNames[i] = (char *)malloc(len * sizeof(char));
        if (schema->attrNames[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(schema->attrNames[j]);
            }
            free(schema->attrNames);
            free(schema->typeLength);
            free(schema->dataTypes);
            free(schema->keyAttrs);
            free(schema);
            return NULL;
        }
        strcpy(schema->attrNames[i], attrVar[i]);
    }
    schema->keySize = ptrSize;
    schema->numAttr = attrCount;
    return schema;
}

RC createRecord(Record **record, Schema *schema)
{
    Record *Obj_rec = (Record *)calloc(1, sizeof(Record));
    Obj_rec->data = (char *)malloc(getRecordSize(schema));
    Obj_rec->id.slot = -1;
    Obj_rec->id.page = -1;
    Obj_rec->data[0] = '*';
    Obj_rec->data[1] = '\0';
    *record = Obj_rec;
    return RC_OK;
}

RC freeRecord(Record *record)
{
	if (1){
		free(record);
		return RC_OK;
		}
	else {
		return RC_ERROR;
	}
}

RC getAttr(Record *record, Schema *schema, int attrNum, Value **value)
{
 int pos = 1;

 

    static const size_t dataTypeSizes[] = {
        sizeof(int),   
        sizeof(bool),  
        sizeof(float), 
        0              
    };

    for (int ctr = 0; ctr < attrNum; ctr++) {
        int dtype = schema->dataTypes[ctr];
        if (dtype == DT_STRING) {
            pos += schema->typeLength[ctr];
        } else {
            pos += dataTypeSizes[dtype];
        }
    }
 	char *rec = record->data + pos;
	Value *newValue = (Value *)calloc(1, sizeof(Value));
	char *dp = record->data + pos;
 if (attrNum == 1)
		(*schema).dataTypes[attrNum] = 1;
 else
		(*schema).dataTypes[attrNum] = (*schema).dataTypes[attrNum] ;

 if((*schema).dataTypes[attrNum]==DT_INT) 
 {
	newValue->dt = DT_INT;
	newValue->v.intV = *((int *)dp);

 }
 else if ((*schema).dataTypes[attrNum]==DT_FLOAT)
 {
	newValue->dt = DT_FLOAT;
	newValue->v.floatV = *((float *)dp);

 }
 else if((*schema).dataTypes[attrNum]==DT_BOOL)
 {
		newValue->dt = DT_BOOL;
		newValue->v.boolV = *((bool *)dp);

 }
 else if((*schema).dataTypes[attrNum]==DT_STRING)
 {	
	int newlen;
	(*newValue).dt = DT_STRING;
	newlen = (*schema).typeLength[attrNum];
	newValue->v.stringV = (char *)malloc(newlen + 1);
	for (int i = 0; i < newlen; i++) {
		newValue->v.stringV[i] = dp[i];
		}
		newValue->v.stringV[newlen] = '\0';
 }
 if(value ==NULL ){
	RC_WRITE_FAILED;
 }
 *value = newValue;
 return RC_OK;
}

RC setAttr(Record *record, Schema *schema, int attrNum, Value *value) {
    char *val = record->data;
	
	DataType attrType = schema->dataTypes[attrNum];
   	int pos = 1;

    static const size_t dataTypeSizes[] = {
        sizeof(int),   
        sizeof(bool),  
        sizeof(float), 
        0              
    };

    for (int ctr = 0; ctr < attrNum; ctr++) {
        int dataType = schema->dataTypes[ctr];
        if (dataType == DT_STRING) {
            pos += schema->typeLength[ctr];
        } else {
            pos += dataTypeSizes[dataType];
        }
    }

    val += pos;

    if (attrType == DT_BOOL) {
        memcpy(val, &(value->v.boolV), sizeof(bool));
        bool* boolPtr = (bool*)val;
		boolPtr++;
    } else if (attrType == DT_STRING) {
        int stringLength = schema->typeLength[attrNum];
        memcpy(val, value->v.stringV, stringLength);
       	char* charPtr = (char*)val;
		charPtr += stringLength;
    } else if (attrType == DT_FLOAT) {
		memcpy(val, &(value->v.floatV), sizeof(float));
		float* floatPtr = (float*)val;
		floatPtr++;
    } else if (attrType == DT_INT) {
    	int intValue = value->v.intV;
		memcpy(val, &intValue, sizeof(int));
		int* intPtr = (int*)val;
		intPtr++;
    }
    return RC_OK;
}