# MicroDevTools
Microservices DevTools

## Install system dependencies
```bash
sudo apt install clang make curl git python autoconf libtool-bin libexpat1-dev \
                 cmake libssl-dev libmariadb-dev libpq-dev libsqlite3-dev \
                 unixodbc-dev
```

#### Install GNUstep OBJ-C support (optional)
```bash
sudo apt install gnustep-devel gobjc \
  && ln -s /usr/lib/gcc/x86_64-linux-gnu/10/include/objc /usr/local/include/objc
```

## Create a new microservices-based project
### Project structure
```
myapp/
  ...
  api/
    helloworld/
      Makefile
      main.c
```

```bash
mkdir -p myapp/api/helloworld
```

### Get the latest verison of Apache Portable Runtime
```bash
git clone https://github.com/apache/apr.git apr
```
```bash
mkdir -p myapp/apr-2 \
  && cd apr \
  && ./buildconf \
  && ./configure --prefix=/tmp/apr --with-mysql --with-pgsql --with-sqlite3 --with-odbc \
  && make \
  && make install \
  && mv /tmp/apr/include/apr-2 ../myapp/apr-2/include \
  && mv /tmp/apr/lib ../myapp/apr-2 \
  && rm -rf /tmp/apr \
  && cd ..
```

### Get the latest version of JSON-c
```bash
git clone https://github.com/json-c/json-c.git json-c
```
```bash
mkdir -p myapp/json-c \
  && mkdir jsonc \
  && cd jsonc \
  && cmake ../json-c -DCMAKE_INSTALL_PREFIX=/tmp/jsonc \
  && make \
  && make install \
  && mv /tmp/jsonc/include/json-c ../myapp/json-c/include \
  && mv /tmp/jsonc/lib ../myapp/json-c/lib \
  && rm -rf /tmp/jsonc \
  && cd .. \
  && rm -rf jsonc /tmp/jsonc
```

### Get the latest version of Mongoose
```bash
git clone https://github.com/cesanta/mongoose.git mongoose
```
```bash
mkdir -p myapp/mongoose \
  && cp mongoose/mongoose.* myapp/mongoose
```

### Get latest version of MicroDevTools
```bash
git clone https://github.com/riccardovacirca/microdevtools.git microdevtools
```
```bash
mkdir -p myapp/microdevtools \
  && cp microdevtools/microdevtools.* myapp/microdevtools
```

### Create a HelloWorld microservice in C

<code>myapp/api/helloworld/helloworld.c</code>
```c
#include "microdevtools.h"

int HelloWorldController(ns_service_t *s) {
  ns_http_response_hd_set(s->response, "Content-Type", "application/json");
  const char *msg = ns_json_encode(s->pool, "Hello, World!", NS_JSON_T_STRING);
  ns_printf(s, "{\"err\":%s,\"log\":%s,\"out\":%s}", "false", "null", msg);
  return 200;
}

void ns_handler(ns_service_t *s) {
  ns_route(s, "GET", "/api/helloworld", HelloWorldController);
}
```
<code>myapp/api/helloworld/main.c</code>
```c
#include "microdevtools.h"

ns_dbd_pool_t *dbd_pool;
volatile sig_atomic_t server_run = 1;

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
    if (TLS && s->addr_s) {
      mg_http_listen(&mgr, s->addr, ns_http_request_handler, NULL);
      mg_http_listen(&mgr, s->addr_s, ns_http_request_handler, (void*)s);
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
<code>Makefile</code>
```makefile
CC:=clang
CFLAGS:=-std=gnu99 -D_MONGOOSE
INCLUDES:=-I. -I../../apr-2/include -I../../json-c/include -I../../mongoose -I../../microdevtools
LIBS:=-L../../apr-2/lib -L../../json-c/lib
LDFLAGS:=-lapr-2 -ljson-c -lssl -lcrypto
SRC:=../../mongoose/mongoose.c ../../microdevtools/microdevtools.c helloworld.c main.c

all:
	$(eval CFLAGS:=-D_DAEMON $(CFLAGS))
	$(CC) $(CFLAGS) -o helloworld $(SRC) $(INCLUDES) $(LIBS) $(LDFLAGS)

debug:
	$(eval CFLAGS:=-g -D_DEBUG $(CFLAGS))
	$(CC) $(CFLAGS) -o helloworld $(SRC) $(INCLUDES) $(LIBS) $(LDFLAGS)
```

### Create a HelloWorld microservice in Objective-c
<code>myapp/api/helloworld/helloworld.c</code>
```c
#import "microdevtools.h"

int HelloWorldController(ns_service_t *s) {
  @autoreleasepool {
    NSString *hello;
    ns_http_response_hd_set(s->response, "Content-Type", "application/json");
    const char *msg = ns_json_encode(s->pool, "Hello, World!", NS_JSON_T_STRING);
    ns_printf(s, "{\"err\":%s,\"log\":%s,\"out\":%s}", "false", "null", msg);
    return 200;
  }
}

void ns_handler(ns_service_t *s) {
  ns_route(s, "GET", "/api/helloworld", HelloWorldController);
}
```
<code>myapp/api/helloworld/main.m</code>
```c
#import "microdevtools.h"

ns_dbd_pool_t *dbd_pool;
volatile sig_atomic_t server_run = 1;

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
    if (TLS && s->addr_s) {
      mg_http_listen(&mgr, s->addr, ns_http_request_handler, NULL);
      mg_http_listen(&mgr, s->addr_s, ns_http_request_handler, (void*)s);
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
<code>Makefile</code>
```makefile
CC:=clang
CFLAGS:=-std=gnu99 -D_MONGOOSE -D_NATIVE_OBJC_EXCEPTIONS \
        -fconstant-string-class=NSConstantString
INCLUDES:=-I. -I../../apr-2/include -I../../json-c/include -I../../mongoose \
          -I../../microdevtools -I `gnustep-config --variable=GNUSTEP_SYSTEM_HEADERS`
LIBS:=-L../../apr-2/lib -L../../json-c/lib \
      -L `gnustep-config --variable=GNUSTEP_SYSTEM_LIBRARIES`
LDFLAGS:=-lapr-2 -ljson-c -lssl -lcrypto -lgnustep-base -lobjc
SRC:=../../mongoose/mongoose.c ../../microdevtools/microdevtools.c helloworld.m main.m

all:
	$(eval CFLAGS:=-D_DAEMON \$(CFLAGS))
	$(CC) $(CFLAGS) -o helloworld $(SRC) $(INCLUDES) $(LIBS) $(LDFLAGS)

debug:
	$(eval CFLAGS:=-g -D_DEBUG \$(CFLAGS))
	$(CC) $(CFLAGS) -o helloworld $(SRC) $(INCLUDES) $(LIBS) $(LDFLAGS)
```

### Compile and run the HelloWorld microservice (debug version)
```bash
make debug
```
```bash
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../../apr-2/lib:../../json-c/lib \
  ./helloworld -h 0.0.0.0 -p 2310 -P 2443 -l helloworld.log
```
<code>TEST HTTP</code>
```bash
curl -i "http://localhost:2310/api/helloworld"
```
<code>TEST HTTPS</code>
```bash
curl -i "https://localhost:2443/api/helloworld"
```

### Create a simple Nginx API gateway
<code>/etc/nginx/sites-available/myapp.conf</code>
```nginx
include /etc/nginx/sites-available/myapp_*_upstream.conf;
server {
  listen 80;
  server_name localhost;
  include /etc/nginx/sites-available/myapp_*_location.conf;
  location / {
    root /var/www/html/myapp;
  }
}
```

<code>/etc/nginx/sites-available/myapp_hello_location.conf</code>
```nginx
location /api/helloworld/ {
  rewrite ^/api/helloworld(.*) /api$1 break;
  proxy_pass https://myapp-helloworld;
}
```

<code>/etc/nginx/sites-available/myapp_*_upstream.conf</code>
```nginx
upstream myapp-helloworld {
  server localhost:2443 fail_timeout=10s max_fails=3;
  server localhost:2444 fail_timeout=10s max_fails=3;
  server localhost:2445 fail_timeout=10s max_fails=3;
}
```
