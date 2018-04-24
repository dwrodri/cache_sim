/**
 * \author Derek Rodriguez
 * \date
 * \copyright
 */
#include <stdio.h>
#include <stdlib.h>
#include <libio.h>
#include <stdbool.h>


/**
 * ONLY WORKS FOR 32 BIT UNSIGNED INTS!
 * @param v input which MUST be a 32-bit unsigned power of 2
 * @return trailing zeros of a power of two very fast, without intrinsics. Kinda like log2.
 */
static inline unsigned int fast_log2(unsigned int v)
{
    static const unsigned int b[] = {0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0, 0xFF00FF00, 0xFFFF0000};
    unsigned int r = (v & b[0]) != 0;
    for (unsigned short i = 4; i > 0; i--) // unroll for speed...
    {
        r |= ((v & b[i]) != 0) << i;
    }

    return r;
}

/**
 * Metadata structure for a cache line of variable length.
 * Lines are grouped into sets, the amount of which is determined by SET_COUNT
 */
typedef struct c_meta{
    bool valid; /**< tracks whether line has been written to (true) or not */
    int tag; /**< tag inside set */
    int set_id; /**< id for which set contains the line */
};

static const unsigned int CACHE_SIZE = 32768; /**< Size of cache in bytes (32KiB)*/
static const unsigned short SET_COUNT = 8; /**< The cache has 8 sets.*/

int main(int argc, char** argv) {

    FILE *infile; /**< input file descriptor for testing in IDE */
    infile = fopen("testfiles/gcc10k.txt", "r");

    unsigned short l_size; /**< size of line collected from input args (default to 64) */
    l_size = (unsigned short) ((argc == 2) ? ( strtoul(argv[1]+1, NULL, 10)) : 64);

    unsigned int l_cnt = CACHE_SIZE / l_size; /**< The amount of lines in the cache. */
    unsigned int assoc = l_cnt / SET_COUNT; /**< The cache set associativity, i.e. lines per set*/

    struct c_meta cache[SET_COUNT][l_cnt]; /**< matrix of cache metadata */
    for (unsigned int i = 0; i < SET_COUNT; ++i) {
        for (unsigned int j = 0; j < l_cnt; ++j) {
            cache[i][j].valid=cache[i][j].set_id=cache[i][j].tag=0;
        }
    }

    unsigned int addr, /**< current 32-bit address */
            addr_tag, /**< tag for address (if it were to be placed in cache) */
            addr_id, /**< set id for address */
            axs_cntr, /**< access counter for LRU removal policy */
            hits, /**< amount of cache line hits */
            misses; /**< amount of cache line misses */
    axs_cntr = hits = misses = 0;

    //this is the main action loop
    while(fscanf(infile, "%x", &addr) != EOF){
        axs_cntr++;

        addr_tag = (addr >> fast_log2(assoc)) & (fast_log2(assoc)/2 + 4);
        addr_id = addr >> 2 * fast_log2(assoc);

        bool banked;
        for (int i = 0; i < SET_COUNT; ++i) {
            banked = (cache[i][addr_id].valid && cache[i][addr_id].tag == addr_tag);
        }

        if(banked){
            hits++;
        }

    }
    return 0;
}