#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "my_postgres.h"

#define PAGE_HEADER_SIZE (sizeof(PageHeaderData) - sizeof(ItemIdData))
#define ITEM_ID_SIZE (sizeof(ItemIdData))
#define HEAP_TUPLE_HEADER_SIZE (sizeof(HeapTupleHeaderData))
#define ATT_DATA_SIZE (sizeof(int))


int
main(int argc, char *argv[])
{
	int row, col;
	int nb_rows, nb_clms;
	int nsm_tuple_size, pax_tuple_size;
	int fd_r, fd_w;
	char pax_filename[100];
	
	char nsm_page[BLCKSZ];
	char pax_page[BLCKSZ];
	
	int page_no;

	PageHeader nsm_ph; 
	PageHeaderData pax_phd;
	ItemIdData pax_iid;
	HeapTupleHeaderData thd;
	HeapTupleHeader th = &(thd);
	
	int pax_data_start_off;
	int nsm_data_start_off;

	int freespace_size;

	if (argc != 2) {
		printf("Usage: %s XXXXXX\n", argv[0]);
		exit(1);
	}
    
	fd_r = open(argv[1], O_RDONLY);
	if (fd_r == -1) {
		perror("open");
		exit(1);
	}

	snprintf(pax_filename, 100, "pax_%s", argv[1]);
	fd_w = open(pax_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd_w == -1) {
		perror("open");
		exit(1);
	}
	
	int ns;
	while (1)
	{
		ns = read(fd_r, nsm_page, BLCKSZ);
		if (ns == -1) 
		{
			perror("read");
			exit(1);
		}
		
		if (ns == 0)
		{
			break;
		}
		

		nsm_ph = (PageHeader) nsm_page;
		pax_phd = *nsm_ph;

		pax_phd.pd_lower = PAGE_HEADER_SIZE;
		pax_phd.pd_upper = BLCKSZ;
		
		nb_rows = (nsm_ph->pd_lower - PAGE_HEADER_SIZE) / ITEM_ID_SIZE;
		nb_clms = (nsm_ph->pd_linp[0].lp_len - HEAP_TUPLE_HEADER_SIZE) / ATT_DATA_SIZE;

		nsm_tuple_size = (HEAP_TUPLE_HEADER_SIZE + nb_clms * ATT_DATA_SIZE);
		pax_tuple_size = (HEAP_TUPLE_HEADER_SIZE + nb_rows * ATT_DATA_SIZE);

		for (col = 0; col < nb_clms; col++) {
			
			pax_iid.lp_off = pax_phd.pd_upper - pax_tuple_size;
			pax_iid.lp_flags = 1;
			pax_iid.lp_len = pax_tuple_size;

			// write ItemIdData
			memcpy(pax_page + pax_phd.pd_lower, &pax_iid, ITEM_ID_SIZE);
			
			// adjust pd_lower
			pax_phd.pd_lower += ITEM_ID_SIZE;
			
			thd.t_hoff = sizeof(HeapTupleHeaderData);
			HeapTupleHeaderSetNatts(th, nb_rows);
			
			// write HeapTupleHeaderData  
			memcpy(pax_page + pax_phd.pd_upper - pax_tuple_size, th, HEAP_TUPLE_HEADER_SIZE);
			
			for (row = 0; row < nb_rows; row++) {
								
				nsm_data_start_off = BLCKSZ - (row + 1) * nsm_tuple_size + HEAP_TUPLE_HEADER_SIZE;
				pax_data_start_off = pax_iid.lp_off + HEAP_TUPLE_HEADER_SIZE;
				
				// write data
				memcpy(pax_page + pax_data_start_off + row * ATT_DATA_SIZE,
				       nsm_page + nsm_data_start_off + col * ATT_DATA_SIZE,
				       ATT_DATA_SIZE);
			
			}

			// adjust pd_upper
			pax_phd.pd_upper -= pax_tuple_size;

			freespace_size = pax_phd.pd_upper - pax_phd.pd_lower;

			if (freespace_size < pax_tuple_size)
				break;			
		}
		
		// write PageHeaderData
		memcpy(pax_page, &(pax_phd), PAGE_HEADER_SIZE);

		if (write(fd_w, pax_page, BLCKSZ) == -1) {
			close(fd_w);
			perror("write");
			exit(1);
		}
		
   	}
	
	close(fd_w);
	close(fd_r);

	return 0;
}
