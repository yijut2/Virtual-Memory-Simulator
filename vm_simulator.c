/* ICS 53
 * Project: Virtual Memory Simulator
 * Yi-Ju, Tsai & Hongji Wu     
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// global variable
int arg_count;
// If FIFO = 1, we will take FIFO as our page replacement algorithm, otherwise LRU;
bool FIFO;

struct MainMemory
{
    int vpn;
    unsigned int PhysicalPageNum;
    int FIFO;
    int LRU;
};

struct Disk
{
    int d_arr[8];
    int vpn;
    unsigned int DiskPageNum;
};

struct Page
{
    int p_arr[8];
    unsigned int valid;
    unsigned int dirty;
    unsigned int pagenum;
};

void showmain(int ppn);
void showdisk(int dpn);
void show_table();

struct MainMemory main_mem[4]; 
struct Disk dsk[8];
struct Page pageTable[8];

void build_MainMemory()
{
    int i;
    for(i=0; i<4; i++)
    {
        main_mem[i].PhysicalPageNum = i;
        main_mem[i].vpn = -1;
        main_mem[i].FIFO = -1;
        main_mem[i].LRU = -1;
    }
}

void build_Disk()
{
    int i;
    for(i=0; i<8; i++)
    {
        dsk[i].DiskPageNum = i;
        int j;
        for (j=0; j<8; j++)
        {
            dsk[i].d_arr[j] = -1;
        }
        dsk[i].vpn = i;
    }
}

void show_main(int ppn)
{
    int i;
    for (i=0;i<4; i++)
    {
        if (i==ppn)
        {
            int j;
            int vpn = main_mem[i].vpn;
            if (vpn == -1)
            {
                return;
            }
            for (j=0; j<8; j++)
            {
                // address = 8*i+j
                printf("%d:%d\n", 8*i+j, pageTable[vpn].p_arr[j]);
            }
            // printf("FIFO: %d\n",main_mem[i].FIFO);
            // printf("LRU: %d\n", main_mem[i].LRU);
            return;
        }
    }
}

void show_disk(int dpn)
{
    int i;
    for (i=0;i<8; i++)
    {
        if (i==dpn)
        {
            int j;
            // int vpn = dsk[i].vpn; // remove for 100% coverage
            for (j=0; j<8; j++)
            {
                printf("%d:%d\n", 8*i+j, dsk[dpn].d_arr[j]);
            }
            return;
        }
    }
}

void show_table()
{
    int i;
    for (i=0; i<8; i++)
    {
        // page in main memory
        if (pageTable[i].valid==1)
        {
            int j;
            for (j=0; j<4; j++)
            {
                if (i==main_mem[j].vpn)
                {
                    printf("%d:%d:%d:%d\n", i, pageTable[i].valid, pageTable[i].dirty, main_mem[j].PhysicalPageNum);
                }
            }
            
        }
        // page in disk
        else
        {
            int j;
            for (j=0; j<8; j++)
            {
                if (i==dsk[j].vpn)
                {
                    printf("%d:%d:%d:%d\n", i, pageTable[i].valid, pageTable[i].dirty, dsk[j].DiskPageNum);
                }
            }
        }
    }
}


void FIFO_Replace(int vpn)
{
    int i = 0;
    for(i = 0; i < 4; i++)
    {
        // Main memory is FULL, start replacement
        if (main_mem[i].FIFO == 4)
        {
            int victim = main_mem[i].vpn;
            if(pageTable[victim].dirty)
            {
                memcpy(dsk[victim].d_arr, pageTable[victim].p_arr,sizeof(int)*8);
            }

            main_mem[i].vpn = vpn; // original vpn being replaced by the new one
            main_mem[i].FIFO = 1; // new page FIFO initialized
            pageTable[vpn].valid = 1; // the page being access is marked as "in main memory"
            pageTable[victim].valid = 0; // victim be move ti disk so valid becomes 1
            pageTable[victim].dirty = 0; // dirty doesn't matter since victim page's valid=0;

        }
        else
        {
            main_mem[i].FIFO +=1;
        }
        
    }
}

void LRU_Replace(int vpn)
{
    int i = 0;
    for(i = 0; i < 4; i++)
    {
        if (main_mem[i].LRU == 4)
        {
            int victim = main_mem[i].vpn;
            if(pageTable[victim].dirty)
            {
                memcpy(dsk[victim].d_arr, pageTable[victim].p_arr,sizeof(int)*8);
            }
            main_mem[i].vpn = vpn;
            main_mem[i].LRU = 1;
            pageTable[vpn].valid = 1;
            pageTable[victim].valid = 0;
            pageTable[victim].dirty = 0;
        }
        else
        {
            main_mem[i].LRU +=1;
        } 
    }
}


void pageFault(int vpn)
{
    printf("A Page Fault Has Occured\n");
    pageTable[vpn].valid = 1;
    int i,j;
    for(i = 0; i < 4; i++)
    {
        if (main_mem[i].vpn == -1)
        {
            main_mem[i].vpn = vpn;
            main_mem[i].FIFO = 1;
            main_mem[i].LRU = 1;
            for(j = 0; j < 4; j++)
            {
                if(main_mem[j].vpn != vpn && main_mem[j].vpn != -1)
                {
                    main_mem[j].FIFO +=1;
                    main_mem[j].LRU += 1;
                }
            }
            break;  
        }
        // Main memory is FULL, start replacement
        else if (main_mem[i].FIFO == 4)
        {
            if(FIFO)
            {
                FIFO_Replace(vpn);
            }
            else
            {
                LRU_Replace(vpn);
            }
            break;
        }
    }
}


void incrementLRU(int vpn)
{
    int i;
    int ppn;
    for(i = 0; i< 4; i++)
    {
        if (main_mem[i].vpn == vpn)
        {
            ppn = i;
        }
    }
    for(i = 0; i < 4; i++)
    {
        if(main_mem[i].vpn != -1 && main_mem[i].LRU < main_mem[ppn].LRU)
        {
            main_mem[i].LRU += 1;
        }

    }
    main_mem[ppn].LRU = 1;
}

void write(int address, int value)
{
    int vpn = address/8;
    int add = address%8;
    if (pageTable[vpn].valid == 0)
    {
        pageFault(vpn);
    }
    incrementLRU(vpn);
    pageTable[vpn].p_arr[add] = value;
    pageTable[vpn].dirty = 1;
}

void read(int address)
{
    int vpn = address/8;
    int add = address%8;
    if (pageTable[vpn].valid == 0)
    {
        pageFault(vpn);
    }
    incrementLRU(vpn);
    printf("%d\n", pageTable[vpn].p_arr[add]);
}

void build_PageTable()
{
    int i,j;
    for (i = 0; i< 8; i++)
    {
        for (j = 0; j< 8;j++)
        {
            pageTable[i].p_arr[j] = -1;
        }
        pageTable[i].valid = 0;
    }
}

int eval_cmd(char* str)
{
    if (strcmp(str, "read")==0)
    {
        return 1;
    }
    if (strcmp(str, "write")==0)
    {
        return 2;
    }
    if (strcmp(str, "showmain")==0)
    {
        return 3;
    }
    if (strcmp(str, "showdisk")==0)
    {
        return 4;
    }
    if (strcmp(str, "showtable")==0)
    {
        return 5;
    }
    if (strcmp(str, "quit")==0)
    {
        return 0;
    }
    return 0;
}

void parse_cmd(char *cmd, char **argv)
{
    // the cmd will be parsed and store in argv list
    cmd[strlen(cmd)-1] = ' ';
    char *token = strtok(cmd, " ");
    while(token != NULL)
    {
        argv[arg_count++] = token;
        token = strtok(NULL," ");
    }
    return;
}


void run_vms(char *replacement)
{
    char cmdline[80];
    char *argv[80];
    //arg_count = 0;
    int cmd_type = 0;
    
    build_PageTable();
    if (strcmp(replacement, "LRU") == 0)
    {
        printf("Taking LRU\n");
        FIFO = 0;
    }
    else
    {
        printf("Taking FIFO\n");
        FIFO = 1;
    }
    
    build_MainMemory();
    build_Disk();

    do
    {
        arg_count = 0;
        printf("prompt> ");
        fgets(cmdline, 80, stdin);
        if (feof(stdin))
        {
            return;
        }
        parse_cmd(cmdline, argv);
        cmd_type = eval_cmd(argv[0]);
        switch (cmd_type)
        {
            // READ
            case (1):
            {
                int add = atoi(argv[1]);
                read(add);
                break;
            }
            // WRITE
            case (2):
            {
                int add = atoi(argv[1]);
                int value = atoi(argv[2]);
                // printf("add: %d value: %d\n", add, value);
                write(add,value);
                break;
            }
            // SHOW MAIN MEMORY
            case (3):
            {
                int ppn = atoi(argv[1]);
                // printf("showmain ppn: %d\n",ppn);
                show_main(ppn);
                break;
            }
            // SHOW DISK
            case (4):
            {
                int dpn = atoi(argv[1]);
                show_disk(dpn);
                break;
            }
            // SHOW TABLE
            case (5):
            {
                show_table();
                break;
            }
        }
    }while (cmd_type != 0);
}

int main(int argc, char *argv[])
{
    if(argc == 1)
    {
        run_vms(" ");
    }
    else
    {
        run_vms(argv[1]);
    }
    return 0;
}
