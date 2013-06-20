/* check_guppi_databuf.c
 *
 * Basic prog to test dstabuf shared mem routines.
 */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "fitshead.h"
#include "guppi_error.h"
#include "hashpipe_status.h"
#include "guppi_databuf.h"
#include "guppi_thread_main.h"

void usage() { 
    fprintf(stderr, 
            "Usage: check_guppi_databuf [options]\n"
            "Options:\n"
            "  -h, --help\n"
            "  -q, --quiet\n"
            "  -I n, --instance=n (0)\n"
            "  -c, --create\n"
            "  -i n, --id=n  (1)\n"
            "  -s n, --size=n (32M)\n"
            "  -n n, --nblock=n (24)\n"
            );
}

int main(int argc, char *argv[]) {

    /* Loop over cmd line to fill in params */
    static struct option long_opts[] = {
        {"help",   0, NULL, 'h'},
        {"quiet",  0, NULL, 'q'},
        {"instance", 1, NULL, 'I'},
        {"create", 0, NULL, 'c'},
        {"id",     1, NULL, 'i'},
        {"size",   1, NULL, 's'},
        {"nblock", 1, NULL, 'n'},
        {0,0,0,0}
    };
    int opt,opti;
    int quiet=0;
    int instance_id=0;
    int create=0;
    int db_id=1;
    int blocksize = 32;
    int nblock = 24;
    while ((opt=getopt_long(argc,argv,"hqI:ci:s:n:t:",long_opts,&opti))!=-1) {
        switch (opt) {
            case 'I':
                instance_id=atoi(optarg);
                break;
            case 'c':
                create=1;
                break;
            case 'q':
                quiet=1;
                break;
            case 'i':
                db_id = atoi(optarg);
                break;
            case 's':
                blocksize = atoi(optarg);
                break;
            case 'n':
                nblock = atoi(optarg);
                break;
            case 'h':
            default:
                usage();
                exit(0);
                break;
        }
    }

    /* Create mem if asked, otherwise attach */
    struct guppi_databuf *db=NULL;
    if (create) { 
        db = guppi_databuf_create(instance_id, nblock, blocksize*1024*1024, db_id);
        if (db==NULL) {
            fprintf(stderr, "Error creating databuf %d (may already exist).\n",
                    db_id);
            exit(1);
        }
    } else {
        db = guppi_databuf_attach(instance_id, db_id);
        if (db==NULL) { 
            fprintf(stderr, 
                    "Error attaching to databuf %d (may not exist).\n",
                    db_id);
            exit(1);
        }
    }

    if(quiet) {
      return 0;
    }

    /* Print basic info */
    printf("databuf %d stats:\n", db_id);
    printf("  shmid=%d\n", db->shmid);
    printf("  semid=%d\n", db->semid);
    printf("  n_block=%d\n", db->n_block);
    printf("  struct_size=%zd\n", db->struct_size);
    printf("  block_size=%zd\n", db->block_size);
    printf("  header_size=%zd\n\n", db->header_size);

    /* loop over blocks */
    int i;
    char buf[81];
    char *hdr, *ptr, *hend;
    for (i=0; i<db->n_block; i++) {
        printf("block %d status=%d\n", i, 
                guppi_databuf_block_status(db, i));
        hdr = guppi_databuf_header(db, i);
        hend = ksearch(hdr, "END");
        if (hend==NULL) {
            printf("header not initialized\n");
        } else {
            hend += 80;
            printf("header:\n");
            for (ptr=hdr; ptr<hend; ptr+=80) {
                strncpy(buf, ptr, 80);
                buf[79]='\0';
                printf("%s\n", buf);
            }
        }

    }

    exit(0);
}
