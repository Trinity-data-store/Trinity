#include <aerospike/aerospike.h>
#include <aerospike/aerospike_batch.h>
#include <aerospike/aerospike_key.h>
#include <aerospike/as_event.h>
#include <aerospike/as_monitor.h>
#include <unistd.h>
#include <sys/time.h>

#if defined(AS_USE_LIBEVENT)
#include <event.h>
#if LIBEVENT_VERSION_NUMBER < 0x02010000
void event_base_add_virtual(struct event_base*);
#endif
#endif

typedef unsigned long long int TimeStamp;

TimeStamp GetTimestamp() {
  struct timeval now;
  gettimeofday(&now, NULL);

  return now.tv_usec + (TimeStamp) now.tv_sec * 1000000;
}


FILE *fp;
size_t len;
const char *name_list[] = {"quantity", "extendedprice", "discount", "tax", "shipdate", "commitdate", "recepitdate", "totalprice", "orderdate"};
const uint32_t max_records = 3000000;
TimeStamp start, end;
/******************************************************************************
 *	Types
 *****************************************************************************/

// External loop definition
typedef struct {
	pthread_t thread;
#if defined(AS_USE_LIBUV)
	uv_loop_t* uv_loop;
#elif defined(AS_USE_LIBEVENT)
	struct event_base* event_loop;
#else
	struct ev_loop* ev_loop;
#endif
	as_event_loop* as_loop;
} loop;

typedef struct {
	uint32_t next_id;     // Key of next record to write.
	uint32_t max;         // Number of records to write.
	uint32_t count;       // Records written.
	uint32_t queue_size;  // Maximum records allowed inflight (in async queue).
	uint32_t pipe_count;  // Records in pipeline. Pipeline mode only.
	as_pipe_listener pipe_listener;  // Pipeline listener callback. Pipeline mode only.
} counter;

/******************************************************************************
 *	Globals
 *****************************************************************************/

static const char* g_host = "10.10.1.5";
static int g_port = 3000;
static const char* g_namespace = "tpch";
static const char* g_set = "tpch_macro";

static aerospike as;
static as_monitor share_loops_monitor;
static as_monitor app_complete_monitor;

/******************************************************************************
 *	Forward Declarations
 *****************************************************************************/

static void print_usage(const char* program);
static bool share_event_loops(loop* loops, uint32_t loop_count);
static void join_event_loops(loop* loops, uint32_t loop_count);
static void* loop_thread(void* udata);
static void write_records_pipeline(counter* counter);
static void write_records_async(counter* counter);
static bool write_record(as_event_loop* event_loop, counter* counter);
static void pipeline_listener(void* udata, as_event_loop* event_loop);
static void write_listener(as_error* err, void* udata, as_event_loop* event_loop);
static void batch_read(as_event_loop* event_loop, uint32_t max_records);
static void batch_listener(as_error* err, as_batch_read_records* records, void* udata, as_event_loop* event_loop);

/******************************************************************************
 *	Functions
 *****************************************************************************/

int
main(int argc, char* argv[])
{
    fp = fopen(argv[1], "r");
	loop external_loop;
	bool share_loop = false;
	bool pipeline = false;
	int c;
	
	while ((c = getopt(argc, argv, "h:p:n:s:el")) != -1) {
		switch (c) {
			case 'h':
				g_host = optarg;
				break;
			case 'p':
				g_port = atoi(optarg);
				break;
			case 'n':
				g_namespace = optarg;
				break;
			case 's':
				g_set = optarg;
				break;
			case 'e':
				share_loop = true;
				break;
			case 'l':
				pipeline = true;
				break;
			default:
				print_usage(argv[0]);
				return 0;
		}
	}
	
	/*
	printf("Host=%s:%d\n", g_host, g_port);
	printf("Namespace=%s\n", g_namespace);
	printf("Set=%s\n", g_set);
	printf("ShareLoop=%s\n", share_loop ? "true" : "false");
	printf("Pipeline=%s\n", pipeline ? "true" : "false");
	*/

	if (share_loop) {
		// Demonstrate how to share an existing event loop.
		if (! share_event_loops(&external_loop, 1)) {
			printf("Failed to share event loop\n");
			return -1;
		}
	}
	else {
		// Have C client create the event loop.
		if (! as_event_create_loops(1)) {
			printf("Failed to create event loop\n");
			return -1;
		}
	}
	
	as_config cfg;
	as_config_init(&cfg);
	as_config_add_host(&cfg, g_host, g_port);
	cfg.async_max_conns_per_node = 200;
	cfg.thread_pool_size = 0;  // disable sync thread pools.
	aerospike_init(&as, &cfg);
	
	// Connect to cluster.
	as_error err;
	if (aerospike_connect(&as, &err) != AEROSPIKE_OK) {
		printf("Failed to connect to cluster\n");
		aerospike_destroy(&as);
		as_event_close_loops();
		return -1;
	}
	
	// Initialize monitor.
	as_monitor_init(&app_complete_monitor);
	start = GetTimestamp();
	if (pipeline) {
		// Demonstrate pipelined writes.
		// Pipeline queue size (1000) is greater because sockets are shared.
		counter counter = {
			.next_id = 0,
			.max = max_records,
			.count = 0,
			.queue_size = 1000,
			.pipe_count = 0,
			.pipe_listener = pipeline_listener
		};
		write_records_pipeline(&counter);
	}
	else {
		// Demonstrate async non-pipelined writes.
		// Async queue size (100) is less because there is one socket per concurrent command.
		counter counter = {
			.next_id = 0,
			.max = max_records,
			.count = 0,
			.queue_size = 100,
			.pipe_count = 0,
			.pipe_listener = NULL
		};
		write_records_async(&counter);
	}

	// Wait till all commands have completed before shutting down.
	as_monitor_wait(&app_complete_monitor);
	as_monitor_destroy(&app_complete_monitor);
	aerospike_close(&as, &err);
	aerospike_destroy(&as);
	as_event_close_loops();
	
	if (share_loop) {
		// Join on external event loop thread.
#if defined(AS_USE_LIBEVENT)
		event_base_loopbreak(external_loop.event_loop);
#endif
		join_event_loops(&external_loop, 1);
	}
	as_event_destroy_loops();
}

static void
print_usage(const char* program)
{
	printf("Usage: %s [-h <host>] [-p <port>] [-n <namespace>] [-s <set>] [-e] [-l]\n", program);
	printf("-e: share event loop\n");
	printf("-l: use pipeline writes\n");
}

static bool
share_event_loops(loop* loops, uint32_t loop_count)
{
	// Tell C client the maximum number of event loops that will be shared.
	if (! as_event_set_external_loop_capacity(loop_count)) {
		return false;
	}
	
	// Initialize monitor.
	as_monitor_init(&share_loops_monitor);
	bool status = true;
	
	for (uint32_t i = 0; i < loop_count; i++) {
		loop* loop = &loops[i];
		
		// Start monitor.
		as_monitor_begin(&share_loops_monitor);
		
		// Create event loop thread that will be shared.
		if (pthread_create(&loop->thread, NULL, loop_thread, loop) != 0) {
			status = false;
			break;
		}
		
		// Wait till event loop has been initialized.
		as_monitor_wait(&share_loops_monitor);
	}
	as_monitor_destroy(&share_loops_monitor);
	return status;
}

static void
join_event_loops(loop* loops, uint32_t loop_count)
{
	for (uint32_t i = 0; i < loop_count; i++) {
		loop* loop = &loops[i];
		pthread_join(loop->thread, NULL);
	}
}

static void*
loop_thread(void* udata)
{
	// Create external loop.
	loop* loop = udata;

#if defined(AS_USE_LIBUV)

	loop->uv_loop = malloc(sizeof(uv_loop_t));
	uv_loop_init(loop->uv_loop);

	// Share event loop with C client.
	// This must be done in event loop thread.
	loop->as_loop = as_event_set_external_loop(loop->uv_loop);

	// Notify parent thread that external loop has been initialized.
	as_monitor_notify(&share_loops_monitor);

	uv_run(loop->uv_loop, UV_RUN_DEFAULT);
	uv_loop_close(loop->uv_loop);
	free(loop->uv_loop);

#elif defined(AS_USE_LIBEVENT)

	loop->event_loop = event_base_new();

	// Add a virtual event to prevent event_base_dispatch() from returning prematurely.
#if LIBEVENT_VERSION_NUMBER < 0x02010000
	event_base_add_virtual(loop->event_loop);
#endif

	// Share event loop with C client.
	// This must be done in event loop thread.
	loop->as_loop = as_event_set_external_loop(loop->event_loop);

	// Notify parent thread that external loop has been initialized.
	as_monitor_notify(&share_loops_monitor);

#if LIBEVENT_VERSION_NUMBER < 0x02010000
	event_base_dispatch(loop->event_loop);
#else
	event_base_loop(loop->event_loop, EVLOOP_NO_EXIT_ON_EMPTY);
#endif

	event_base_free(loop->event_loop);

#else

	loop->ev_loop = ev_loop_new(EVFLAG_AUTO);

	// Share event loop with C client.
	// This must be done in event loop thread.
	loop->as_loop = as_event_set_external_loop(loop->ev_loop);

	// Notify parent thread that external loop has been initialized.
	as_monitor_notify(&share_loops_monitor);

	ev_loop(loop->ev_loop, 0);
	ev_loop_destroy(loop->ev_loop);

#endif
	return NULL;
}

static void
write_records_pipeline(counter* counter)
{
	// Write a single record to start pipeline.
	// More records will be written in pipeline_listener to fill pipeline queue.
	// A NULL event_loop indicates that an event loop will be chosen round-robin.
	counter->pipe_count++;
	write_record(NULL, counter);
}

static void
write_records_async(counter* counter)
{
	// Use same event loop for all records.
	as_event_loop* event_loop = as_event_loop_get();
	
	// Write queue_size commands on the async queue.
	for (uint32_t i = 0; i < counter->queue_size; i++) {
		if (! write_record(event_loop, counter)) {
			break;
		}
	}
}

static bool
write_record(as_event_loop* event_loop, counter* counter)
{
	int64_t id = counter->next_id++;
	// printf("Started id: %ld\n", id);
	// Create an as_record object with one (integer value) bin. By using
	// as_record_inita(), we won't need to destroy the record if we only set
	// bins using as_record_set_int64().
	as_record rec;
	as_record_inita(&rec, 9);
	char *line = NULL;
	{
		ssize_t read;
		// [id, QUANTITY, EXTENDEDPRICE, DISCOUNT, TAX, SHIPDATE, COMMITDATE, RECEIPTDATE, TOTALPRICE, ORDERDATE]
		if ((read = getline(&line, &len, fp)) != -1)
		{
			char *ptr;
			
			char *token = strtok(line, ","); // id
			if (!token) {
				printf("line: %s\n", line);
				return false;
			}
			id = strtoul(token, &ptr, 10);
			
			// printf("id: %ld\n", id);

			for (int i = 0; i < 9; i++){
				token = strtok(NULL, ","); // id
				if (!token) {
					printf("line: %s\n", line);
					return false;
				}
				int64_t num = strtoll(token, &ptr, 10);
				as_record_set_int64(&rec, name_list[i], num);

				// as_record_set_int64(&rec, name_list[i], num);
			}
			
		}
	}
	// No need to destroy a stack as_key object, if we only use as_key_init_int64().
	as_key key;
	as_key_init_int64(&key, g_namespace, g_set, id);

	// Write a record to the database.
	as_error err;
	if (aerospike_key_put_async(&as, &err, NULL, &key, &rec, write_listener, counter, event_loop, counter->pipe_listener) != AEROSPIKE_OK) {
		// printf("error id: %ld\n", id);

		write_listener(&err, counter, event_loop);
		return false;
	}
	// printf("fine id: %ld\n", id);

	return true;
}

static void
pipeline_listener(void* udata, as_event_loop* event_loop)
{
	counter* counter = udata;
	
	// Check if pipeline has space.
	if (counter->pipe_count < counter->queue_size && counter->next_id < counter->max) {
		// Issue another write.
		counter->pipe_count++;
		write_record(event_loop, counter);
	}
}

static void
write_listener(as_error* err, void* udata, as_event_loop* event_loop)
{
	counter* counter = udata;
	
	if (err) {
		printf("aerospike_key_put_async() returned %d - %s\n", err->code, err->message);
		as_monitor_notify(&app_complete_monitor);
		return;
	}

	// Atomic increment is not necessary since only one event loop is used.
	if (++counter->count == counter->max) {
		// We have written all records.
		// printf("\n", );
		end = GetTimestamp();
		printf("Wrote %u records, throughput: %lld\n", counter->count, max_records / ((end - start) / 1000000));
		// Records can now be read in a batch.
		// batch_read(event_loop, counter->max);
		return;
	}
	
	// Check if we need to write another record.
	if (counter->next_id < counter->max) {
		write_record(event_loop, counter);
	}
	else {
		if (counter->pipe_listener) {
			// There's one fewer command in the pipeline.
			counter->pipe_count--;
		}
	}
}

static void
batch_read(as_event_loop* event_loop, uint32_t max_records)
{
	// Make a batch of all the keys we inserted.
	as_batch_read_records* records = as_batch_read_create(max_records);
	
	for (uint32_t i = 0; i < max_records; i++) {
		as_batch_read_record* record = as_batch_read_reserve(records);
		as_key_init_int64(&record->key, g_namespace, g_set, (int64_t)i);
		record->read_all_bins = true;
	}
	
	// Read these keys.
	as_error err;
	if (aerospike_batch_read_async(&as, &err, NULL, records, batch_listener, NULL, event_loop) != AEROSPIKE_OK) {
		batch_listener(&err, records, NULL, event_loop);
	}
}

static void
batch_listener(as_error* err, as_batch_read_records* records, void* udata, as_event_loop* event_loop)
{
	if (err) {
		printf("aerospike_batch_read_async() returned %d - %s\n", err->code, err->message);
		as_batch_read_destroy(records);
		as_monitor_notify(&app_complete_monitor);
		return;
	}

	as_vector* list = &records->list;

	uint32_t n_found = 0;

	for (uint32_t i = 0; i < list->size; i++) {
		as_batch_read_record* record = as_vector_get(list, i);

		if (record->result == AEROSPIKE_OK) {
			n_found++;
		}
		else if (record->result == AEROSPIKE_ERR_RECORD_NOT_FOUND) {
			// The transaction succeeded but the record doesn't exist.
			printf("AEROSPIKE_ERR_RECORD_NOT_FOUND\n");
		}
		else {
			// The transaction failed.
			printf("Error %d\n", record->result);
		}
	}

	printf("Found %u/%u records\n", n_found, list->size);
	as_batch_read_destroy(records);
	as_monitor_notify(&app_complete_monitor);
}
