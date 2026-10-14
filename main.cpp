#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdint>
#include <sys/stat.h>
#include "header.h"
#include "event.h"
#include <unistd.h>
#include <utime.h>

unsigned int calcChecksum(union record *r)
{
    memset(r->header.chksum, ' ', 8);
    unsigned int sum=0;
    unsigned char *p = (unsigned char*)r->charptr;
    for(int i=0; i<RECORDSIZE;i++)
    {
        sum +=p[i];
    }
    sprintf(r->header.chksum,"%06o ",(unsigned int)sum);
    r->header.chksum[7] = '\0';
    return sum;
}

void Creation(event& Event)
{
    int argc = Event.getArgc();
    for( int i=3;i<argc;i++)
    {
        const char* arg = Event.getArgument(i).c_str();
        union record file;
        memset(&file, 0, sizeof(file));
        strcpy(file.header.path, arg);
        struct stat st;
        stat(arg, &st);
        sprintf(file.header.mode, "%07o", (unsigned int)st.st_mode &07777);
        sprintf(file.header.uid, "%07o", (unsigned int)st.st_uid);
        sprintf(file.header.gid, "%07o", (unsigned int)st.st_gid);
        sprintf(file.header.size, "%011o",(unsigned int)st.st_size);
        sprintf(file.header.mtime, "%011o",(unsigned int)st.st_mtime);
        file.header.linkflag = '0';
        calcChecksum(&file);
        Event.getStream().write(file.charptr, RECORDSIZE);
        if(st.st_size>0 && S_ISREG(st.st_mode))
        {
            std::ifstream src(Event.getArgument(i),std::ios::binary);
            if(!src.is_open())
            {
                std::cerr<<"cant create file"<< Event.getArgument(i) <<std::endl;
            }
            char buf[4096];
            size_t total_read = 0;
            while(src.read(buf,sizeof(buf)))
            {
                Event.getStream().write(buf,src.gcount());
                total_read+=src.gcount();
            }
            if(src.gcount()>0)
            {
                Event.getStream().write(buf,src.gcount());
                total_read+=src.gcount();
            }
            size_t reminder = total_read % RECORDSIZE;
            if(reminder)
            {
                char zeros[RECORDSIZE]={0};
                Event.getStream().write(zeros,RECORDSIZE - reminder);
            }
        }
        if(Event.checkVerbose())
        std::cout << Event.getArgument(i)<<std::endl;
    }
    if (Event.checkEmptuBlocks())
    {
        char end[RECORDSIZE*2] = {0};
        Event.getStream().write(end,RECORDSIZE*2);
    }
}

static void Append(event& Event)
{
    Event.getStream().seekp(0,std::ios::end);
    std::streampos end = Event.getStream().tellp();
    std::streamoff offset = -(RECORDSIZE*2);
    std::streampos target = end+offset;
    Event.getStream().seekp(target);
    Creation(Event);
    char endBlocks[RECORDSIZE*2] = {0};
    Event.getStream().write(endBlocks,RECORDSIZE*2);
}

static void Extract(event& Event)
{
    union record rec;
    short emptyCount = 0;
    while(Event.getStream().read(rec.charptr, RECORDSIZE))
    {
        bool empty = true;
        for(int i=0; i<RECORDSIZE;i++)
        {
        if(rec.charptr[i] != 0)
        {
        empty = false;
        break;
        }
        }
    if(empty)
    {
        emptyCount++;
        if(emptyCount>=2)
            break;
        continue;
    }
    std::string fileName = rec.header.path;
    bool extractThis = Event.check3Argc();
    if(!extractThis)
    {
        for(int i=3; i<Event.getArgc(); i++)
        {
            if(fileName ==Event.getArgument(i))
            {
                extractThis = true;
                break;
            }
        }
    }
    unsigned int savedSum = strtol(rec.header.chksum,nullptr,8);
    unsigned int controlSum = calcChecksum(&rec);
    if(savedSum != controlSum)
    {
        std::cerr<<"file is broken!"<<std::endl;
        return;
    }
    size_t fileSize = strtol(rec.header.size, nullptr, 8);
    if (extractThis)
    {
        std::ofstream out;
        out.open(fileName, std::ios::binary | std::ios::trunc);
        if(!out.is_open())
        {
            std::cerr<<"cannot create file: " << fileName <<std::endl;
            continue;
        }
        char buffer[4096];
        size_t remaining = fileSize;
        while(remaining>0)
        {
            size_t toRead = std::min(sizeof(buffer), remaining);
            Event.getStream().read(buffer, toRead);
            size_t readBytes = Event.getStream().gcount();
            if (readBytes==0)
                break;
            if(out)
                out.write(buffer,readBytes);
            remaining-=readBytes;
        }
        long int offset = (RECORDSIZE -(fileSize % RECORDSIZE))%RECORDSIZE;
        if(offset>0)
            Event.getStream().seekg(offset, std::ios::cur);
        if(out)
        {
            out.close();
            mode_t mode = strtol(rec.header.mode,nullptr,8);
            chmod(fileName.c_str(),mode);
            time_t mtime = strtol(rec.header.mtime,nullptr,8);
            struct utimbuf time;
            time.actime = mtime;
            time.modtime = mtime;
            utime(fileName.c_str(), &time);
        }
        if(Event.checkVerbose())
            std::cout<<fileName<< std::endl;
    }
    else
    {
        size_t dataBlocks = (fileSize+RECORDSIZE -1)/RECORDSIZE;
        Event.getStream().seekg(dataBlocks * RECORDSIZE, std::ios::cur);
    }
}
}

void List(event& Event)
{
    union record rec;
    short emptyCount = 0;
    while(Event.getStream().read(rec.charptr, RECORDSIZE))
    {
        bool empty = true;
        for(int i = 0; i < RECORDSIZE; i++)
        {
            if(rec.charptr[i] != 0)
            {
                empty = false;
                break;
            }
        }
        if(empty)
        {
            emptyCount++;
            if(emptyCount >= 2)
                break;
            continue;
        }
        std::string fileName = rec.header.path;
        bool listThis = Event.check3Argc();
        if(!listThis)
        {
            for(int i = 3; i < Event.getArgc(); i++)
            {
                if(fileName == Event.getArgument(i))
                {
                    listThis = true;
                    break;
                }
            }
        }
 
        unsigned int savedSum = strtol(rec.header.chksum, nullptr, 8);
        unsigned int controlSum = calcChecksum(&rec);
        if(savedSum != controlSum)
        {
            std::cerr << "file is broken: " << fileName << std::endl;
            size_t fileSize = strtol(rec.header.size, nullptr, 8);
            size_t dataBlocks = (fileSize + RECORDSIZE - 1) / RECORDSIZE;
            Event.getStream().seekg(dataBlocks * RECORDSIZE, std::ios::cur);
            continue;
        }
 
        size_t fileSize = strtol(rec.header.size, nullptr, 8);
        if(listThis)
        {
            if(Event.checkVerbose())
            {
                mode_t mode = strtol(rec.header.mode, nullptr, 8);
                std::cout << std::oct << mode << std::dec<< " " << fileSize << " " << rec.header.mtime<< " " << fileName << std::endl;
            }
            else
            {
                std::cout << fileName << std::endl;
            }
        }
        size_t dataBlocks = (fileSize + RECORDSIZE - 1) / RECORDSIZE;
        Event.getStream().seekg(dataBlocks * RECORDSIZE, std::ios::cur);
    }
}

int main(int argc, char* argv[])
{
event CoreEvent={argc,argv};
CoreEvent.Parser();
switch(CoreEvent.getOption())
{
    case parameters::create:
        Creation(CoreEvent);
        break;
    case parameters::append:
        Append(CoreEvent);
        break;
    case parameters::extract:
        Extract(CoreEvent);
        break;
    case parameters::list:
        List(CoreEvent);
        break;
    default: break;
}
return 0;
}
