 #include <rte_malloc.h>
 #include <rte_eal.h>
 #include <rte_log.h>
 #include <rte_comp.h>
 #include <rte_compressdev.h>


 #include <time.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdint.h>
 #include <inttypes.h>
 #include <sys/types.h>
 #include <sys/queue.h>
 #include <netinet/in.h>
 #include <setjmp.h>
 #include <stdarg.h>
 #include <ctype.h>
 #include <errno.h>
 #include <getopt.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <rte_atomic.h>
 #include <rte_branch_prediction.h>
 #include <rte_bus_vdev.h>
 #include <rte_common.h>
 #include <rte_cycles.h>
 #include <rte_dev.h>
 #include <rte_debug.h>
 #include <rte_eal.h>
 #include <rte_ether.h>
 #include <rte_ethdev.h>
 #include <rte_interrupts.h>
 #include <rte_ip.h>
 #include <rte_launch.h>
 #include <rte_lcore.h>
 #include <rte_log.h>
 #include <rte_mbuf.h>
 #include <rte_memcpy.h>
 #include <rte_memory.h>
 #include <rte_mempool.h>
 #include <rte_per_lcore.h>
 #include <rte_prefetch.h>
 #include <rte_random.h>
 #include <rte_hexdump.h>
 #define DEFAULT_WINDOW_SIZE 15
 #define DEFAULT_MEM_LEVEL 8
 #define MAX_DEQD_RETRIES 10
 #define DEQUEUE_WAIT_TIME 10000
 #define NUM_MAX_XFORMS 16
 #define CACHE_SIZE 0
 #define NUM_MAX_INFLIGHT_OPS 512
 #define NUM_MBUFS            8192
 #define POOL_CACHE_SIZE      256
 #define NUM_OPS              1
 #define BUFFER_SIZE          32
 #define PRIV_SIZE            16

 void print_bytes(unsigned char *, size_t);
 void print_bytes(unsigned char *c, size_t n)
 {
     size_t i;
     for(i = 0; i < n; i++)
         printf("%02X ", c[i]);
     printf("\n");
 }


 int main(int argc, char **argv)
 {

 srand ((unsigned int) time (NULL));

 //struct rte_mempool *mbuf_pool, *op_pool;

 int ret;
 /* Initialize EAL. */
 ret = rte_eal_init(argc, argv);
 if (ret < 0)
     rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");

 uint8_t socket_id = rte_socket_id();
 struct rte_mempool *mbuf_pool, *op_pool;
 /* Create the mbuf pool. */
 mbuf_pool = rte_pktmbuf_pool_create("mbuf_pool",
                                 NUM_MBUFS,
                                 POOL_CACHE_SIZE,
,
                                 RTE_MBUF_DEFAULT_BUF_SIZE,
                                 socket_id);
 if (mbuf_pool == NULL)
     rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");


 unsigned int comp_op_private_data = PRIV_SIZE;

 /* Create compression operation pool. */
 op_pool = rte_comp_op_pool_create(      "op_pool",
                                         NUM_MBUFS,
                                         POOL_CACHE_SIZE,
                                         comp_op_private_data,
                                         socket_id);
 if (op_pool == NULL)
     rte_exit(EXIT_FAILURE, "Cannot create crypto op pool\n");


 /** COMPRESS DEVICE ************************************************************/
 char args[128];
 const char *compress_name = "compress_zlib";
 snprintf(args, sizeof(args), "socket_id=%d", socket_id);
 ret = rte_vdev_init(compress_name, args);
 if (ret != 0)
     rte_exit(EXIT_FAILURE, "Cannot create virtual device");
 uint8_t cdev_id_compress = rte_compressdev_get_dev_id(compress_name);

 /** DECOMPRESS DEVICE ************************************************************/

 const char *compress_name2 = "compress_zlib1";
 snprintf(args, sizeof(args), "socket_id=%d", socket_id);
 ret = rte_vdev_init(compress_name2, args);
 if (ret != 0)
     rte_exit(EXIT_FAILURE, "Cannot create virtual device");
 uint8_t cdev_id_decompress = rte_compressdev_get_dev_id(compress_name2);

 /* configure the compress device. Allocation of resourses */

 struct rte_compressdev_config conf = { //structure is used to pass configuration parameters.
                 .socket_id = rte_socket_id(),
                 .nb_queue_pairs = 1,
                 .max_nb_priv_xforms = NUM_MAX_XFORMS,
                 .max_nb_streams = 1
         };

 if (rte_compressdev_configure(cdev_id_compress, &conf) < 0) // rte_compressdev_configure API is used to configure a compression device
     rte_exit(EXIT_FAILURE, "Failed to configure compressdev %u", cdev_id_compress);

 if (rte_compressdev_configure(cdev_id_compress, &conf) == 0) // rte_compressdev_configure API is used to configure a compression device
     printf(  "Compress device is succesfully configured. \n");

 if (rte_compressdev_queue_pair_setup(cdev_id_compress, 0, NUM_MAX_INFLIGHT_OPS,
                         rte_socket_id()) < 0)
     rte_exit(EXIT_FAILURE, "Failed to setup queue pair\n");

 if (rte_compressdev_queue_pair_setup(cdev_id_compress, 0, NUM_MAX_INFLIGHT_OPS,
                         rte_socket_id()) == 0)
      printf("The queue pair is successfully configured in the compress device. \n");



 if (rte_compressdev_start(cdev_id_compress) < 0)
     rte_exit(EXIT_FAILURE, "Failed to start device\n");
 else
     printf("Compress device successfully created \n");


 /* configure the decompress device. Allocation of resources */

 if (rte_compressdev_configure(cdev_id_decompress, &conf) < 0)
     rte_exit(EXIT_FAILURE, "Failed to configure compressdev %u", cdev_id_decompress);

 if (rte_compressdev_configure(cdev_id_decompress, &conf) == 0) // rte_compressdev_configure API is used to configure a compression device
     printf(  "Decompress device is succesfully configured. \n");

 if (rte_compressdev_queue_pair_setup(cdev_id_decompress, 0, NUM_MAX_INFLIGHT_OPS,
                         rte_socket_id()) < 0)
     rte_exit(EXIT_FAILURE, "Failed to setup queue pair\n");

 if (rte_compressdev_queue_pair_setup(cdev_id_decompress, 0, NUM_MAX_INFLIGHT_OPS,
                         rte_socket_id()) == 0)
     printf("The queue pair is successfully configured in the decompress device. \n");

 if (rte_compressdev_start(cdev_id_decompress) < 0)
     rte_exit(EXIT_FAILURE, "Failed to start device\n");
 else
     printf("Decompress device successfully created \n");


 /* setup compress transform */

 struct rte_comp_xform compress_xform = {
     //.type=RTE_COMP_OP_STATELESS,
     .type = RTE_COMP_COMPRESS,
     .compress = {
         .algo = RTE_COMP_ALGO_DEFLATE,
         .deflate = {
             .huffman = RTE_COMP_HUFFMAN_DEFAULT
         },
         .level = RTE_COMP_LEVEL_PMD_DEFAULT,
         .chksum = RTE_COMP_CHECKSUM_NONE,
         .window_size = DEFAULT_WINDOW_SIZE,
         .hash_algo = RTE_COMP_HASH_ALGO_NONE //test deer bhgui uchir tur comment
     }
 };



 /* setup decompress transform */

 struct rte_comp_xform decompress_xform = {
     .type = RTE_COMP_DECOMPRESS,
     .decompress = {
         .algo = RTE_COMP_ALGO_DEFLATE,
         .chksum = RTE_COMP_CHECKSUM_NONE,
         .window_size = DEFAULT_WINDOW_SIZE,
     }
 };

 struct rte_compressdev_info dev_info;
 void *priv_xform = NULL;
 int shareable;

 rte_compressdev_info_get(cdev_id_compress, &dev_info);
 if(dev_info.capabilities->comp_feature_flags & RTE_COMP_FF_SHAREABLE_PRIV_XFORM) {
    rte_compressdev_private_xform_create(cdev_id_compress, &compress_xform, &priv_xform);
     printf("success");
 } else {
     shareable = 0;
     printf("Non-shareable is configured in the shareable device. \n");
 }


 struct rte_compressdev_info dev_info2;
 void *priv_xform2 = NULL;

 rte_compressdev_info_get(cdev_id_decompress, &dev_info2);

 if(dev_info.capabilities->comp_feature_flags & RTE_COMP_FF_SHAREABLE_PRIV_XFORM) {
     rte_compressdev_private_xform_create(cdev_id_decompress, &decompress_xform, &priv_xform2);
     printf("success");
 } else {
     shareable = 0;
     printf("Non-shareable is configured in the decompress device.  \n");
 }


 /* create an op pool and allocate ops */

 struct rte_comp_op *comp_ops[NUM_OPS], *processed_ops[NUM_OPS] ;

 if (rte_comp_op_bulk_alloc(op_pool, comp_ops, NUM_OPS) == 0)
     rte_exit(EXIT_FAILURE, "Not enough crypto operations available\n");

  if (rte_comp_op_bulk_alloc(op_pool, processed_ops, NUM_OPS) == 0)
     rte_exit(EXIT_FAILURE, "Not enough crypto operations available\n");



 /* Prepare source and destination mbufs for compression operations*/
 struct rte_mbuf *comp_mbufs[BUFFER_SIZE], *uncomp_mbufs[BUFFER_SIZE];

 if (rte_pktmbuf_alloc_bulk(mbuf_pool, comp_mbufs, NUM_OPS) < 0)
     rte_exit(EXIT_FAILURE, "Not enough room in the source mbuf");

 if (rte_pktmbuf_alloc_bulk(mbuf_pool, comp_mbufs, NUM_OPS) == 0)
     printf("Allocate a source bulk of comp_mbufs, initialize refcnt and reset the fields to default values\n");


 if (rte_pktmbuf_alloc_bulk(mbuf_pool, uncomp_mbufs, NUM_OPS) < 0)
     rte_exit(EXIT_FAILURE, "Not enough room in the dst mbuf");

 if (rte_pktmbuf_alloc_bulk(mbuf_pool, uncomp_mbufs, NUM_OPS) == 0)
             printf("Allocate a destination bulk of mbufs, initialize refcnt and reset the fields to default values\n");


 /* Prepare source and destination mbufs for compression operations*/
 unsigned int i;

 for (i = 0; i < NUM_OPS; i++) {
     if (rte_pktmbuf_append(uncomp_mbufs[i], BUFFER_SIZE) == NULL)
         rte_exit(EXIT_FAILURE, "Not enough room in the mbuf\n");

            comp_ops[i]->m_src=uncomp_mbufs[i];
 }

 for (i = 0; i < NUM_OPS; i++) {
     if (rte_pktmbuf_append(comp_mbufs[i], BUFFER_SIZE) == NULL)
         rte_exit(EXIT_FAILURE, "Not enough room in the mbuf\n");

            comp_ops[i]->m_dst=comp_mbufs[i];
 }


 uint16_t num_enqd, num_deqd;

 // Set up the compress operations.
 for (i = 0; i < NUM_OPS; i++) {

 //rte_comp_op structure contains data relating to performing a compression operation on the referenced mbuf data buffers.


 if (!shareable)
         { //rte_compressdev_private_xform_create should alloc a private_xform from the device's mempool and initialise it.
         if(rte_compressdev_private_xform_create(cdev_id_compress, &compress_xform, &comp_ops[i]->private_xform)==0)
         {printf("compresss xform created \n");}
         }
 else
        {comp_ops[i]->private_xform = priv_xform; }

  comp_ops[i]->op_type = RTE_COMP_OP_STATELESS;
  comp_ops[i]->flush_flag = RTE_COMP_FLUSH_FINAL;
  comp_ops[i]->src.offset = 0;
  comp_ops[i]->dst.offset = 0;
  comp_ops[i]->src.length = BUFFER_SIZE;


 printf("Original data:  ");
 int j;
 struct rte_mbuf *m= comp_ops[i]->m_src;
 unsigned char *data = rte_pktmbuf_mtod(m, unsigned char*);  //A macro that points to the start of the data in the mbuf.
 for (j = 1; j < 16; j++)
 data[j] = 9;

 comp_ops[i]->input_chksum = 0;

 print_bytes(data, 16);




     /*rte_compressdev_enqueue_burst---Enqueue a burst of operations for processing on a compression device.
      *The rte_comp_op contains both input and output parameters and is the
      * vehicle for the application to pass data into and out of the PMD. While an
      * op is inflight, i.e. once it has been enqueued, the private_xform
      * attached to it and any mbufs or memory referenced by it should not be altered
      * or freed by the application. The PMD may use or change some of this data at
      * any time until it has been returned in a dequeue operation.
      *
      */

 num_enqd = rte_compressdev_enqueue_burst(cdev_id_compress, 0, comp_ops , NUM_OPS);

 if(comp_ops[i]->status == RTE_COMP_OP_STATUS_SUCCESS)
     printf("Enqueue operation completed succesfully  \n");


 printf("comp_ops[i]->consumed \n");
 printf("%d\n", comp_ops[i]->consumed);

 printf("comp_ops[i]->produced \n");
 printf("%d\n", comp_ops[i]->produced);


 printf("processed_ops[i]->consumed \n");
 printf("%d\n", processed_ops[i]->consumed);

 printf(" processed_ops[i]->produced \n");
 printf("%d\n", processed_ops[i]->produced);
 }


 uint16_t total_processed_ops=0;

 usleep(DEQUEUE_WAIT_TIME);

     do {

 //Dequeue a burst of processed compression operations from a queue on the compress device.
 num_deqd = rte_compressdev_dequeue_burst(cdev_id_compress, 0 , processed_ops, NUM_OPS);
 total_processed_ops += num_deqd;


         /* Check if operation was processed successfully */
 for (i = 0; i < num_deqd; i++)
         {
         if (processed_ops[i]->status != RTE_COMP_OP_STATUS_SUCCESS)
             rte_exit(EXIT_FAILURE,
                     "Some operations were not processed correctly");
         else
            {printf("Dequeu operation completed successfully \n");

             printf("comp_ops[i]->consumed \n");
             printf("%d\n", comp_ops[i]->consumed);

             printf("comp_ops[i]->produced \n");
             printf("%d\n", comp_ops[i]->produced);


             printf("processed_ops[i]->consumed \n");
             printf("%d\n", processed_ops[i]->consumed);

             printf(" processed_ops[i]->produced \n");
             printf("%d\n", processed_ops[i]->produced);


             struct rte_mbuf *m = processed_ops[i]->m_dst;
             unsigned char *data = rte_pktmbuf_mtod(m, unsigned char*);
             printf("Compressed data: ");
             print_bytes(data, 16);


            }
         }

 rte_mempool_put_bulk(op_pool, (void **)processed_ops,num_deqd);
       }

 while (total_processed_ops < num_enqd);
      //push next op

 //--------------------------------------------------------------------


 for (i = 0; i < NUM_OPS; i++) {

 if (!shareable)
     {   //rte_compressdev_private_xform_create should alloc a private_xform from the device's mempool and initialise it.
         rte_compressdev_private_xform_create(cdev_id_decompress, &decompress_xform, &comp_ops[i]->private_xform);
                         printf("decompress xform created \n");
     }
 else

     {comp_ops[i]->private_xform = priv_xform2; }

     printf("Compressed data in decompress device:  ");


     comp_ops[i]->m_src=processed_ops[i]->m_dst;
     comp_ops[i]->m_dst=uncomp_mbufs[i];


     struct rte_mbuf *m= comp_ops[i]->m_src;
     unsigned char *data = rte_pktmbuf_mtod(m, unsigned char*);
     print_bytes(data, 16);

    // rte_comp_op_bulk_free(processed_ops[i],NUM_OPS);

 num_enqd = rte_compressdev_enqueue_burst(cdev_id_decompress, 0, comp_ops , NUM_OPS);
 if(comp_ops[i]->status == RTE_COMP_OP_STATUS_SUCCESS)
 printf("Enqueue a burst of operations for processing on a decompression device \n");

 //rte_comp_op_bulk_free(comp_ops[i],NUM_OPS);


 printf("comp_ops[i]->consumed \n");
 printf("%d\n", comp_ops[i]->consumed);

 printf("comp_ops[i]->produced \n");
 printf("%d\n", comp_ops[i]->produced);


 printf("processed_ops[i]->consumed \n");
 printf("%d\n", processed_ops[i]->consumed);

 printf(" processed_ops[i]->produced \n");
 printf("%d\n", processed_ops[i]->produced);


                           }

 total_processed_ops=0;
 usleep(DEQUEUE_WAIT_TIME);

  do {
 //dequeue a burst of processed compression operations from a queue on the compress device
 num_deqd = rte_compressdev_dequeue_burst(cdev_id_decompress, 0 , processed_ops, NUM_OPS);
 total_processed_ops += num_deqd;
 /* Check if operation was processed successfully */
 for (i = 0; i < num_deqd; i++) {

 if (processed_ops[i]->status != RTE_COMP_OP_STATUS_SUCCESS)
             rte_exit(EXIT_FAILURE,
                     "Some operations were not processed correctly");
 else
         {   printf("dequeu burst of operation from deque on the comp device \n");

             struct rte_mbuf *m = processed_ops[i]->m_dst;
             unsigned char *data = rte_pktmbuf_mtod(m, unsigned char*);
             printf("Original data in decompress device: ");
             print_bytes(data, 16);

         }
     }

     //rte_mempool_put_bulk(op_pool, (void **)processed_ops, num_deqd);

  }
 while (total_processed_ops < num_enqd);


 }
