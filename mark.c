//
// Created by dwrodri on 4/24/18.
//

/* simple cache simulation
 *
 * CPSC 3300 / Clemson University / Fall 2014
 *
 * 32 KB eight-way set-associative cache
 *
 *   512 total lines, 64 lines/bank, 64 bytes/line
 *
 * true LRU replacement using access count as a timestamp
 *
 * input: 32-bit addresses (read as hex values)
 * output: cache stats
 *
 */

#include<stdio.h>

#define LINES 64
#define BANKS 8

int lastused[BANKS][LINES], /* access count for a given line */
        valid[BANKS][LINES],    /* valid bit for a given line    */
/*   0 == invalid, 1 == valid    */
        tag[BANKS][LINES];      /* tag bits for a given line     */

int access,                 /* access counter == timestamp        */
        address,                /* 32-bit address value read as input */
        addr_tag,               /* tag bits from address              */
        addr_index;             /* index bits from address == set id  */

void init( void ){
    int i, j;
    for( i = 0; i < BANKS; i++ ){
        for( j = 0; j < LINES; j++ ){
            lastused[i][j] = valid[i][j] = tag[i][j] = 0;
        }
    }
}

int main( void ){

    FILE* infile = fopen("testfiles/gcc10k.txt", "r");
    int hits, misses;
    int i, bank;

    init();
    access = hits = misses = 0;

    while( fscanf(infile,  "%x", &address ) != EOF ){

        /* this version ignores the read/write indicator rw */

        access++;

        addr_index = (address >> 6) & 0x3f;
        addr_tag = address >> 12;

        /* search for hit */

        bank = -1;
        for( i = 0; i < BANKS; i++ ){
            if( valid[i][addr_index] && (addr_tag == tag[i][addr_index]) ){
                bank = i;
                break;
            }
        }

        if( bank != -1 ){

            hits++;
            lastused[bank][addr_index] = access;

        }else{

            misses++;

            /* search for replacement */

            bank = -1;
            for( i = 0; i < BANKS; i++ ){
                if( !valid[i][addr_index] ){
                    bank = i;
                    break;
                }
            }

            if( bank != -1 ){

                /* use previously invalid line */

                valid[bank][addr_index] = 1;
                tag[bank][addr_index] = addr_tag;
                lastused[bank][addr_index] = access;

            }else{

                /* search for oldest line in set */

                bank = 0;
                for( i = 1; i < BANKS; i++ ){
                    if( lastused[i][addr_index] < lastused[bank][addr_index] ){
                        bank = i;
                    }
                }

                /* replace oldest line in set */

                valid[bank][addr_index] = 1;
                tag[bank][addr_index] = addr_tag;
                lastused[bank][addr_index] = access;

            }
        }
    }

    printf( "%d misses of %d references (%5.2f%% hit rate)\n",
            misses, hits + misses, 100.0*(float)hits/(float)(hits+misses));

    return 0;
}

