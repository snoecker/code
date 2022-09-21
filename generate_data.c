#include<stdio.h>
#include<time.h>
#include<string.h>
#include<stdlib.h>

char lineBuffer[80];

void fillOutString() {
    int year,month,day,hour,min,sec,timeZone,tz_sign,tz_hour,tz_min;

    year = rand() % 10000;
    month = rand() % 12 +1;
    day = rand() % 31 + 1;
    hour = rand() % 24;
    min =  rand() % 60;
    sec =  rand() % 60;
    timezone = rand() % 2;
    tz_sign = rand() % 2;
    tz_hour = rand() % 24;
    tz_min =  rand() % 60;

    sprintf(lineBuffer,"%04d-%02d-%02dT%02d:%02d:%02d",year,month,day,hour,min,sec);

    if (timezone) {
        char c = (tz_sign) ? '+' : '-';
        char tz_buf[20];

        sprintf(tz_buf,"%c%02d:%02d",c,tz_hour,tz_min);
        strcat(lineBuffer,tz_buf);
    } else {
        strcat(lineBuffer,"Z");
    }
}

int main(int argc, char *argv[]) {

    /* YYYY-MM-DDThh:mm:ssTZD
    Where:
    •	YYYY = four-digit year
    •	MM = two-digit month (01 through 12)
    •	DD = two-digit day of month (01 through 31)
    •	hh = two digits of hour (00 through 23)
    •	mm = two digits of minute (00 through 59)
    •	ss = two digits of second (00 through 59)
    •	TZD = time zone designator (“Z” for GMT or +hh:mm or -hh:mm)
    *
    *
    * 9999-02-31T12:34:56+12:34 */
    struct tm TimePlaceHolder;
    FILE *outputFile;

    srand(time(NULL));
    
    outputFile = fopen("input","w");
    
    fprintf(outputFile, "9999-02-31T12:34:56+12:34\n");
    
    for (int i=0;i<25;i++) {
        fillOutString();
        fprintf(outputFile,"%s\n",lineBuffer);
    }

    fclose(outputFile);
    return 0;

}