
#ifndef MICRODEVTOOLS_H
#define MICRODEVTOOLS_H

#include "apr.h"
#include "apr_pools.h"
#include "apr_tables.h"
#include "apr_file_io.h"
#include "apr_file_info.h"
#include "apr_lib.h"
#include "apr_strings.h"
#include "apr_escape.h"
#include "apr_md5.h"
#include "apr_base64.h"
#include "apr_crypto.h"
#include "apr_time.h"
#include "apr_env.h"
#include "apr_time.h"
#include "apr_date.h"
#include "apr_getopt.h"
#include "apr_proc_mutex.h"
#include "apr_file_io.h"
#include "apr_file_info.h"
#include "apr_time.h"
#include "apr_dbd.h"
#include "stdio.h"
#include "errno.h"
#include "time.h"
#include "syscall.h"
#include "unistd.h"
#include "stdlib.h"
#include "stdbool.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/file.h"
#include "string.h"
#include "signal.h"

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/hmac.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * COMMON
 */

#define NS_MAX_READ_BUFFER 16384
#define NS_ERROR_TIMESTAMP (-1)

int ns_rand(int l, int h);
int ns_is_empty(const char *s);
int ns_is_int(const char *s);
int ns_is_double(const char *s);
int ns_in_string(const char *s, const char *sub);
/**
 * @brief 
 * @param p Memory pool
 * @param s The string to allocate
 * @param bf_size The size of the buffer
 * @return The pointer to the buffer
 * @note The returned string always has a NULL terminator
 *       and a size of at most bf_size-1 bytes
 */
char* ns_buffer(apr_pool_t *mp, const char *s, apr_size_t *bf_size);
char* ns_str(apr_pool_t *mp, const char *s, apr_size_t sz);
char* ns_trim(apr_pool_t *pool, const char *str);
const char* ns_strip_char(apr_pool_t *mp, const char *s, char c);
char* ns_slice(apr_pool_t *mp, const char *s, apr_size_t i, apr_size_t l);
const char* ns_str_replace(apr_pool_t *mp, const char *s, const char *f, const char *r);
const char* ns_replace_char(apr_pool_t *mp, const char *s, char f, char r);
char* ns_empty_string_make(apr_pool_t *mp);
apr_array_header_t* ns_split(apr_pool_t *mp, const char *s, const char *sp);
char* ns_join(apr_pool_t *mp, apr_array_header_t *a, const char *sp);
char* ns_md5(apr_pool_t *mp, const char *s);
char* ns_base64_encode(apr_pool_t *mp, const char *s);
char* ns_base64_decode(apr_pool_t* mp, const char* s);
apr_table_t* ns_args_to_table(apr_pool_t *mp, const char *q);
int ns_table_nelts(apr_table_t *t);
apr_table_entry_t* ns_table_elt(apr_table_t *t, int i);
char* ns_datetime(apr_pool_t *mp, apr_time_t t, const char *f);
char* ns_datetime_local(apr_pool_t *mp, apr_time_t t, const char *f);
char* ns_datetime_utc(apr_pool_t *mp, apr_time_t t, const char *f);
int ns_is_dir(const char *d, apr_pool_t *mp);
int ns_is_file(const char *f, apr_pool_t *mp);
apr_status_t ns_file_open(apr_file_t **fd, const char *f, apr_int32_t fl, apr_pool_t *mp);
apr_status_t ns_file_open_read(apr_file_t **fd, const char *f, apr_pool_t *mp);
apr_status_t ns_file_open_append(apr_file_t **fd, const char *f, apr_pool_t *mp);
apr_status_t ns_file_open_truncate(apr_file_t **fd, const char *f, apr_pool_t *mp);
apr_size_t ns_file_write(apr_file_t *fd, const char *buf, apr_size_t l);
apr_size_t ns_file_read(apr_pool_t *mp, apr_file_t *fd, void **buf);
apr_time_t ns_timestamp(int year, int month, int day, int hour, int minute, int second);
apr_time_t ns_now();
apr_table_entry_t* ns_table_entry(apr_table_t *t, int i);
// Legge i dati dallo standard input e li restituisce come una stringa.
// 'm' è il pool di memoria da utilizzare per l'allocazione di eventuali risorse.
char* ns_pipein(apr_pool_t *mp);
char* ns_env(const char *e, apr_pool_t *mp);
void ns_daemonize();

/*
 * JSON
 */

#include "json.h"

#define NS_JSON_TRUE_S  "true"
#define NS_JSON_FALSE_S "false"
#define NS_JSON_NULL_S  "null"

typedef enum ns_json_type_t
{
  NS_JSON_T_ZERO       = 0x00000000,
  NS_JSON_T_NULL       = 0x00000001,
  NS_JSON_T_BOOLEAN    = 0x00000002,
  NS_JSON_T_PAIR       = 0x00000004,
  NS_JSON_T_INT16      = 0x00000008,
  NS_JSON_T_UINT16     = 0x00000010,
  NS_JSON_T_INT32      = 0x00000020,
  NS_JSON_T_UINT32     = 0x00000040,
  NS_JSON_T_INT64      = 0x00000080,
  NS_JSON_T_UINT64     = 0x00000100,
  NS_JSON_T_FLOAT      = 0x00000200,
  NS_JSON_T_DOUBLE     = 0x00000400,
  NS_JSON_T_STRING     = 0x00000800,
  NS_JSON_T_OBJECT     = 0x00001000,
  NS_JSON_T_ARRAY      = 0x00002000,
  NS_JSON_T_DATE       = 0x00004000,
  NS_JSON_T_NUMBER     = 0x00008000,
  NS_JSON_T_TABLE      = 0x00010000,
  NS_JSON_T_TIMESTAMP  = 0x00020000,
  NS_JSON_T_JSON       = 0x00040000,
  NS_JSON_T_DBD_SCHEMA = 0x00080000,
  NS_JSON_T_VECTOR     = 0x00100000
} ns_json_type_t;

typedef struct ns_json_pair_t {
  const char *key;
  void *val;
  ns_json_type_t type;  
} ns_json_pair_t;

typedef apr_array_header_t ns_json_object_t;
ns_json_object_t* ns_json_decode(apr_pool_t *mp, const char *s);
const char* ns_json_encode(apr_pool_t *mp, const void *obj, ns_json_type_t t);

/*
 * LOGGER
 */

#define NS_LOG_MAX_FILE_SIZE 500 * 1024 * 1024 /* (10MB) */
#define NS_LOG_MAX_MSG_SIZE 512
#define NS_LOG_MSG_FMT "[%s] [%s] [%05d] %s\r\n"

/**
 * @brief Struttura del logger
 */
typedef struct ns_logger_t {
  apr_pool_t *pool;
  apr_file_t *fh;
  const char *fname;
  apr_proc_mutex_t *mutex;
  apr_size_t max_size;
} ns_logger_t;

ns_logger_t* ns_log_alloc(apr_pool_t *mp, apr_proc_mutex_t *m, const char *f, apr_size_t sz);
void ns_log_rotate(ns_logger_t *l);
void ns_log_destroy(ns_logger_t *l);

#define ns_log(l, t, m) do {\
  if (l != NULL && t != NULL && m != NULL) {\
    char _log[NS_LOG_MAX_MSG_SIZE], _ts[APR_CTIME_LEN];\
    apr_time_t _now = apr_time_now();\
    apr_ctime(_ts, _now);\
    apr_snprintf(_log, sizeof(_log), NS_LOG_MSG_FMT, _ts, t, __LINE__, m);\
    size_t _len = strlen(_log);\
    if (_len > 0 && _len < (sizeof(_log)-1) && _log[_len-1] == '\n') {\
      apr_proc_mutex_lock(l->mutex);\
      apr_file_printf(l->fh, "%s", _log);\
      apr_file_flush(l->fh);\
      ns_log_rotate(l);\
      apr_proc_mutex_unlock(l->mutex);\
    }\
  }\
} while (0)

/*
 * DBD
 */

typedef struct ns_dbd_t {
  int err;
  const char *er_msg;
  const apr_dbd_driver_t *drv;
  apr_dbd_t *hdl;
  apr_dbd_transaction_t *trx;
} ns_dbd_t;

ns_dbd_t* ns_dbd_alloc(apr_pool_t *mp);
int ns_dbd_open(apr_pool_t *mp, ns_dbd_t *dbd, const char *drv, const char *con);
const char* ns_dbd_escape(apr_pool_t *mp, ns_dbd_t *dbd, const char *s);
int ns_dbd_query(apr_pool_t *mp, ns_dbd_t *dbd, const char *sql);
apr_array_header_t* ns_dbd_select(apr_pool_t *mp, ns_dbd_t *dbd, const char *sql);
int ns_dbd_transaction_start(apr_pool_t *mp, ns_dbd_t *dbd);
int ns_dbd_transaction_end(apr_pool_t *mp, ns_dbd_t *dbd);
//int ns_dbd_prepared_query(apr_pool_t *mp, ns_dbd_t *dbd, const char *sql, const char **args, int sz);
int ns_dbd_prepared_query(apr_pool_t *mp, ns_dbd_t *dbd, const char *sql, apr_table_t *args);
apr_array_header_t* ns_dbd_prepared_select(apr_pool_t *mp, ns_dbd_t *dbd, const char *sql, apr_table_t *args);
int ns_dbd_num_records(apr_array_header_t *rset);
int ns_dbd_num_columns(apr_array_header_t *rset);
apr_array_header_t* ns_dbd_column_names(apr_pool_t *mp, apr_array_header_t *rset);
apr_table_t* ns_dbd_record(apr_array_header_t *rset, int idx);
const char* ns_dbd_field_value(apr_array_header_t *rset, int idx, const char*key);
int ns_dbd_field_set(apr_array_header_t *rset, int idx, const char *key, const char *val);
int ns_dbd_close(ns_dbd_t *dbd);
const char* ns_dbd_driver_name(ns_dbd_t *dbd);
const char* ns_dbd_error(ns_dbd_t *dbd);

/*
 * HTTP REQUEST
 */

typedef enum ns_request_type_t {
  NS_REQUEST_T_NONE,
  NS_REQUEST_T_INT,
  NS_REQUEST_T_DOUBLE,
  NS_REQUEST_T_STRING,
  NS_REQUEST_T_PASSWORD,
  NS_REQUEST_T_DATE,
  NS_REQUEST_T_EMAIL
} ns_request_type_t;

typedef enum ns_request_filter_t {
  NS_REQUEST_F_NONE,
  NS_REQUEST_F_MD5
} ns_request_filter_t;

typedef struct ns_request_validator_t {
  const char *key;
  ns_request_type_t type;
  ns_request_filter_t filter;
} ns_request_validator_t;

typedef struct ns_http_request_t {
  apr_pool_t *pool;
  int client_port;
  const char *method;
  const char *body;
  const char *query;
  const char *uri;
  const char *http_version;
  const char *client_ip;
  const char *prev_method;
  const char *prev_uri;
  const char *session_id;
  const char *message;
  const char *username;
  const char *password;
  apr_table_t *headers;
  apr_table_t *args;
  apr_table_t *parsed_uri;
  apr_table_t *cookies;
  apr_array_header_t *multipart_data;
} ns_http_request_t;

ns_http_request_t* ns_http_request_alloc(apr_pool_t *mp);
apr_table_t *ns_http_request_validate_args(ns_http_request_t *r, ns_request_validator_t *vd, int nargs);
apr_table_t *ns_http_request_validate_multipart_args(ns_http_request_t *r, ns_request_validator_t *vd, int nargs);

/*
 * HTTP RESPONSE
 */

typedef struct ns_http_response_t {
  apr_pool_t *pool;
  int status;
  int is_binary;
  apr_table_t *headers;
  apr_size_t size;
  void *buffer;
} ns_http_response_t;

ns_http_response_t* ns_http_response_alloc(apr_pool_t *mp);
const char* ns_http_response_hd_serialize(ns_http_response_t *r);
void ns_http_response_hd_set(ns_http_response_t *r, const char *k, const char *v);
const char* ns_http_response_hd_get(ns_http_response_t *r, const char *k);
void ns_http_response_buffer_set(ns_http_response_t *r, void *buf, apr_size_t sz);

/*
 * SERVICE
 */

typedef struct ns_service_t {
  apr_pool_t *pool;
  int authorized;
  char *er_msg;
  ns_http_request_t *request;
  ns_http_response_t *response;
  ns_dbd_t *dbd;
  ns_logger_t *logger;
} ns_service_t;

typedef int (*ns_route_t)(ns_service_t *sv);

ns_service_t* ns_alloc(apr_pool_t *mp);
void ns_handler(ns_service_t *sv);
void ns_route(ns_service_t *sv, const char *mth, const char *uri, ns_route_t fn);
void ns_printf(ns_service_t *sv, const char *fmt, ...);

#define _ns_authorized_routes(_s) \
  for (\
    int _auth_flag = (_s->authorized = 1); \
    _auth_flag && !_s->response->status; \
    _auth_flag = (_s->authorized = 0)\
  )

#define ns_authorized_routes(_s,_f) \
  for (int _loop = 1; _loop && !_s->response->status && _f(_s);_loop = 0)

#define ns_download(s, buf, sz, f, enc) \
  do {\
    ns_http_response_hd_set(s->response, "Expires", "Mon, 23 Oct 1972 16:00:00 GMT");\
    ns_http_response_hd_set(s->response, "Pragma", "hack");\
    ns_http_response_hd_set(s->response, "Cache-Control", "must-revalidate,post-check=0,pre-check=0");\
    ns_http_response_hd_set(s->response, "Cache-Control", "private");\
    ns_http_response_hd_set(s->response, "Content-Description", "File Transfer");\
    ns_http_response_hd_set(s->response, "Content-Disposition", apr_psprintf(s->pool, "attachment; filename=%s", f));\
    ns_http_response_hd_set(s->response, "Content-Transfer-Encoding", enc);\
    /*ns_http_response_hd_set(s, "Content-Length", apr_psprintf(s->pool, "%" APR_UINT64_T_FMT, sz));*/\
    ns_http_response_hd_set(s->response, "Content-Type", "application/download");\
    ns_http_response_buffer_set(s->response, (void*)buf, sz);\
  } while (0)


char* ns_jwt_base64_encode(const unsigned char *input, int length);
char* ns_jwt_token_create(apr_pool_t *mp, apr_table_t *claims, const char *key);
char* ns_hmac_encode(const char *key, const char *s, apr_size_t sz);
int ns_jwt_token_validate(apr_pool_t *mp, const char *tok, const char *key);





// -----------------------------------------------------------------------------


#include "apr.h"
#include "apr_pools.h"
#include "apr_tables.h"
#include "apr_strings.h"
#include "apr_proc_mutex.h"
#include "apr_dbd.h"

#include "stdio.h"
#include "errno.h"
#include "time.h"
#include "syscall.h"
#include "unistd.h"
#include "stdlib.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "sys/file.h"
#include "string.h"
#include "signal.h"

#include "mongoose.h"
#include "microdevtools.h"

#ifdef _DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

#ifdef _DAEMON
#define DAEMON 1
#else
#define DAEMON 0
#endif

#ifdef _MONGOOSE
#define MONGOOSE 1
#else
#define MONGOOSE 0
#endif

#ifdef _TLS
#define TLS 1
#else
#define TLS 0
#endif

#define HTTP_OK_FMT "HTTP/1.1 200 OK\r\n%sContent-Length: %d\r\n\r\n"
#define NS_DBD_POOL_INIT_SIZE 20

typedef struct ns_dbd_pool_t {
  apr_array_header_t *connections;
  int counter;
  apr_proc_mutex_t *mutex;
} ns_dbd_pool_t;


typedef struct ns_server_t {
  apr_pool_t *pool;
  const char *host;
  const char *port;
  const char *port_s;
  const char *addr;
  const char *addr_s;
  const char *timeout;
  const char *max_threads;
  const char *log_file;
  const char *dbd_driver;
  const char *dbd_conn_s;
  const char *upload_dir;
  ns_logger_t *logger;
} ns_server_t;



typedef void(*sighd_t)(int s);


ns_server_t* ns_server_alloc(apr_pool_t *mp);
void ns_server_destroy(ns_server_t *s);
int ns_dbd_pool_alloc(apr_pool_t *mp);
void ns_dbd_pool_add(apr_pool_t *mp, const char *drv, const char *conn_s);
int ns_dbd_pool_init(apr_pool_t *mp, const char *drv, const char *conn_s);
ns_dbd_t* ns_dbd_pool_get();
void ns_dbd_pool_release();
void ns_dbd_pool_destroy();
void ns_http_request_headers_set(apr_pool_t *mp, ns_http_request_t *rq, struct mg_http_message *hm);
apr_table_t* ns_http_request_args_parse(apr_pool_t *mp, const char *s, const char *sp1, const char *sp2);
apr_table_t* ns_http_request_cookies_parse(apr_pool_t *mp, struct mg_http_message *hm);
apr_table_t* ns_http_query_string_parse(apr_pool_t*mp, struct mg_http_message *hm);
apr_table_t* ns_http_request_body_parse(apr_pool_t*mp, struct mg_http_message *hm);
void ns_signal_exit(int signum);
void ns_signal_handler(struct sigaction *sig_action, sighd_t cb);
int ns_http_request_multipart_parse(apr_pool_t *mp, ns_http_request_t *rq, struct mg_connection *c, struct mg_http_message *hm);
void ns_http_request_handler(struct mg_connection *c, int ev, void *ev_data);
int ns_cmd_args_parse(ns_server_t *s, int argc, char *argv[], char **er_msg);
int ns_server_init(apr_pool_t *mp, ns_server_t **s, int argc, char *argv[], char **er_msg);

extern ns_dbd_pool_t *dbd_pool;



#ifdef _ZET_HAS_PDF
// debian: apt install libcairo-dev

#include <math.h>
#include <cairo/cairo.h>
#include <cairo/cairo-pdf.h>
#include <fontconfig/fontconfig.h>

void z_pdf(apr_pool_t *m, const char *s, const char *d, double w, double h);

#endif /* _ZET_HAS_PDF */


#ifdef download
#define Z_APACHE_RESPONSE_BUFFER(r, bf, sz, ct) do {\
  if (sz > 0) {\
    void *buffer = (void*)apr_palloc(r->pool, sizeof(unsigned char)*(sz));\
    if (buffer != NULL) {\
      memcpy(buffer, bf, sz);\
      ap_set_content_type(r, ct);\
      ap_rwrite(buffer, sz, r);\
    }\
  }\
} while (0)

#define Z_APACHE_RESPONSE_DOWNLOAD(r, bf, sz, fn) do {\
  if ((r!=NULL) && (bf!=NULL) && (sz > 0) && (fn!=NULL)){\
    apr_table_setn(r->headers_out, "Expires", "Mon, 23 Oct 1972 16:00:00 GMT");\
    apr_table_setn(r->headers_out, "Pragma", "hack");\
    apr_table_setn(r->headers_out, "Cache-Control", "must-revalidate,post-check=0,pre-check=0");\
    apr_table_setn(r->headers_out, "Cache-Control", "private");\
    apr_table_setn(r->headers_out, "Content-Description", "File Transfer");\
    apr_table_setn(r->headers_out, "Content-Disposition", apr_psprintf(r->pool, "attachment; filename=\"%s\"", fn));\
    apr_table_setn(r->headers_out, "Content-Transfer-Encoding", "binary");\
    apr_table_setn(r->headers_out, "Content-Length", apr_psprintf(r->pool, "%" APR_UINT64_T_FMT, sz));\
    Z_APACHE_RESPONSE_BUFFER(r, bf, sz, "application/download");\
  }\
} while (0)

#endif







#ifdef __cplusplus
}
#endif
#endif /* MICRODEVTOOLS_H */
