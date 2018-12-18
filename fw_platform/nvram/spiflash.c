#include "spiflash.h"
#include "spiflashdev.h"

#define READ_ARRAY_OPCODE   	0x03
#define DEVICE_ID_READ      	0x9F


#define WRITE_ENABLE_CMD    	0x06
#define WRITE_DISABLE_CMD   	0x4
#define PROGRAM_PAGE_CMD    	0x02
#define WRITE_STATUS1_OPCODE    0x01
#define CHIP_ERASE_OPCODE   	0xC7
#define ERASE_SECTOR_OPCODE  	0xD8
#define READ_STATUS         	0x05


#define READY_BIT_MASK      		0x01

#define UNPROTECT_SECTOR_OPCODE     	0x39

#define DONT_CARE       		0x00u

#define NB_BYTES_PER_PAGE   		256

//--------------------------------------------------------------
spiflash_id_t spiflash_id_tmp={
	.device_id_num = 2, 
	.extern_id_num = 0
};
   
/* 
 * Macro To Enable SPI-FLASH with PDMA 
*/
#define SPI_FLASH_WITH_PDMA   

/*******************************************************************************
 * Local functions
 */
#define WAITE_TIMES	0x00ffffff
static int wait_ready( mss_spi_instance_t * mss_spi )
{
    	uint8_t ready_bit;
    	uint8_t command = READ_STATUS;
	//uint32_t wait_times=0;
    
    	do {
		MSS_SPI_transfer_block(mss_spi, &command, sizeof(command), &ready_bit, sizeof(ready_bit) );
		ready_bit = ready_bit & READY_BIT_MASK;
		//wait_times ++;
    	} 
	//while( (ready_bit != READY_BIT_MASK) && (wait_times <WAITE_TIMES) );
	while(ready_bit==READY_BIT_MASK);
	  
#if 0
	if(ready_bit)
		return 0;

	return -1;
#else
	return 0;
#endif
}


static int transfer_block(mss_spi_instance_t * mss_spi, const uint8_t * cmd_buffer, uint16_t cmd_byte_size, uint8_t * rd_buffer, uint16_t rd_byte_size)
{
  	if(wait_ready(mss_spi))
		return -1;
    	MSS_SPI_transfer_block(mss_spi, cmd_buffer, cmd_byte_size, rd_buffer, rd_byte_size);
	if(wait_ready(mss_spi))
		return -1;
	return 0;
}

/* Send Write Enable command */
static int write_enable(mss_spi_instance_t * mss_spi)
{
	uint8_t cmd_buffer;
	
    	cmd_buffer = WRITE_ENABLE_CMD;
	return transfer_block(mss_spi, &cmd_buffer, 1, 0, 0);
}

/* Send Write Disable command. */
static int write_disable(mss_spi_instance_t * mss_spi)
{
  	uint8_t cmd_buffer;
  	
    	cmd_buffer = WRITE_DISABLE_CMD;	
	return transfer_block(mss_spi, &cmd_buffer, 1, 0, 0);
}

static int write_cmd_data(    mss_spi_instance_t * mss_spi, const uint8_t * cmd_buffer, uint16_t cmd_byte_size, uint8_t * data_buffer, uint16_t data_byte_size)
{
	uint32_t wait_times=0;
#ifdef SPI_FLASH_WITH_PDMA   
    uint32_t transfer_size;
    
    transfer_size = cmd_byte_size + data_byte_size;
    
    MSS_SPI_disable( mss_spi );
    MSS_SPI_set_transfer_byte_count( mss_spi, transfer_size );

    PDMA_start(PDMA_CHANNEL_0,         /* channel_id */
            (uint32_t)cmd_buffer,   /* src_addr */
            PDMA_SPI0_TX_REGISTER,  /* dest_addr */
            cmd_byte_size           /* transfer_count */ );
    
    PDMA_load_next_buffer(PDMA_CHANNEL_0,         /* channel_id */
            (uint32_t)data_buffer,  /* src_addr */
            PDMA_SPI0_TX_REGISTER,  /* dest_addr */
            data_byte_size          /* transfer_count */);

    MSS_SPI_enable( mss_spi );
#else

    uint8_t tx_buffer[516];
    uint16_t transfer_size;
    uint16_t idx = 0;
    
    transfer_size = cmd_byte_size + data_byte_size;

    #if 0
    for(idx = 0; idx < cmd_byte_size; ++idx)
    {
        tx_buffer[idx] = cmd_buffer[idx];
    }

    for(idx = 0; idx < data_byte_size; ++idx)
    {
        tx_buffer[cmd_byte_size + idx] = data_buffer[idx];
    }
    #else
	memcpy(&tx_buffer[0], cmd_buffer, cmd_byte_size);
    	memcpy(&tx_buffer[cmd_byte_size], data_buffer, data_byte_size);
    #endif
    
    MSS_SPI_transfer_block( mss_spi, tx_buffer, transfer_size, 0, 0 );
    
#endif    
    while ( (!MSS_SPI_tx_done(mss_spi)) && (wait_times <WAITE_TIMES) )
    {
    	wait_times++;
    }

    if(wait_times <WAITE_TIMES)
    	return 0;
    return -1;
}

/*******************************************************************************
 *
 */
static void spiflash_hw_init( struct nvram *nvm )
{
	struct spiflash_resource *spiflash_rc = (struct spiflash_resource *)nvm->resource;
	
#ifdef SPI_FLASH_WITH_PDMA
    /*--------------------------------------------------------------------------
     * Configure DMA channel used as part of this SPI Flash driver.
     */
    PDMA_init();
    
    /*
     * Configure PDMA channel 0 to perform a memory to SPI0 byte transfer
     * incrementing source address after each transfer and no increment for
     * destination address.
     */
    PDMA_configure(PDMA_CHANNEL_0, 
                   PDMA_TO_SPI_0,
                   PDMA_BYTE_TRANSFER | PDMA_LOW_PRIORITY
                                      | PDMA_INC_SRC_ONE_BYTE 
                                      | PDMA_NO_INC,
                   PDMA_DEFAULT_WRITE_ADJ);
    
    /*
     * Configure PDMA channel 1 to perform a SPI0 to memory byte transfer
     * incrementing the destination address after each transfer 
     * and No increment of source address. 
     */
    PDMA_configure(PDMA_CHANNEL_1, 
                   PDMA_FROM_SPI_0,
                   PDMA_BYTE_TRANSFER | PDMA_LOW_PRIORITY 
                                      | PDMA_NO_INC 
                                      | PDMA_INC_DEST_ONE_BYTE,
                   PDMA_DEFAULT_WRITE_ADJ);

#endif
    
    /*--------------------------------------------------------------------------
     * Configure SPI.
     */
    MSS_SPI_init( spiflash_rc->spi_instance );
    
    MSS_SPI_configure_master_mode(spiflash_rc->spi_instance,
            spiflash_rc->spi_slave_id,
            MSS_SPI_MODE3,
            8,
            MSS_SPI_BLOCK_TRANSFER_FRAME_SIZE);
}

/*******************************************************************************
 *
 */
static int spiflash_read_device_id(    struct nvram *nvm, spiflash_id_t *spiflash_id)
{
	uint8_t cmd_buffer[6], read_buffer[6];
    	volatile uint32_t status;
    	struct spiflash_resource *spiflash_rc = (struct spiflash_resource *)nvm->resource;
	uint8_t i, j, k;
    	uint32_t wait_times=0;
	
    	cmd_buffer[0] = DEVICE_ID_READ;
    	cmd_buffer[1] = 0x0;
    	cmd_buffer[2] = 0x0;
    	cmd_buffer[3] = 0x0;
    	cmd_buffer[4] = 0x0;
    	cmd_buffer[5] = 0x0;
    
    	MSS_SPI_set_slave_select(spiflash_rc->spi_instance, spiflash_rc->spi_slave_id);
#ifdef SPI_FLASH_WITH_PDMA

    	MSS_SPI_disable(spiflash_rc->spi_instance);
    	MSS_SPI_set_transfer_byte_count(spiflash_rc->spi_instance, 6);
             
    	PDMA_start(PDMA_CHANNEL_0,
           (uint32_t)cmd_buffer, 
           PDMA_SPI0_TX_REGISTER, 
           (sizeof(cmd_buffer)));
    
    	PDMA_start(PDMA_CHANNEL_1, 
               PDMA_SPI0_RX_REGISTER, 
               (uint32_t)read_buffer, 
               (sizeof(read_buffer)));  
    
    	MSS_SPI_enable(spiflash_rc->spi_instance); 
    
    
    	while ( (!MSS_SPI_tx_done(spiflash_rc->spi_instance)) && (wait_times <WAITE_TIMES) )
    	{
    		wait_times++;
    	}

    	if((wait_times ==WAITE_TIMES))
					return -1;

    	MSS_SPI_clear_slave_select(spiflash_rc->spi_instance, spiflash_rc->spi_slave_id);

    	spiflash_id->manufacturer_id = read_buffer[1];

	if(spiflash_id->device_id_num<=2)
		j = spiflash_id->device_id_num;
	else
		j = 2;
	for(i=0; i<j; i++)
		spiflash_id->device_id[i] = read_buffer[2+i];

	if(spiflash_id->extern_id_num<=2)
		k = spiflash_id->extern_id_num;
	else
		k = 2;
	for(i=0; i<k; i++)
		spiflash_id->extern_id[i] = read_buffer[2+j+i];
        
#else
    	MSS_SPI_set_slave_select( spiflash_rc->spi_instance, spiflash_rc->spi_slave_id);
    	MSS_SPI_transfer_block( spiflash_rc->spi_instance, cmd_buffer, 4, read_buffer, sizeof(read_buffer) );
    	MSS_SPI_clear_slave_select( spiflash_rc->spi_instance, spiflash_rc->spi_slave_id );

	spiflash_id->manufacturer_id = read_buffer[0];

	if(spiflash_id->device_id_num<=2)
		j = spiflash_id->device_id_num;
	else
		j = 2;
	for(i=0; i<j; i++)
		spiflash_id->device_id[i] = read_buffer[1+i];

	if(spiflash_id->extern_id_num<=2)
		k = spiflash_id->extern_id_num;
	else
		k = 2;
	for(i=0; i<k; i++)
		spiflash_id->extern_id[i] = read_buffer[1+j+i];
	
#endif
	return 0;
}

/*******************************************************************************
 *
 */
int spiflash_global_unprotect( struct nvram *nvm )
{
    	uint8_t cmd_buffer[2];
	struct spiflash_resource *spiflash_rc = (struct spiflash_resource *)nvm->resource;
	int ret;
	
    	MSS_SPI_set_slave_select( spiflash_rc->spi_instance, spiflash_rc->spi_slave_id );
    
    	/* Send Write Enable command */
    	ret = write_enable(spiflash_rc->spi_instance);
	if(ret)
		return ret;
    
    	/* Send Chip Erase command */
    	cmd_buffer[0] = WRITE_STATUS1_OPCODE;
    	cmd_buffer[1] = 0;

    	ret = transfer_block(spiflash_rc->spi_instance, cmd_buffer, 2, 0, 0);
    
    	MSS_SPI_clear_slave_select( spiflash_rc->spi_instance, spiflash_rc->spi_slave_id );
    	return ret;
}

/*******************************************************************************
 *
 */
static int spiflash_get_status( struct nvram *nvm, uint8_t *pstatus )
{
    	uint8_t status;
    	uint8_t command = READ_STATUS;
    	struct spiflash_resource *spiflash_rc = (struct spiflash_resource *)nvm->resource;
	int ret;
    
    	ret = transfer_block(spiflash_rc->spi_instance, &command, sizeof(uint8_t), &status, sizeof(status) );
	*pstatus = status;
	
    	return ret;
}

/*******************************************************************************
 *
 */
static int spiflash_chip_erase( struct nvram *nvm )
{
    	uint8_t cmd_buffer;    
	struct spiflash_resource *spiflash_rc = (struct spiflash_resource *)nvm->resource;
	int ret;
	
	//ret = spiflash_global_unprotect(nvm);
	//if(ret)
	//	return ret;
	
    	MSS_SPI_set_slave_select( spiflash_rc->spi_instance, spiflash_rc->spi_slave_id );

    	/* Send Write Enable command */
    	ret = write_enable(spiflash_rc->spi_instance);
    	if(!ret)
    	{
    		/* Send Chip Erase command */
    		cmd_buffer = CHIP_ERASE_OPCODE;
    		ret = transfer_block(spiflash_rc->spi_instance, &cmd_buffer, 1, 0, 0);
    	}
    	MSS_SPI_clear_slave_select( spiflash_rc->spi_instance, spiflash_rc->spi_slave_id );

	return ret;
}

/*******************************************************************************
 *
 */
int spiflash_erase_sector(    struct nvram *nvm, uint32_t offset)
{
    	uint8_t cmd_buffer[4];
    	struct spiflash_resource *spiflash_rc = (struct spiflash_resource *)nvm->resource;
	int ret;
	
	//ret = spiflash_global_unprotect(nvm);
	//if(ret)
	//	return ret;
	
    	MSS_SPI_set_slave_select( spiflash_rc->spi_instance, spiflash_rc->spi_slave_id );
    
    	/* Send Write Enable command */
    	ret = write_enable(spiflash_rc->spi_instance);
    	if(!ret)
    	{
	    	/* Send Chip Erase command */
	    	cmd_buffer[0] = ERASE_SECTOR_OPCODE;
	    	cmd_buffer[1] = (offset >> 16) & 0xFF;
	    	cmd_buffer[2] = (offset >> 8 ) & 0xFF;
	    	cmd_buffer[3] = offset & 0xFF;
    	
    		ret = transfer_block(spiflash_rc->spi_instance, cmd_buffer, sizeof(cmd_buffer), 0, 0);
    	}
    	MSS_SPI_clear_slave_select( spiflash_rc->spi_instance, spiflash_rc->spi_slave_id );
	return ret;
}

uint8_t read_buf[32];
int spiflash_erase_block(    struct nvram *nvm, uint32_t offset, uint32_t block_size)
{
	uint32_t erase_count = 0, i, sectors=0;	
	
    	if(spiflash_global_unprotect(nvm))
	{ 
		printf((const char * )"\n\rspiflash_global_unprotect error.\n\r");
		return -1;
	}			

	sectors = (block_size/nvm->feature.pagesize) + ((block_size%nvm->feature.pagesize)?1:0);
	for(erase_count = 0;erase_count<sectors;erase_count++)
	{
		spiflash_erase_sector(nvm, offset);
		i = 10000;
		while(i!=0)
		{
		        i--;
		}

		spiflash_read(nvm, read_buf, offset, sizeof(read_buf));  
		offset+= nvm->feature.pagesize;

		for(i=0; i<sizeof(read_buf); i++)
		{
			if(read_buf[i] != 0xFF)
			{
				printf((const char * )"F\t");
				printf((const char * )"\n\rspiflash erase error.\n\r");
				return -1;
			}
			else
				printf((const char * )".");
		}
	 }
	return 0;
}

/*******************************************************************************
 *
 */
int spiflash_read(    struct nvram *nvm,  void *buffer, int offset, int size_in_bytes)
{
	uint8_t * rx_buffer = (uint8_t *)buffer;
    	uint8_t cmd_buffer[6];
    	struct spiflash_resource *spiflash_rc = (struct spiflash_resource *)nvm->resource;
    	int ret=0;
	uint32_t wait_times=0;
	
#ifdef SPI_FLASH_WITH_PDMA

    	uint32_t transfer_size;
    	uint32_t count;
    	uint8_t *ptr; 
    	uint8_t *dst_ptr;

    	cmd_buffer[0] = READ_ARRAY_OPCODE;
    	cmd_buffer[1] = (uint8_t)((offset >> 16) & 0xFF);
    	cmd_buffer[2] = (uint8_t)((offset >> 8) & 0xFF);
   	cmd_buffer[3] = (uint8_t)(offset & 0xFF);
    	cmd_buffer[4] = DONT_CARE;
    	cmd_buffer[5] = DONT_CARE;
    	ptr = &cmd_buffer[0];

	dst_ptr = rx_buffer;

	if(dst_ptr != NULL)
    	{
        	transfer_size = (size_in_bytes + sizeof(cmd_buffer));
        	MSS_SPI_set_slave_select(spiflash_rc->spi_instance, spiflash_rc->spi_slave_id);
        	ret = wait_ready(spiflash_rc->spi_instance);
    		if(!ret)
    		{
        		MSS_SPI_disable(spiflash_rc->spi_instance);
        		MSS_SPI_set_transfer_byte_count(spiflash_rc->spi_instance, transfer_size);

        		PDMA_start(PDMA_CHANNEL_0,          /* channel_id */
	                   (uint32_t)ptr,           /* src_addr */
	                   PDMA_SPI0_TX_REGISTER,   /* dest_addr */
	                   transfer_size);
    
	        	PDMA_start(PDMA_CHANNEL_1,          /* channel_id */
	                   PDMA_SPI0_RX_REGISTER,   /* src_addr */
	                   (uint32_t)dst_ptr,       /* dest_addr */
	                   transfer_size);
    
        		MSS_SPI_enable(spiflash_rc->spi_instance);
    
        		while ( (!MSS_SPI_tx_done(spiflash_rc->spi_instance)) && (wait_times <WAITE_TIMES) )
    			{
    				wait_times++;
    			}

    			if(wait_times ==WAITE_TIMES)
				return -1;
    		}
       		MSS_SPI_clear_slave_select(spiflash_rc->spi_instance, spiflash_rc->spi_slave_id);
                
        	for(count = 0; count < size_in_bytes; count++)
        	{
            		rx_buffer[count] = dst_ptr[4+count];
        	}    
		
    	}
    	else
		return -1;        
#else    
	cmd_buffer[0] = READ_ARRAY_OPCODE;
	cmd_buffer[1] = (uint8_t)((offset >> 16) & 0xFF);
	cmd_buffer[2] = (uint8_t)((offset >> 8) & 0xFF);;
	cmd_buffer[3] = (uint8_t)(offset & 0xFF);
	cmd_buffer[4] = DONT_CARE;
	cmd_buffer[5] = DONT_CARE;

	MSS_SPI_set_slave_select( spiflash_rc->spi_instance, spiflash_rc->spi_slave_id );
	ret = transfer_block(spiflash_rc->spi_instance, cmd_buffer, 4, rx_buffer, size_in_bytes);
	MSS_SPI_clear_slave_select( spiflash_rc->spi_instance, spiflash_rc->spi_slave_id );
#endif
	return ret;
}



        
/*******************************************************************************
 *
 */
int spiflash_write(struct nvram *nvm, const void *buffer, int offset, int size_in_bytes)
{
	uint8_t cmd_buffer[4];
	uint8_t * write_buffer = (uint8_t *)buffer;
	uint32_t in_buffer_idx;
	uint32_t nb_bytes_to_write;
	uint32_t target_addr;
	int ret=0;
	struct spiflash_resource *spiflash_rc = (struct spiflash_resource *)nvm->resource;

	MSS_SPI_set_slave_select( spiflash_rc->spi_instance, spiflash_rc->spi_slave_id );

	/* Send Write Enable command */
	ret = write_enable(spiflash_rc->spi_instance);
	if(!ret)
	{
		/* Unprotect sector */
		cmd_buffer[0] = UNPROTECT_SECTOR_OPCODE;
		cmd_buffer[1] = (offset >> 16) & 0xFF;
		cmd_buffer[2] = (offset >> 8 ) & 0xFF;
		cmd_buffer[3] = offset & 0xFF;

		ret = transfer_block(spiflash_rc->spi_instance, cmd_buffer, sizeof(cmd_buffer), 0, 0 );
		if(!ret)
		{
			/* Send Write Enable command */
			ret = write_enable(spiflash_rc->spi_instance);

			/**/
			in_buffer_idx = 0;
			nb_bytes_to_write = size_in_bytes;
			target_addr = offset;

			while ( in_buffer_idx < size_in_bytes )
			{
				uint32_t size_left;
				nb_bytes_to_write = 0x100 - (target_addr & 0xFF);   /* adjust max possible size to page boundary. */
				size_left = size_in_bytes - in_buffer_idx;
				if ( size_left < nb_bytes_to_write )
				{
			    		nb_bytes_to_write = size_left;
				}

			/* Send Write Enable command */
				write_enable(spiflash_rc->spi_instance);
			    
				/* Program page */
				cmd_buffer[0] = PROGRAM_PAGE_CMD;
				cmd_buffer[1] = (target_addr >> 16) & 0xFF;
				cmd_buffer[2] = (target_addr >> 8 ) & 0xFF;
				cmd_buffer[3] = target_addr & 0xFF;

				ret = write_cmd_data(spiflash_rc->spi_instance, cmd_buffer, sizeof(cmd_buffer), &write_buffer[in_buffer_idx], nb_bytes_to_write);
				if(ret)
					break;
				target_addr += nb_bytes_to_write;
				in_buffer_idx += nb_bytes_to_write;
			}
		}
		/* Send Write Disable command. */
		ret = write_disable(spiflash_rc->spi_instance);
	}
	MSS_SPI_clear_slave_select( spiflash_rc->spi_instance, spiflash_rc->spi_slave_id );

	return ret;
}




int spiflash_install(struct nvram *nvm)
{
	struct spiflash_resource *spiflash_rc;
	spiflash_info_t *pspiflash_info;
	int i;
		
	if (!nvm)
		return -1;
	if (!nvm->resource)
		return -1;

	spiflash_hw_init(nvm);
	spiflash_read_device_id(nvm, &spiflash_id_tmp);

	spiflash_rc = (struct spiflash_resource *)nvm->resource;
	pspiflash_info = spiflash_rc->pspiflash_info;
	
	 if ((pspiflash_info->spiflash_id.manufacturer_id != spiflash_id_tmp.manufacturer_id) || 
	 	(pspiflash_info->spiflash_id.device_id[0] != spiflash_id_tmp.device_id[0]) ||
	 	(pspiflash_info->spiflash_id.device_id[1] != spiflash_id_tmp.device_id[1]))
	 	return -1;

	if(pspiflash_info->spiflash_id.extern_id_num)
	{
		for(i=0; i<pspiflash_info->spiflash_id.extern_id_num; i++)
			if(pspiflash_info->spiflash_id.extern_id[i] != spiflash_id_tmp.extern_id[i])
				return -1;
	}

	// set nvram features
	nvm->feature = pspiflash_info->nvram_feature_struct;
	
	// set nvram operation functions
	nvm->global_unprotect = spiflash_global_unprotect;
	nvm->erase = spiflash_chip_erase;
	nvm->erase_block = spiflash_erase_block;
	nvm->read = spiflash_read;
	nvm->write = spiflash_write;

	return 0;
}


int spiflash_drvinit(void)
{
	return 0;
}


