# MicroDevTools
Microservices DevTools

## Install system dependencies
```bash
apt install clang make curl git python autoconf libtool-bin libexpat1-dev \
            cmake libssl-dev libmariadb-dev libpq-dev libsqlite3-dev \
            unixodbc-dev
```
#### Install GNUstep OBJ-C support (optional)
```bash
apt install gnustep-devel gobjc
ln -s /usr/lib/gcc/x86_64-linux-gnu/10/include/objc /usr/local/include/objc
```

## Create a new microservice project
### Make the project structure
```bash
/path/to/my/project/
  Makefile
  main.c
```
### Get the latest Apache Portable Runtime version
```bash
git clone https://github.com/apache/apr.git apr
cd apr && ./buildconf \
       && ./configure --prefix=/tmp/apr --with-mysql --with-pgsql --with-sqlite3 --with-odbc \
       && make \
       && make install \
       && mv /tmp/apr/include/apr-2 /path/to/my/project/apr-2/include \
       && mv /tmp/apr-install/lib /path/to/my/project/apr-2/apr-2 \
       && rm -rf /tmp/apr
```
### Get the latest JSON-c version
```bash
git clone https://github.com/json-c/json-c.git json-c
mkdir jsonc && cd jsonc
            && cmake ../json-c -DCMAKE_INSTALL_PREFIX=/tmp/jsonc \
            && make \
            && make install \
            && mv /tmp/jsonc/include/json-c /path/to/my/project/json-c/include \
            && mv /tmp/jsonc/lib /path/to/my/project/json-c/lib \
            && rm -rf /tmp/jsonc \
            && cd ..
            && rm -rf jsonc
```
### Get the latest Mongoose version
```bash
git clone https://github.com/cesanta/mongoose.git mongoose
cp mongoose/mongoose.* /path/to/my/project/mongoose
```
### Get latest MicroDevTools version
```bash
git clone https://github.com/riccardovacirca/microdevtools.git microdevtools
cp microdevtools/microdevtools.* /path/to/my/project/microdevtools
```
### Create an HelloWorld C microservice
```c
#include "microdevtools.h"

int HelloWorldController(ns_service_t *s) {
  ns_http_response_hd_set(s->response, "Content-Type", "application/json");
  const char *msg = ns_json_encode(s->pool, "Hello, World!", NS_JSON_T_STRING);
  ns_printf(s, "{\"err\":%s,\"log\":%s,\"out\":%s}", "false", "null", msg);
  return 200;
}

ns_dbd_pool_t *dbd_pool;
volatile sig_atomic_t server_run = 1;

void ns_handler(ns_service_t *s) {
  ns_route(s, "GET", "/api/hello", HelloWorldController);
}

void ns_signal_exit(int signum) {
  if (signum == SIGTERM || signum == SIGINT) {
    server_run = 0;
  }
}

int main(int argc, char **argv) {
  struct sigaction sig_action;
  ns_signal_handler(&sig_action, ns_signal_exit);
  apr_status_t rv = apr_initialize();
  if (rv != APR_SUCCESS) exit(EXIT_FAILURE);
  apr_pool_t *mp;
  rv = apr_pool_create(&mp, NULL);
  if (rv != APR_SUCCESS) exit(EXIT_FAILURE);
  ns_server_t *s = ns_server_alloc(mp);
  if (s == NULL) exit(EXIT_FAILURE);
  char *er_msg;
  if (!ns_server_init(mp, &s, argc, argv, &(er_msg))) exit(EXIT_FAILURE);
  dbd_pool = NULL;
  if (s->dbd_driver != NULL) {
    if (s->dbd_conn_s != NULL) {
      rv = apr_dbd_init(mp);
      if (rv == APR_SUCCESS) {
        if (!ns_dbd_pool_alloc(mp)) exit(EXIT_FAILURE);
        if (!ns_dbd_pool_init(mp, s->dbd_driver, s->dbd_conn_s)) {
          exit(EXIT_FAILURE);
        }
      }
    }
  }
  if (DAEMON) ns_daemonize();
  if (MONGOOSE) {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    if (TLS) {
      if (s->addr_s) {
        mg_http_listen(&mgr, s->addr, ns_http_request_handler, NULL);
        mg_http_listen(&mgr, s->addr_s, ns_http_request_handler, (void*)s);
      }
    } else {
      mg_http_listen(&mgr, s->addr, ns_http_request_handler, (void*)s);
    }
    while (server_run) mg_mgr_poll(&mgr, 1000);
    mg_mgr_free(&mgr);
  }
  if(dbd_pool) ns_dbd_pool_destroy();
  ns_server_destroy(s);
  apr_pool_destroy(mp);
  apr_terminate();
  return 0;
}
```
#### Makefile
```makefile
CC:=clang
CFLAGS:=-std=gnu99 -D_MONGOOSE \$(TLS_2WAY)
INCLUDES:=-I. -I${APP_ROOT}/.tools/apr-2/include -I${APP_ROOT}/.tools/json-c/include -I${APP_ROOT}/.tools/mongoose -I${APP_ROOT}/.tools/microdevtools
LIBS:=-L${APP_ROOT}/.tools/apr-2/lib -L${APP_ROOT}/.tools/json-c/lib
LDFLAGS:=-lapr-2 -ljson-c -lssl -lcrypto
SRC:=${APP_ROOT}/.tools/mongoose/mongoose.c ${APP_ROOT}/.tools/microdevtools/microdevtools.c main.c

all: debug

debug:
	\$(eval CFLAGS:=-g -D_DEBUG \$(CFLAGS))
	\$(CC) \$(CFLAGS) -o ${name} \$(SRC) \$(INCLUDES) \$(LIBS) \$(LDFLAGS)

release:
	\$(eval CFLAGS:=-D_DAEMON \$(CFLAGS))
	\$(CC) \$(CFLAGS) -o ${name} \$(SRC) \$(INCLUDES) \$(LIBS) \$(LDFLAGS)
```

### Create an HelloWorld ObjC microservice
```c
#import "microdevtools.h"

int HelloWorldController(ns_service_t *s) {
  @autoreleasepool {
    NSString *hello;
    ns_http_response_hd_set(s->response, "Content-Type", "application/json");
    const char *msg = ns_json_encode(s->pool, "Hello from ${name}!", NS_JSON_T_STRING);
    ns_printf(s, "{\"err\":%s,\"log\":%s,\"out\":%s}", "false", "null", msg);
    return 200;
  }
}

ns_dbd_pool_t *dbd_pool;
volatile sig_atomic_t server_run = 1;

void ns_handler(ns_service_t *s) {
  ns_route(s, "GET", "/api/hello", HelloWorldController);
}

void ns_signal_exit(int signum) {
  if (signum == SIGTERM || signum == SIGINT) {
    server_run = 0;
  }
}

int main(int argc, char **argv) {
  struct sigaction sig_action;
  ns_signal_handler(&sig_action, ns_signal_exit);
  apr_status_t rv = apr_initialize();
  if (rv != APR_SUCCESS) exit(EXIT_FAILURE);
  apr_pool_t *mp;
  rv = apr_pool_create(&mp, NULL);
  if (rv != APR_SUCCESS) exit(EXIT_FAILURE);
  ns_server_t *s = ns_server_alloc(mp);
  if (s == NULL) exit(EXIT_FAILURE);
  char *er_msg;
  if (!ns_server_init(mp, &s, argc, argv, &(er_msg))) exit(EXIT_FAILURE);
  dbd_pool = NULL;
  if (s->dbd_driver != NULL) {
    if (s->dbd_conn_s != NULL) {
      rv = apr_dbd_init(mp);
      if (rv == APR_SUCCESS) {
        if (!ns_dbd_pool_alloc(mp)) exit(EXIT_FAILURE);
        if (!ns_dbd_pool_init(mp, s->dbd_driver, s->dbd_conn_s)) {
          exit(EXIT_FAILURE);
        }
      }
    }
  }
  if (DAEMON) ns_daemonize();
  if (MONGOOSE) {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    if (TLS) {
      if (s->addr_s) {
        mg_http_listen(&mgr, s->addr, ns_http_request_handler, NULL);
        mg_http_listen(&mgr, s->addr_s, ns_http_request_handler, (void*)s);
      }
    } else {
      mg_http_listen(&mgr, s->addr, ns_http_request_handler, (void*)s);
    }
    while (server_run) mg_mgr_poll(&mgr, 1000);
    mg_mgr_free(&mgr);
  }
  if(dbd_pool) ns_dbd_pool_destroy();
  ns_server_destroy(s);
  apr_pool_destroy(mp);
  apr_terminate();
  return 0;
}
```
#### Makefile
```makefile
CC:=clang
CFLAGS:=-std=gnu99 -D_MONGOOSE -D_NATIVE_OBJC_EXCEPTIONS -fconstant-string-class=NSConstantString
INCLUDES:=-I. -I./apr-2/include -I./json-c/include -I./mongoose -I./microdevtools -I `gnustep-config --variable=GNUSTEP_SYSTEM_HEADERS\`
LIBS:=-L./apr-2/lib -L./json-c/lib -L \`gnustep-config --variable=GNUSTEP_SYSTEM_LIBRARIES\`
LDFLAGS:=-lapr-2 -ljson-c -lssl -lcrypto -lgnustep-base -lobjc
SRC:=./mongoose/mongoose.c ./microdevtools/microdevtools.c main.m

all:
	$(eval CFLAGS:=-D_DAEMON \$(CFLAGS))
	$(CC) $(CFLAGS) -o hello $(SRC) $(INCLUDES) $(LIBS) $(LDFLAGS)

debug:
	$(eval CFLAGS:=-g -D_DEBUG \$(CFLAGS))
	$(CC) $(CFLAGS) -o hello $(SRC) $(INCLUDES) $(LIBS) $(LDFLAGS)
```
### Compile and run the HelloWorld microservice (debug version)
```bash
make debug
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./apr-2/lib:./json-c/lib ./hello -h 0.0.0.0 -p 2310 -P 2443 -l hello.log
```
### Test from a second terminal
#### HTTP
```bash
curl -i "http://localhost:2310/api/hello"
```
#### HTTPS
```bash
curl -i "https://localhost:2443/api/hello"
```
