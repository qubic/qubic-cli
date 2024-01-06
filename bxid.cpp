/*

 BXID Archival Service Creation
 
build: g++ -I.. ../bxid.cpp -o bxid
usage:
./bxid calcbxid epoch tick srcid destid amount
./bxid makejson <json filename>

 example bxid calculation: ./bxid calcbxid 90 11867469 LZLDOEIBQWIUGGMZGOISLOAACDGAFVAMAYXSSJMLQBHSHWDBPMSDFTGAYRMN QHQPMJVNGZJGZDSQREFXHHAZFYPBIYDOTFAOTTWGYCWGTIRNGBVMKBGGNDDA 1521139
 outputs bxid: 397e947847ada93de80907d88a835419fb532b3ca1fd68b3c95ebab11cd24190
 
 
 The makejson file will use the logfile created by updated qubic-cli -getlogfromnode and generate a json output file
 this json file can be used directly as input to the charmed opensearch command.
 
 { "index" : { "_index": "bxid", "_id" : "397e947847ada93de80907d88a835419fb532b3ca1fd68b3c95ebab11cd24190" } }
 {"utime":"1707059413", "epoch":"90", "tick":"11867469", "type":"1", "src":"LZLDOEIBQWIUGGMZGOISLOAACDGAFVAMAYXSSJMLQBHSHWDBPMSDFTGAYRMN", "dest":"QHQPMJVNGZJGZDSQREFXHHAZFYPBIYDOTFAOTTWGYCWGTIRNGBVMKBGGNDDA", "amount": "1521139" }

 Install charmed opensearch following: https://charmhub.io/opensearch/docs/t-overview
 Once it is running you will have available a REST API server with amazing amounts of functionality: https://opensearch.org/docs/latest/api-reference/
 
 To add the json file entries to opensearch (of course change the <variables> to your setup:
 curl --cacert demo-ca.pem -XPOST https://<username>:<password>@<ipaddr>:9200/_bulk --data-binary @<jsonfile>  -H 'Content-Type: application/json'
 
 Once the bxid is added to the opensearch, then you can search directly by bxid (calculated using calcbxid)

 curl --cacert demo-ca.pem -XGET https://<username>:<password>@<ipaddr>:9200/bxid/_doc/397e947847ada93de80907d88a835419fb532b3ca1fd68b3c95ebab11cd24190
 {
   "_index": "bxid",
   "_id": "397e947847ada93de80907d88a835419fb532b3ca1fd68b3c95ebab11cd24190",
   "_version": 2,
   "_seq_no": 32754,
   "_primary_term": 1,
   "found": true,
   "_source": {
     "utime": "1707059413",
     "epoch": "90",
     "tick": "11867469",
     "type": "1",
     "src": "LZLDOEIBQWIUGGMZGOISLOAACDGAFVAMAYXSSJMLQBHSHWDBPMSDFTGAYRMN",
     "dest": "QHQPMJVNGZJGZDSQREFXHHAZFYPBIYDOTFAOTTWGYCWGTIRNGBVMKBGGNDDA",
     "amount": "1521139"
   }
 }

 opensearch even lets you search for any field!
 curl --cacert demo-ca.pem -XGET https://<username>:<password>@<ipaddr>:9200/bxid/_search?q=LZLDOEIBQWIUGGMZGOISLOAACDGAFVAMAYXSSJMLQBHSHWDBPMSDFTGAYRMN
 curl --cacert demo-ca.pem -XGET https://<username>:<password>@<ipaddr>:9200/bxid/_search?q=11867469
 curl --cacert demo-ca.pem -XGET https://<username>:<password>@<ipaddr>:9200/bxid/_search?q=1521139
 
 I do not know all the capabilities of opensearch but it seems to be able to scale up to handle large volumes
 redundantly.
 
 To create an archival service:
 
 1. poll qubic-cli -getlogfromnode every minute
 2. ./bxid makejson bxids.json
 3.  curl --cacert demo-ca.pem -XPOST https://<username>:<password>@<ipaddr>:9200/_bulk --data-binary @bxids.json -H 'Content-Type: application/json'
 4. move the logfile and bxids.json once the json is imported. bxids.json can be easily regenerated but the logfile cannot be obtained again
 
 Now to allow read only access to the archive is a matter to figure out how to create read only account in opensearch. This is left to the reader as with the above localhost access to the DB is available and for security it could make sense to insulate the opensearch DB with another layer above it, especially one like qubic-http that has additonal functionality.
*/


#include <cstring>
#include <vector>
#include <cstdlib>
#include "structs.h"
#include "connection.h"
#include "nodeUtils.h"
#include "logger.h"
#include "K12AndKeyUtil.h"
#include "keyUtils.h"
#include "walletUtils.h"
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "qubicLogParser.h"
#include "logger.h"
#define QU_TRANSFER 0
#define QU_TRANSFER_LOG_SIZE 72
#define ASSET_ISSUANCE 1
#define ASSET_ISSUANCE_LOG_SIZE 55
#define ASSET_OWNERSHIP_CHANGE 2
#define ASSET_OWNERSHIP_CHANGE_LOG_SIZE 119
#define ASSET_POSSESSION_CHANGE 3
#define ASSET_POSSESSION_CHANGE_LOG_SIZE 119
#define CONTRACT_ERROR_MESSAGE 4
#define CONTRACT_ERROR_MESSAGE_LOG_SIZE 4
#define CONTRACT_WARNING_MESSAGE 5
#define CONTRACT_INFORMATION_MESSAGE 6
#define CONTRACT_DEBUG_MESSAGE 7
#define CUSTOM_MESSAGE 255

std::string mylogTypeToString(uint8_t type){
    switch(type){
        case 0:
            return "QU transfer";
        case 1:
            return "Asset issuance";
        case 2:
            return "Asset ownership change";
        case 3:
            return "Asset possession change";
        case 4:
            return "Contract error";
        case 5:
            return "Contract warning";
        case 6:
            return "Contract info";
        case 7:
            return "Contract debug";
        case 255:
            return "Custom msg";
    }
    return "Unknown msg";
}

long long __year_to_secs(long long year, int *is_leap)
{
    if (year-2ULL <= 136) {
        int y = (int)year;
        int leaps = (y-68)>>2;
        if (!((y-68)&3)) {
            leaps--;
            if (is_leap) *is_leap = 1;
        } else if (is_leap) *is_leap = 0;
        return 31536000*(y-70) + 86400*leaps;
    }

    int cycles, centuries, leaps, rem;

    //if ( !is_leap )
    //    is_leap = &(int){0};
    cycles = (int)(year-100) / 400;
    rem = (year-100) % 400;
    if (rem < 0) {
        cycles--;
        rem += 400;
    }
    if (!rem) {
        *is_leap = 1;
        centuries = 0;
        leaps = 0;
    } else {
        if (rem >= 200) {
            if (rem >= 300) { centuries = 3; rem -= 300; }
            else { centuries = 2; rem -= 200; }
        } else {
            if (rem >= 100) { centuries = 1; rem -= 100; }
            else centuries = 0;
        }
        if (!rem) {
            *is_leap = 0;
            leaps = 0;
        } else {
            leaps = rem / 4U;
            rem %= 4U;
            *is_leap = !rem;
        }
    }

    leaps += 97*cycles + 24*centuries - *is_leap;

    return (year-100) * 31536000LL + leaps * 86400LL + 946684800 + 86400;
}

int __month_to_secs(int month, int is_leap)
{
    static const int secs_through_month[] = {
        0, 31*86400, 59*86400, 90*86400,
        120*86400, 151*86400, 181*86400, 212*86400,
        243*86400, 273*86400, 304*86400, 334*86400 };
    int t = secs_through_month[month];
    if (is_leap && month >= 2) t+=86400;
    return t;
}

long long __tm_to_secs(const struct tm *tm)
{
    int is_leap;
    long long year = tm->tm_year;
    int month = tm->tm_mon;
    if (month >= 12 || month < 0) {
        int adj = month / 12;
        month %= 12;
        if (month < 0) {
            adj--;
            month += 12;
        }
        year += adj;
    }
    long long t = __year_to_secs(year, &is_leap);
    t += __month_to_secs(month, is_leap);
    t += 86400LL * (tm->tm_mday-1);
    t += 3600LL * tm->tm_hour;
    t += 60LL * tm->tm_min;
    t += tm->tm_sec;
    return t;
}

uint32_t makeunixtime(int32_t year,uint8_t month,uint8_t day,uint8_t hour,uint8_t minute,uint8_t second)
{
    struct tm ts;
    memset(&ts,0,sizeof(ts));
    ts.tm_year = year - 1900;
    ts.tm_mon = month;
    ts.tm_mday = day;
    ts.tm_hour = hour;
    ts.tm_min = minute;
    ts.tm_sec = second;
    return((uint32_t)__tm_to_secs(&ts));
}

void getIdentityFromPublicKey(const uint8_t* pubkey, char* dstIdentity, bool isLowerCase)
{
    uint8_t publicKey[32] ;
    memcpy(publicKey, pubkey, 32);
    uint16_t identity[61] = {0};
    for (int i = 0; i < 4; i++)
    {
        unsigned long long publicKeyFragment = *((unsigned long long*)&publicKey[i << 3]);
        for (int j = 0; j < 14; j++)
        {
            identity[i * 14 + j] = publicKeyFragment % 26 + (isLowerCase ? L'a' : L'A');
            publicKeyFragment /= 26;
        }
    }
    unsigned int identityBytesChecksum;
    KangarooTwelve(publicKey, 32, (uint8_t*)&identityBytesChecksum, 3);
    identityBytesChecksum &= 0x3FFFF;
    for (int i = 0; i < 4; i++)
    {
        identity[56 + i] = identityBytesChecksum % 26 + (isLowerCase ? L'a' : L'A');
        identityBytesChecksum /= 26;
    }
    identity[60] = 0;
    for (int i = 0; i < 60; i++) dstIdentity[i] = identity[i];
}

void getPublicKeyFromIdentity(const char* identity, uint8_t* publicKey)
{
    unsigned char publicKeyBuffer[32];
    for (int i = 0; i < 4; i++)
    {
        *((unsigned long long*)&publicKeyBuffer[i << 3]) = 0;
        for (int j = 14; j-- > 0; )
        {
            if (identity[i * 14 + j] < 'A' || identity[i * 14 + j] > 'Z')
            {
                return;
            }

            *((unsigned long long*)&publicKeyBuffer[i << 3]) = *((unsigned long long*)&publicKeyBuffer[i << 3]) * 26 + (identity[i * 14 + j] - 'A');
        }
    }
    memcpy(publicKey, publicKeyBuffer, 32);
}

int32_t calcbxid0(uint8_t bxid[32],int32_t epoch,uint32_t tick,char *srcid,char *destid,uint64_t amount)
{
    // need to add txid, latest tick
    uint8_t buf[82];
    uint32_t tmp;
    memset(bxid,0,32);
    memset(buf,0,sizeof(buf));
    *(uint16_t *)&buf[0] = epoch;
    *(uint32_t *)&buf[2] = tick;
    tmp = (QU_TRANSFER << 24);
    tmp |= (32 + 32 + 8);
    *(uint32_t *)&buf[6] = tmp;
    getPublicKeyFromIdentity(srcid,&buf[10]);
    getPublicKeyFromIdentity(destid,&buf[42]);
    memcpy(&buf[74],&amount,8);
    if ( (0) )
    {
        int i;
        for (i=0; i<sizeof(buf); i++)
            printf("%02x",buf[i]);
        printf("buf\n");
    }
    KangarooTwelve(buf,sizeof(buf),bxid,32);
    return(0);
}

std::string myparseLogToString_type0(uint8_t bxid[32],uint32_t t,uint16_t epoch,uint32_t tick,uint8_t *ptr)
{
    char sourceIdentity[61] = {0};
    char destIdentity[61] = {0};
    char bxidstr[65];
    uint64_t amount;
    uint8_t cmpbxid[32];
    const bool isLowerCase = false;
    byteToHex(bxid, bxidstr, 32);
    getIdentityFromPublicKey(ptr, sourceIdentity, isLowerCase);
    getIdentityFromPublicKey(ptr+32, destIdentity, isLowerCase);
    memcpy(&amount, ptr+64, 8);
    //{ "index" : { "_index": "bxid", "_id" : "<bxid>" } }
    //{"utime": "<U>", "epoch": "<N>",  "tick": "<N>", "type": "<N>", "src":"<id>", "dest":"<id>", "amount":"<N>" }
    std::string result = "{ \"index\" : { \"_index\": \"bxid\", \"_id\" : \"" + std::string(bxidstr) + "\" } }\n{\"utime\":\"" + std::to_string(t) + "\", \"epoch\":\"" + std::to_string(epoch) + "\", \"tick\":\"" + std::to_string(tick) + "\", \"type\":\"1\", \"src\":\"" + std::string(sourceIdentity) + "\", \"dest\":\"" + std::string(destIdentity) + "\", \"amount\": \"" + std::to_string(amount) + "\" }";
    //std::string result = "from " + std::string(sourceIdentity) + " to " + std::string(destIdentity) + " " + std::to_string(amount) + "QU.";
    calcbxid0(cmpbxid,epoch,tick,sourceIdentity,destIdentity,amount);
    if ( memcmp(bxid,cmpbxid,sizeof(cmpbxid)) != 0 )
    {
        char tmpstr[65];
        byteToHex(cmpbxid,tmpstr,32);
        printf("ERROR mismatched bxid %s != %s\n",bxidstr,tmpstr);
    }
    return result;
}

std::string myparseLogToString_type1(uint8_t bxid[32],uint32_t t,int32_t epoch,uint32_t tick,uint8_t* ptr)
{
    char sourceIdentity[61] = {0};
    char name[8] = {0};
    char numberOfDecimalPlaces = 0;
    uint8_t unit[8] = {0};

    long long numberOfShares = 0;
    const bool isLowerCase = false;
    getIdentityFromPublicKey(ptr, sourceIdentity, isLowerCase);
    memcpy(&numberOfShares, ptr+32, 8);
    memcpy(name, ptr+32+8, 7);
    numberOfDecimalPlaces = ((char*)ptr)[32+8+7];
    memcpy(unit, ptr+32+8+7+1, 7);
    std::string result = std::string(sourceIdentity) + " issued " + std::to_string(numberOfShares) + " " + std::string(name)
                       + ". Number of decimal: " + std::to_string(numberOfDecimalPlaces) + ". Unit of measurement: "
                       + std::to_string(unit[0]) + "-"
                       + std::to_string(unit[1]) + "-"
                       + std::to_string(unit[2]) + "-"
                       + std::to_string(unit[3]) + "-"
                       + std::to_string(unit[4]) + "-"
                       + std::to_string(unit[5]) + "-"
                       + std::to_string(unit[6]);
    return result;
}

std::string myparseLogToString_type2_type3(uint8_t bxid[32],uint32_t t,int32_t epoch,uint32_t tick,uint8_t* ptr)
{
    char sourceIdentity[61] = {0};
    char dstIdentity[61] = {0};
    char issuerIdentity[61] = {0};
    char name[8] = {0};
    char numberOfDecimalPlaces = 0;
    char unit[8] = {0};
    long long numberOfShares = 0;
    const bool isLowerCase = false;
    getIdentityFromPublicKey(ptr, sourceIdentity, isLowerCase);
    getIdentityFromPublicKey(ptr+32, dstIdentity, isLowerCase);
    getIdentityFromPublicKey(ptr+64, issuerIdentity, isLowerCase);
    memcpy(&numberOfShares, ptr+96, 8);
    memcpy(name, ptr+96+8, 7);
    numberOfDecimalPlaces = ((char*)ptr)[96+8+7];
    memcpy(unit, ptr+96+8+7+1, 7);
    std::string result = "from " + std::string(sourceIdentity) + " to " + std::string(dstIdentity) + " " + std::to_string(numberOfShares) + " " + std::string(name)
                         + "(Issuer: " + std::string(issuerIdentity) + ")"
                         + ". Number of decimal: " + std::to_string(numberOfDecimalPlaces) + ". Unit of measurement: "
                         + std::to_string(unit[0]) + "-"
                         + std::to_string(unit[1]) + "-"
                         + std::to_string(unit[2]) + "-"
                         + std::to_string(unit[3]) + "-"
                         + std::to_string(unit[4]) + "-"
                         + std::to_string(unit[5]) + "-"
                         + std::to_string(unit[6]);
    return result;
}



void myprintQubicLog(FILE *jsonfp,uint8_t* logBuffer, int bufferSize)
{
    if (bufferSize == 0){
        LOG("Empty log\n");
        return;
    }
    if (bufferSize < 16){
        LOG("Buffer size is too small (not enough to contain the header), expected 16 | received %d\n", bufferSize);
        return;
    }
    uint32_t t;
    uint8_t bxid[32],bxid2[32],*end = logBuffer + bufferSize;
    while ( logBuffer < end )
    {
        // basic info
        uint8_t year = *((unsigned char*)(logBuffer + 0));
        uint8_t month = *((unsigned char*)(logBuffer + 1));
        uint8_t day = *((unsigned char*)(logBuffer + 2));
        uint8_t hour = *((unsigned char*)(logBuffer + 3));
        uint8_t minute = *((unsigned char*)(logBuffer + 4));
        uint8_t second = *((unsigned char*)(logBuffer + 5));
        t = makeunixtime(2000 + year,month,day,hour,minute,second);
        uint16_t epoch = *((unsigned short*)(logBuffer + 6));
        uint32_t tick = *((unsigned int*)(logBuffer + 8));
        uint32_t tmp = *((unsigned int*)(logBuffer + 12));
        uint8_t messageType = tmp >> 24;
        std::string mt = mylogTypeToString(messageType);
        uint32_t messageSize = (tmp << 8) >> 8;
        KangarooTwelve(logBuffer+6,10+messageSize, bxid, 32);
        logBuffer += 16;
        std::string humanLog = "null";
        switch(messageType){
            case QU_TRANSFER:
                if (messageSize == QU_TRANSFER_LOG_SIZE){
                    humanLog = myparseLogToString_type0(bxid,t,epoch,tick,logBuffer);
                } else {
                    LOG("Malfunction buffer size for QU_TRANSFER log\n");
                }
                break;
            case ASSET_ISSUANCE:
                if (messageSize == ASSET_ISSUANCE_LOG_SIZE){
                    humanLog = myparseLogToString_type1(bxid,t,epoch,tick,logBuffer);
                } else {
                    LOG("Malfunction buffer size for ASSET_ISSUANCE log\n");
                }
                break;
            case ASSET_OWNERSHIP_CHANGE:
                if (messageSize == ASSET_OWNERSHIP_CHANGE_LOG_SIZE){
                    humanLog = myparseLogToString_type2_type3(bxid,t,epoch,tick,logBuffer);
                } else {
                    LOG("Malfunction buffer size for ASSET_OWNERSHIP_CHANGE log\n");
                }
                break;
            case ASSET_POSSESSION_CHANGE:
                if (messageSize == ASSET_POSSESSION_CHANGE_LOG_SIZE){
                    humanLog = myparseLogToString_type2_type3(bxid,t,epoch,tick,logBuffer);
                } else {
                    LOG("Malfunction buffer size for ASSET_POSSESSION_CHANGE log\n");
                }
                break;
            // TODO: stay up-to-date with core node contract logger
            case CONTRACT_ERROR_MESSAGE:
            case CONTRACT_WARNING_MESSAGE:
            case CONTRACT_INFORMATION_MESSAGE:
            case CONTRACT_DEBUG_MESSAGE:
            case 255:
                break;
        }
        if ( messageType != QU_TRANSFER )
        {
            LOG("%u %02d-%02d-%02d %02d:%02d:%02d %u.%03d %s: %s\n", t,year, month, day, hour, minute, second, tick, epoch, mt.c_str(), humanLog.c_str());
            if (humanLog == "null")
            {
                char buff[1024] = {0};
                for (int i = 0; i < messageSize; i++)
                    sprintf(buff + i*2, "%02x", logBuffer[i]);
                LOG("Can't parse, original message: %s\n", buff);
            }
        }
        else
        {
            fprintf(jsonfp,"%s\n",humanLog.c_str());
            fflush(jsonfp);
        }
        logBuffer+= messageSize;
    }
}

void makejsonlog(char *jsonfname)
{
    FILE *fp,*jsonfp;
    int32_t len,bufsize = 1024*1024;
    uint8_t *buf = (uint8_t *)calloc(bufsize,1);
    if ( (jsonfp= fopen(jsonfname,"rb+")) == 0 )
        jsonfp = fopen(jsonfname,"wb");
    else fseek(jsonfp,0,SEEK_END);
    if ( jsonfp == 0 )
        jsonfp = stdout;
    if ( buf != 0 && (fp= fopen("logfile","rb")) != 0 )
    {
        while ( fread(&len,1,sizeof(len),fp) > 0 )
        {
            if ( len > bufsize )
            {
                printf("ERROR len.%d > bufsize %d\n",len,bufsize);
                break;
            }
            if ( fread(buf,1,len,fp) == len )
            {
                myprintQubicLog(jsonfp,buf,len);
            }
            else
            {
                printf("fread %d error\n",len);
                break;
            }
        }

        fclose(fp);
    }
    if ( buf != 0 )
        free(buf);
}

int main(int argc,char *argv[])
{
    if ( argc == 3 )
    {
        if ( strcmp(argv[1],"makejson") == 0 )
        {
            makejsonlog(argv[2]);
            return(0);
        }
    }
    else if ( argc == 7 )
    {
        if ( strcmp(argv[1],"calcbxid") == 0 )
        {
            uint8_t bxid[32];
            char bxidstr[65];
            calcbxid0(bxid,atoi(argv[2]),atoi(argv[3]),argv[4],argv[5],atol(argv[6]));
            byteToHex(bxid,bxidstr,32);
            printf("%s\n",bxidstr);
            return(0);
        }
    }
    else
    {
        printf("usage:\n%s makejson <json filename>\n%s calcbxid epoch tick srcid destid amount\n",argv[0],argv[0]);
        return(0);
    }
    return(-1);
}


