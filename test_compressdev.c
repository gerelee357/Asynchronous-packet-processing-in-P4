 1 #include <rte_malloc.h>
  2 #include <rte_eal.h>
  3 #include <rte_log.h>
  4 #include <rte_comp.h>
  5 #include <rte_compressdev.h>
  6
  7
  8 #include <time.h>
  9 #include <stdio.h>
 10 #include <stdlib.h>
 11 #include <string.h>
 12 #include <stdint.h>
 13 #include <inttypes.h>
 14 #include <sys/types.h>
 15 #include <sys/queue.h>
 16 #include <netinet/in.h>
 17 #include <setjmp.h>
 18 #include <stdarg.h>
 19 #include <ctype.h>
 20 #include <errno.h>
 21 #include <getopt.h>
 22 #include <fcntl.h>
 23 #include <unistd.h>
 24 #include <rte_atomic.h>
 25 #include <rte_branch_prediction.h>
 26 #include <rte_bus_vdev.h>
 27 #include <rte_common.h>
 28 #include <rte_cycles.h>
 29 #include <rte_dev.h>
 30 #include <rte_debug.h>
 31 #include <rte_eal.h>
 32 #include <rte_ether.h>
 33 #include <rte_ethdev.h>
 34 #include <rte_interrupts.h>
 35 #include <rte_ip.h>
 36 #include <rte_launch.h>
 37 #include <rte_lcore.h>
 38 #include <rte_log.h>
 39 #include <rte_mbuf.h>
 40 #include <rte_memcpy.h>
 41 #include <rte_memory.h>
 42 #include <rte_mempool.h>
43 #include <rte_per_lcore.h>
 44 #include <rte_prefetch.h>
 45 #include <rte_random.h>
 46 #include <rte_hexdump.h>
 47 #define DEFAULT_WINDOW_SIZE 15
 48 #define DEFAULT_MEM_LEVEL 8
 49 #define MAX_DEQD_RETRIES 10
 50 #define DEQUEUE_WAIT_TIME 10000
 51 #define NUM_MAX_XFORMS 16
 52 #define CACHE_SIZE 0
 53 #define NUM_MAX_INFLIGHT_OPS 512
 54 #define NUM_MBUFS            8192
 55 #define POOL_CACHE_SIZE      256
 56 #define NUM_OPS              1
 57 #define BUFFER_SIZE          32
 58 #define PRIV_SIZE            16
 59
 60 void print_bytes(unsigned char *, size_t);
 61 void print_bytes(unsigned char *c, size_t n)
 62 {
 63     size_t i;
 64     for(i = 0; i < n; i++)
 65         printf("%02X ", c[i]);
 66     printf("\n");
 67 }
 68
 69
 70 int main(int argc, char **argv)
 71 {
 72
 73 srand ((unsigned int) time (NULL));
 74
 75 //struct rte_mempool *mbuf_pool, *op_pool;
 76
 77 int ret;
 78 /* Initialize EAL. */
 79 ret = rte_eal_init(argc, argv);
 80 if (ret < 0)
 81     rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");
 82
 83 uint8_t socket_id = rte_socket_id();
 84 struct rte_mempool *mbuf_pool, *op_pool;
 85 /* Create the mbuf pool. */
 86 mbuf_pool = rte_pktmbuf_pool_create("mbuf_pool",
 87                                 NUM_MBUFS,
 88                                 POOL_CACHE_SIZE,
 89                                 0,
 90                                 RTE_MBUF_DEFAULT_BUF_SIZE,
 91                                 socket_id);
 92 if (mbuf_pool == NULL)
 93     rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");
 94
 95
 96 unsigned int comp_op_private_data = PRIV_SIZE;
 97
 98 /* Create compression operation pool. */
 99 op_pool = rte_comp_op_pool_create(      "op_pool",
100                                         NUM_MBUFS,
101                                         POOL_CACHE_SIZE,
102                                         comp_op_private_data,
103                                         socket_id);
104 if (op_pool == NULL)
105     rte_exit(EXIT_FAILURE, "Cannot create crypto op pool\n");
106
107
108 /** COMPRESS DEVICE ************************************************************/
109 char args[128];
110 const char *compress_name = "compress_zlib";
111 snprintf(args, sizeof(args), "socket_id=%d", socket_id);
112 ret = rte_vdev_init(compress_name, args);
113 if (ret != 0)
114     rte_exit(EXIT_FAILURE, "Cannot create virtual device");
115 uint8_t cdev_id_compress = rte_compressdev_get_dev_id(compress_name);
116
117 /** DECOMPRESS DEVICE ************************************************************/
118
119 const char *compress_name2 = "compress_zlib1";
120 snprintf(args, sizeof(args), "socket_id=%d", socket_id);
121 ret = rte_vdev_init(compress_name2, args);
122 if (ret != 0)
123     rte_exit(EXIT_FAILURE, "Cannot create virtual device");
124 uint8_t cdev_id_decompress = rte_compressdev_get_dev_id(compress_name2);
125
126 /* configure the compress device. Allocation of resourses */
127
128 struct rte_compressdev_config conf = { //structure is used to pass configuration parameters.
129                 .socket_id = rte_socket_id(),
130                 .nb_queue_pairs = 1,
131                 .max_nb_priv_xforms = NUM_MAX_XFORMS,
132                 .max_nb_streams = 1
133         };
134
135 if (rte_compressdev_configure(cdev_id_compress, &conf) < 0) // rte_compressdev_configure API is used to configure a compression device
136     rte_exit(EXIT_FAILURE, "Failed to configure compressdev %u", cdev_id_compress);
137
138 if (rte_compressdev_configure(cdev_id_compress, &conf) == 0) // rte_compressdev_configure API is used to configure a compression device
139     printf(  "Compress device is succesfully configured. \n");
140
141 if (rte_compressdev_queue_pair_setup(cdev_id_compress, 0, NUM_MAX_INFLIGHT_OPS,
142                         rte_socket_id()) < 0)
143     rte_exit(EXIT_FAILURE, "Failed to setup queue pair\n");
144
145 if (rte_compressdev_queue_pair_setup(cdev_id_compress, 0, NUM_MAX_INFLIGHT_OPS,
146                         rte_socket_id()) == 0)
147      printf("The queue pair is successfully configured in the compress device. \n");
148
149
150
151 if (rte_compressdev_start(cdev_id_compress) < 0)
152     rte_exit(EXIT_FAILURE, "Failed to start device\n");
153 else
154     printf("Compress device successfully created \n");
155
156
157 /* configure the decompress device. Allocation of resources */
158
159 if (rte_compressdev_configure(cdev_id_decompress, &conf) < 0)
160     rte_exit(EXIT_FAILURE, "Failed to configure compressdev %u", cdev_id_decompress);
161
162 if (rte_compressdev_configure(cdev_id_decompress, &conf) == 0) // rte_compressdev_configure API is used to configure a compression device
163     printf(  "Decompress device is succesfully configured. \n");
164
165 if (rte_compressdev_queue_pair_setup(cdev_id_decompress, 0, NUM_MAX_INFLIGHT_OPS,
166                         rte_socket_id()) < 0)
167     rte_exit(EXIT_FAILURE, "Failed to setup queue pair\n");
168
169 if (rte_compressdev_queue_pair_setup(cdev_id_decompress, 0, NUM_MAX_INFLIGHT_OPS,
170                         rte_socket_id()) == 0)
171     printf("The queue pair is successfully configured in the decompress device. \n");
172
173 if (rte_compressdev_start(cdev_id_decompress) < 0)
174     rte_exit(EXIT_FAILURE, "Failed to start device\n");
175 else
176     printf("Decompress device successfully created \n");
177
178
179 /* setup compress transform */
180
181 struct rte_comp_xform compress_xform = {
182     //.type=RTE_COMP_OP_STATELESS,
183     .type = RTE_COMP_COMPRESS,
184     .compress = {
185         .algo = RTE_COMP_ALGO_DEFLATE,
186         .deflate = {
187             .huffman = RTE_COMP_HUFFMAN_DEFAULT
188         },
189         .level = RTE_COMP_LEVEL_PMD_DEFAULT,
190         .chksum = RTE_COMP_CHECKSUM_NONE,
191         .window_size = DEFAULT_WINDOW_SIZE,
192         .hash_algo = RTE_COMP_HASH_ALGO_NONE //test deer bhgui uchir tur comment
193     }
194 };
195
196
197
198 /* setup decompress transform */
199
200 struct rte_comp_xform decompress_xform = {
201     .type = RTE_COMP_DECOMPRESS,
202     .decompress = {
203         .algo = RTE_COMP_ALGO_DEFLATE,
204         .chksum = RTE_COMP_CHECKSUM_NONE,
205         .window_size = DEFAULT_WINDOW_SIZE,
206     }
207 };
208
209 struct rte_compressdev_info dev_info;
210 void *priv_xform = NULL;
211 int shareable;
212
213 rte_compressdev_info_get(cdev_id_compress, &dev_info);
214 if(dev_info.capabilities->comp_feature_flags & RTE_COMP_FF_SHAREABLE_PRIV_XFORM) {
215    rte_compressdev_private_xform_create(cdev_id_compress, &compress_xform, &priv_xform);
216     printf("success");
217 } else {
218     shareable = 0;
219     printf("Non-shareable is configured in the shareable device. \n");
220 }
221
222
223 struct rte_compressdev_info dev_info2;
224 void *priv_xform2 = NULL;
225
226 rte_compressdev_info_get(cdev_id_decompress, &dev_info2);
227
228 if(dev_info.capabilities->comp_feature_flags & RTE_COMP_FF_SHAREABLE_PRIV_XFORM) {
229     rte_compressdev_private_xform_create(cdev_id_decompress, &decompress_xform, &priv_xform2);
230     printf("success");
231 } else {
232     shareable = 0;
233     printf("Non-shareable is configured in the decompress device.  \n");
234 }
235
236
237 /* create an op pool and allocate ops */
238
239 struct rte_comp_op *comp_ops[NUM_OPS], *processed_ops[NUM_OPS] ;
240
241 if (rte_comp_op_bulk_alloc(op_pool, comp_ops, NUM_OPS) == 0)
242     rte_exit(EXIT_FAILURE, "Not enough crypto operations available\n");
243
244  if (rte_comp_op_bulk_alloc(op_pool, processed_ops, NUM_OPS) == 0)
245     rte_exit(EXIT_FAILURE, "Not enough crypto operations available\n");
246
247
248
249 /* Prepare source and destination mbufs for compression operations*/
250 struct rte_mbuf *comp_mbufs[BUFFER_SIZE], *uncomp_mbufs[BUFFER_SIZE];
251
252 if (rte_pktmbuf_alloc_bulk(mbuf_pool, comp_mbufs, NUM_OPS) < 0)
253     rte_exit(EXIT_FAILURE, "Not enough room in the source mbuf");
254
255 if (rte_pktmbuf_alloc_bulk(mbuf_pool, comp_mbufs, NUM_OPS) == 0)
256     printf("Allocate a source bulk of comp_mbufs, initialize refcnt and reset the fields to default values\n");
257
258
259 if (rte_pktmbuf_alloc_bulk(mbuf_pool, uncomp_mbufs, NUM_OPS) < 0)
260     rte_exit(EXIT_FAILURE, "Not enough room in the dst mbuf");
261
262 if (rte_pktmbuf_alloc_bulk(mbuf_pool, uncomp_mbufs, NUM_OPS) == 0)
263             printf("Allocate a destination bulk of mbufs, initialize refcnt and reset the fields to default values\n");
264
265
266 /* Prepare source and destination mbufs for compression operations*/
267 unsigned int i;
268
269 for (i = 0; i < NUM_OPS; i++) {
270     if (rte_pktmbuf_append(uncomp_mbufs[i], BUFFER_SIZE) == NULL)
271         rte_exit(EXIT_FAILURE, "Not enough room in the mbuf\n");
272
273            comp_ops[i]->m_src=uncomp_mbufs[i];
274 }
275
276 for (i = 0; i < NUM_OPS; i++) {
277     if (rte_pktmbuf_append(comp_mbufs[i], BUFFER_SIZE) == NULL)
278         rte_exit(EXIT_FAILURE, "Not enough room in the mbuf\n");
279
280            comp_ops[i]->m_dst=comp_mbufs[i];
281 }
282
283
284 uint16_t num_enqd, num_deqd;
285
286 // Set up the compress operations.
287 for (i = 0; i < NUM_OPS; i++) {
288
289 //rte_comp_op structure contains data relating to performing a compression operation on the referenced mbuf data buffers.
290
291
292 if (!shareable)
293         { //rte_compressdev_private_xform_create should alloc a private_xform from the device's mempool and initialise it.
294         if(rte_compressdev_private_xform_create(cdev_id_compress, &compress_xform, &comp_ops[i]->private_xform)==0)
295         {printf("compresss xform created \n");}
296         }
297 else
298        {comp_ops[i]->private_xform = priv_xform; }
299
300  comp_ops[i]->op_type = RTE_COMP_OP_STATELESS;
301  comp_ops[i]->flush_flag = RTE_COMP_FLUSH_FINAL;
302  comp_ops[i]->src.offset = 0;
303  comp_ops[i]->dst.offset = 0;
304  comp_ops[i]->src.length = BUFFER_SIZE;
305
306
307 printf("Original data:  ");
308 int j;
309 struct rte_mbuf *m= comp_ops[i]->m_src;
310 unsigned char *data = rte_pktmbuf_mtod(m, unsigned char*);  //A macro that points to the start of the data in the mbuf.
311 for (j = 1; j < 16; j++)
312 data[j] = 9;
313
314 comp_ops[i]->input_chksum = 0;
315
316 print_bytes(data, 16);
317
318
319
320
321     /*rte_compressdev_enqueue_burst---Enqueue a burst of operations for processing on a compression device.
322      *The rte_comp_op contains both input and output parameters and is the
323      * vehicle for the application to pass data into and out of the PMD. While an
324      * op is inflight, i.e. once it has been enqueued, the private_xform
325      * attached to it and any mbufs or memory referenced by it should not be altered
326      * or freed by the application. The PMD may use or change some of this data at
327      * any time until it has been returned in a dequeue operation.
328      *
329      */
330
331 num_enqd = rte_compressdev_enqueue_burst(cdev_id_compress, 0, comp_ops , NUM_OPS);
332
333 if(comp_ops[i]->status == RTE_COMP_OP_STATUS_SUCCESS)
334     printf("Enqueue operation completed succesfully  \n");
335
336
337 printf("comp_ops[i]->consumed \n");
338 printf("%d\n", comp_ops[i]->consumed);
339
340 printf("comp_ops[i]->produced \n");
341 printf("%d\n", comp_ops[i]->produced);
342
343
344 printf("processed_ops[i]->consumed \n");
345 printf("%d\n", processed_ops[i]->consumed);
346
347 printf(" processed_ops[i]->produced \n");
348 printf("%d\n", processed_ops[i]->produced);
349 }
350
351
352 uint16_t total_processed_ops=0;
353
354 usleep(DEQUEUE_WAIT_TIME);
355
356     do {
357
358 //Dequeue a burst of processed compression operations from a queue on the compress device.
359 num_deqd = rte_compressdev_dequeue_burst(cdev_id_compress, 0 , processed_ops, NUM_OPS);
360 total_processed_ops += num_deqd;
361
362
363         /* Check if operation was processed successfully */
364 for (i = 0; i < num_deqd; i++)
365         {
366         if (processed_ops[i]->status != RTE_COMP_OP_STATUS_SUCCESS)
367             rte_exit(EXIT_FAILURE,
368                     "Some operations were not processed correctly");
369         else
370            {printf("Dequeu operation completed successfully \n");
371
372             printf("comp_ops[i]->consumed \n");
373             printf("%d\n", comp_ops[i]->consumed);
374
375             printf("comp_ops[i]->produced \n");
376             printf("%d\n", comp_ops[i]->produced);
377
378
379             printf("processed_ops[i]->consumed \n");
380             printf("%d\n", processed_ops[i]->consumed);
381
382             printf(" processed_ops[i]->produced \n");
383             printf("%d\n", processed_ops[i]->produced);
384
385
386             struct rte_mbuf *m = processed_ops[i]->m_dst;
387             unsigned char *data = rte_pktmbuf_mtod(m, unsigned char*);
388             printf("Compressed data: ");
389             print_bytes(data, 16);
390
391
392            }
393         }
394
395 rte_mempool_put_bulk(op_pool, (void **)processed_ops,num_deqd);
396       }
397
398 while (total_processed_ops < num_enqd);
399      //push next op
400
401 //--------------------------------------------------------------------
402
403
404 for (i = 0; i < NUM_OPS; i++) {
405
406 if (!shareable)
407     {   //rte_compressdev_private_xform_create should alloc a private_xform from the device's mempool and initialise it.
408         rte_compressdev_private_xform_create(cdev_id_decompress, &decompress_xform, &comp_ops[i]->private_xform);
409                         printf("decompress xform created \n");
410     }
411 else
412
413     {comp_ops[i]->private_xform = priv_xform2; }
414
415     printf("Compressed data in decompress device:  ");
416
417
418     comp_ops[i]->m_src=processed_ops[i]->m_dst;
419     comp_ops[i]->m_dst=uncomp_mbufs[i];
420
421
422     struct rte_mbuf *m= comp_ops[i]->m_src;
423     unsigned char *data = rte_pktmbuf_mtod(m, unsigned char*);
424     print_bytes(data, 16);
425
426    // rte_comp_op_bulk_free(processed_ops[i],NUM_OPS);
427
428 num_enqd = rte_compressdev_enqueue_burst(cdev_id_decompress, 0, comp_ops , NUM_OPS);
429 if(comp_ops[i]->status == RTE_COMP_OP_STATUS_SUCCESS)
430 printf("Enqueue a burst of operations for processing on a decompression device \n");
431
432 //rte_comp_op_bulk_free(comp_ops[i],NUM_OPS);
433
434
435 printf("comp_ops[i]->consumed \n");
436 printf("%d\n", comp_ops[i]->consumed);
437
438 printf("comp_ops[i]->produced \n");
439 printf("%d\n", comp_ops[i]->produced);
440
441
442 printf("processed_ops[i]->consumed \n");
443 printf("%d\n", processed_ops[i]->consumed);
444
445 printf(" processed_ops[i]->produced \n");
446 printf("%d\n", processed_ops[i]->produced);
447
448
449                           }
450
451 total_processed_ops=0;
452 usleep(DEQUEUE_WAIT_TIME);
453
454  do {
455 //dequeue a burst of processed compression operations from a queue on the compress device
456 num_deqd = rte_compressdev_dequeue_burst(cdev_id_decompress, 0 , processed_ops, NUM_OPS);
457 total_processed_ops += num_deqd;
458 /* Check if operation was processed successfully */
459 for (i = 0; i < num_deqd; i++) {
460
461 if (processed_ops[i]->status != RTE_COMP_OP_STATUS_SUCCESS)
462             rte_exit(EXIT_FAILURE,
463                     "Some operations were not processed correctly");
464 else
465         {   printf("dequeu burst of operation from deque on the comp device \n");
466
467             struct rte_mbuf *m = processed_ops[i]->m_dst;
468             unsigned char *data = rte_pktmbuf_mtod(m, unsigned char*);
469             printf("Original data in decompress device: ");
470             print_bytes(data, 16);
471
472         }
473     }
474
475     //rte_mempool_put_bulk(op_pool, (void **)processed_ops, num_deqd);
476
477  }
478 while (total_processed_ops < num_enqd);
479
480
481 }
