#include "cachelab.h"
#include "unistd.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

int getopt(int argc, char *const argv[], const char *optstring);

extern char *optarg;
extern int optind, opterr, optopt;

// 缓存行
typedef struct
{
    int sign; // 有效位
    int mark; // 标志t
    int count; // 时间戳，用来实现LRU
} line;

int two2n(int n)
{
    int product = 1;
    for (int i = 0; i < n; i++)
        product *= 2;
    return product;
}

// 从原始输入行中取出内存地址并返回
char *split_address(char *address)
{
    char *front = strchr(address, ' ') + 1;
    char *end = strchr(address, ',');
    end[0] = '\0';
    return front;
}

// 将十六进制字符转换成二进制串
char *hex2bi(char *hex)
{
    char *temp, new = '\0';
    int len = strlen(hex);
    temp = (char *)malloc(sizeof(char) * (4 * len + 1)); // 分配保存二进制字符串的空间

    for (int i = 0; i < len; i++)
    {
        new = hex[i];
        switch (new)
        {
        case '0':
            strcat(temp, "0000");
            break;
        case '1':
            strcat(temp, "0001");
            break;
        case '2':
            strcat(temp, "0010");
            break;
        case '3':
            strcat(temp, "0011");
            break;
        case '4':
            strcat(temp, "0100");
            break;
        case '5':
            strcat(temp, "0101");
            break;
        case '6':
            strcat(temp, "0110");
            break;
        case '7':
            strcat(temp, "0111");
            break;
        case '8':
            strcat(temp, "1000");
            break;
        case '9':
            strcat(temp, "1001");
            break;
        case 'A':
        case 'a':
            strcat(temp, "1010");
            break;
        case 'B':
        case 'b':
            strcat(temp, "1011");
            break;
        case 'C':
        case 'c':
            strcat(temp, "1100");
            break;
        case 'D':
        case 'd':
            strcat(temp, "1101");
            break;
        case 'E':
        case 'e':
            strcat(temp, "1110");
            break;
        case 'F':
        case 'f':
            strcat(temp, "1111");
            break;
        }
    }
    return temp;
}

// 将二进制字符串转换为十进制数字
int bi2de(char *bistring)
{
    int i, sum = 0;
    int len = strlen(bistring);
    for (i = 0; i < len; i++)
    {
        sum = (bistring[i] - 48 + sum * 2);
    }
    return sum;
}

// 从原始输入中分离出内存地址对应的组数s0，标志t0
void seperate_st(char *origin_address, int *s0, int *t0, int s, int b)
{
    int len = 0;
    char *address0 = NULL;

    address0 = hex2bi(split_address(origin_address)); // 得到二进制的内存地址
    len = strlen(address0) - b; // 去除低b位
    address0[len] = '\0';

    *s0 = bi2de(&address0[len - s]); // 取出组数s0并转为十进制

    address0[len - s] = '\0';
    *t0 = bi2de(address0); // 取出标志t0并转为十进制
    free(address0); // 由于二进制地址是动态分配的，取出需要的信息后应归还堆栈空间
}

// eviction
void evict(int t0, int e, line *sett)
{
    int most = 0, num = 0;

    for (int i = 0; i < e; i++)
    {
        if (sett[i].count > most) // 找到留存时间最久的数据
        {
            most = sett[i].count;
            num = i;
        }
    }
    sett[num].count = 0;
    sett[num].mark = t0;
    return;
}

// 缓存操作的主程序
void write2cache(char *filename, int v, int s, int e, int b, int *miss, int *hit, int *eviction)
{
    FILE *fp = NULL;
    line **cache = NULL, *sett = NULL;
    char origin_address[100], signal = '\0';
    int miss_count = *miss, hit_count = *hit, eviction_count = *eviction;
    int s0 = 0, t0 = 0;
    int i, j;
    int set_num = two2n(s);

    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("fail to open %s\n", filename);
        return;
    }

    // 动态分配缓存空间
    cache = (line **)malloc(sizeof(line *) * set_num);
    for (i = 0; i < set_num; i++)
    {
        cache[i] = (line *)malloc(sizeof(line) * e);
    }

    // 初始化缓存
    for (i = 0; i < set_num; i++)
    {
        for (j = 0; j < e; j++)
        {
            cache[i][j].sign = 0;
            cache[i][j].count = 0;
        }
    }

    while (fgets(origin_address, 100, fp) != NULL)
    {
        origin_address[strlen(origin_address) - 1] = '\0'; // 去除原本地址的\n

        // 获取缓存操作的种类
        signal = origin_address[0];
        if (signal == 'I')
            continue;
        signal = origin_address[1];

        if (v)
            printf("%s ", &origin_address[1]);

        // 获取组数s0，标志t0
        seperate_st(&origin_address[1], &s0, &t0, s, b);
        sett = cache[s0];

        // 遍历组sett中的每一行
        for (i = 0; i < e; i++)
        {
            if (sett[i].sign == 0) // 如果有空行
            {
                sett[i].sign = 1;
                sett[i].mark = t0;
                sett[i].count = 0;

                if (v)
                {
                    printf("miss ");
                }
                miss_count++;
                break;
            }
            else
            {
                if (sett[i].mark == t0) // 检查是否命中
                {
                    sett[i].count = 0;
                    hit_count++;
                    for (j = i + 1; j < e; j++) // 让后面的行时间戳+1
                    {
                        sett[j].count++;
                    }
                    if (v)
                    {
                        printf("hit ");
                    }
                    break;
                }
            }
            sett[i].count++;
        }
        if (i == e) // 如果每一行都不空且不命中
        {
            miss_count++;
            eviction_count++;
            evict(t0, e, sett);

            if (v)
                printf("miss eviction ");
        }
        if (signal == 'M')
        {
            hit_count++;
            if (v)
                printf("hit ");
        }
        if (v)
            printf("\n");
    }

    *miss = miss_count;
    *hit = hit_count;
    *eviction = eviction_count;
    fclose(fp);
    return;
}

int main(int argc, char *argv[])
{
    int ch = -1, s = 0, e = 0, b = 0, miss = 0, hit = 0, eviction = 0, v = 0;
    char *filename = NULL;
    opterr = 0;

    while ((ch = getopt(argc, argv, "vhs:E:b:t:")) != -1)
    {
        switch (ch)
        {
        case 'v':
            v = 1;
            break;
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            e = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            filename = optarg;
            break;
        default:
            printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n"
                   "\t• -h: Optional help flag that prints usage info\n"
                   "\t• -v: Optional verbose flag that displays trace info\n"
                   "\t• -s <s>: Number of set index bits (S = 2^s is the number of sets)\n"
                   "\t• -E <E>: Associativity (number of lines per set)\n"
                   "\t• -b <b>: Number of block bits (B = 2^b is the block size)\n"
                   "\t• -t <tracefile>: Name of the valgrind trace to replay\n");
            return 0;
        }
    }

    write2cache(filename, v, s, e, b, &miss, &hit, &eviction);

    printSummary(hit, miss, eviction);

    return 0;
}
