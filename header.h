#define RECORDSIZE 512
#define NAMSIZE 100

union record 
{
    char charptr[RECORDSIZE];
struct header
{
    char path[NAMSIZE];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char linkflag;
    char arch_linkname[NAMSIZE];
}header;
};