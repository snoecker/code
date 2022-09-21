#include<stdio.h>
#include<time.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>

#define LINE_BUFFER_SIZE 80


typedef struct {
    unsigned long  packedData;
    unsigned short packedTZData;
} packedDateTime;

typedef struct NODE {
    packedDateTime pdtData;
    struct NODE *left;
    struct NODE *right;
} NODE;


NODE* searchTree(NODE **tree, packedDateTime *pdtData) {
    
    /* see if root node exists first */
    if (!(*tree)) {
        return NULL;
    }

    if (pdtData->packedData < (*tree)->pdtData.packedData){
        searchTree(&((*tree)->left),pdtData);
    }
    else if (pdtData->packedData > (*tree)->pdtData.packedData){
        searchTree(&((*tree)->right),pdtData);
    }
    else if (pdtData->packedData == (*tree)->pdtData.packedData){
        if (pdtData->packedTZData != (*tree)->pdtData.packedTZData){
            searchTree(&((*tree)->right),pdtData);
        }
        else 
            return *tree;
    }
}

void insertNode(NODE **tree, packedDateTime *pdtData){
    NODE *temp = NULL;
    
    if (!(*tree)) {
        temp = (NODE *)malloc(sizeof(NODE));
        temp->left = temp->right = NULL;
        temp->pdtData.packedData=pdtData->packedData;
        temp->pdtData.packedTZData=pdtData->packedTZData;
        *tree = temp;
        return;
    }
    if (pdtData->packedData < (*tree)->pdtData.packedData){
        insertNode(&(*tree)->left, pdtData);
    } else if (pdtData->packedData > (*tree)->pdtData.packedData){
        insertNode(&(*tree)->right, pdtData);
    } else if (pdtData->packedData == (*tree)->pdtData.packedData){
        insertNode(&(*tree)->right, pdtData);
    }
}

void deltree(NODE * tree)
{
    if (tree)
    {
        deltree(tree->left);
        deltree(tree->right);
        free(tree);
    }
}

bool checkRange(int number, int minValue, int maxValue) {
    return number >= minValue && number <= maxValue;
}

packedDateTime* validateString(char *buffer) {

    char year_buf[10],extra[20],tz_value;
    int year,month,day,hour,min,sec,timeZone,tz_sign,tz_hour,tz_min;
    int retval;
    packedDateTime* pdtPacket = NULL;
    
    /* VERY simple sanity check*/
    if (strlen(buffer)<20) {
        printf("%s too short\n",buffer);
        return NULL;
    }
    /*sprintf(lineBuffer,"%04d-%02d-%02dT%02d:%02d:%02d",year,month,day,hour,min,sec);*/
     
    retval = sscanf(buffer,"%4s-%d-%dT%d:%d:%d%s",year_buf,&month,&day,&hour,&min,&sec,extra);
    if (retval != 7) {
        #ifdef DEBUG
        printf("%s missing args\n",buffer);
        #endif
        return NULL;
    }
    /*validate first six args */
    if (strlen(year_buf)!=4) {
        #ifdef DEBUG
        printf("%d Year too short\n",year);
        #endif
        return NULL;
    }
    year = atoi(year_buf);
    if (!checkRange(year,0,9999)) {
        #ifdef DEBUG
        printf("%d Year off\n",year);
        #endif
        return NULL;
    }
    if (!checkRange(month,1,12)) {
        #ifdef DEBUG
        printf("%d month off\n",month);
        #endif
        return NULL;
    }
    if (!checkRange(day,1,31)) {
        #ifdef DEBUG
        printf("%d day off\n",day);
        #endif
        return NULL;
    }
    if (!checkRange(hour,0,23)) {
        #ifdef DEBUG
        printf("%d hour off\n",hour);
        #endif
        return NULL;
    }
    if (!checkRange(min,0,59)) {
        #ifdef DEBUG
        printf("%d min off\n",min);
        #endif
        return NULL;
    }
    if (!checkRange(sec,0,59)) {
        #ifdef DEBUG
        printf("%d sec off\n",sec);
        #endif
        return NULL;
    }
    /* sprintf(tz_buf,"%c%02d:%02d",c,tz_hour,tz_min); */         
    if (strlen(extra)==0) {
        return NULL;
    }
    switch(extra[0])
    {
        case 'Z':
            timeZone=0;
            break;
        
        case '+':
        case '-':
            timeZone=1;
            retval = sscanf(extra,"%c%d:%d",&tz_value,&tz_hour,&tz_min);
            if (retval != 3) {
                #ifdef DEBUG
                printf("%s missing TZ args\n",extra);
                #endif
                return NULL;
            }
            break;

        default:
            return NULL;
    }

    if (timeZone == 1) {
        if (!checkRange(tz_hour,0,23)) {
            #ifdef DEBUG
            printf("%d tz_hour off\n",tz_hour);
            #endif
            return NULL;
        }
        if (!checkRange(tz_min,0,59)) {
            #ifdef DEBUG
            printf("%d tz_min off\n",tz_min);
            #endif
            return NULL;
        }
    }
    /*sprintf(lineBuffer,"%04d-%02d-%02dT%02d:%02d:%02d",year,month,day,hour,min,sec);*/

    /* Our time date value is valid - Pack it up */
    pdtPacket = malloc(sizeof(packedDateTime));
    if(!pdtPacket) {
        return NULL;
    }

    pdtPacket->packedData = 0;
    pdtPacket->packedData |= (unsigned long)(year  & 0xffff); /* byte 0 & 1*/
    pdtPacket->packedData |= (unsigned long)(month & 0xff) << 16;
    pdtPacket->packedData |= (unsigned long)(day   & 0xff) << 24;
    pdtPacket->packedData |= (unsigned long)(hour  & 0xff) << 32;
    pdtPacket->packedData |= (unsigned long)(min   & 0xff) << 40;
    pdtPacket->packedData |= (unsigned long)(sec   & 0xff) << 48;
    pdtPacket->packedData |= (unsigned long)(timeZone & 0xff) << 56; /*byte 7*/

    pdtPacket->packedTZData = 0;
    pdtPacket->packedTZData |= (tz_hour  & 0xff);
    pdtPacket->packedTZData |= (tz_min   & 0xff) << 8;

    return pdtPacket;
}

int main() {


    FILE *fp_readData,*fp_writeData;
    char lineBuffer[LINE_BUFFER_SIZE];
    packedDateTime *packedResult;
    NODE *root = NULL;

    /* Open two file streams, one for reading, one for writing validated output with no duplicates */
    fp_readData = fopen("input","r");
    if (!fp_readData) {
        fprintf(stderr,"File Error opening input data");
        exit(1);
    }

    fp_writeData = fopen("output","w");
    if (!fp_writeData) {
        fprintf(stderr,"File Error opening output data");
        exit(1);
    }

    while(fgets(lineBuffer,LINE_BUFFER_SIZE,fp_readData)) {
        #ifdef DEBUG
        printf("TRYING %s\n",lineBuffer);
        #endif
        packedResult=validateString(lineBuffer);
                    

        if(!packedResult)
            continue;
        #ifdef DEBUG
        printf("SEARCHING %s\n",lineBuffer);
        #endif

        if(!searchTree(&root, packedResult)) {
            #ifdef DEBUG
            printf("INSERTRING %s\n",lineBuffer);
            #endif
            insertNode(&root, packedResult);
            fprintf(fp_writeData,"%s",lineBuffer);
        }
        free(packedResult);
    }

    /* tidy up here */
    deltree(root);
    fclose(fp_writeData);
    fclose(fp_readData);

    return 0;

}