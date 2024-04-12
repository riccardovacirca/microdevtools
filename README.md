# MicroDevTools

Microservices DevTools

#### Table of Contents

[Install system dependencies](#install-system-dependencies)  
[Create a new microservices-based project](#create-a-new-microservices-based-project)  
[Install GNUstep OBJ-C support (optional)](#install-gnustep-obj-c-support-optional)  
[Create a new microservices-based project](#create-a-new-microservices-based-project)  
[Get the latest verison of Apache Portable Runtime](#get-the-latest-verison-of-apache-portable-runtime)  
[Get the latest version of JSON-c](#get-the-latest-version-of-json-c)  
[Get the latest version of Mongoose](#get-the-latest-version-of-mongoose)  
[Get latest version of MicroDevTools](#get-latest-version-of-microdevtools)  
[Create a HelloWorld microservice in C](#create-a-helloworld-microservice-in-c)  
[Create a HelloWorld microservice in Objective-c](#create-a-helloworld-microservice-in-objective-c)  
[Compile and run the HelloWorld microservice (debug version)](#compile-and-run-the-helloworld-microservice-debug-version)  
[Connect to a PostgreSQL database](#connect-to-a-postgresql-database)  
[Connect to a MySQL/MariaDB database](#connect-to-a-mysql-mariadb-database)  
[Enable TLS](#enable-tls)  
[Create a simple Nginx API gateway](#create-a-simple-nginx-api-gateway)  

### Install system dependencies

```bash
sudo apt install clang make curl git python autoconf libtool-bin libexpat1-dev \
                 cmake libssl-dev libmariadb-dev libpq-dev libsqlite3-dev \
                 unixodbc-dev libapr1-dev libaprutil1-dev libaprutil1-dbd-mysql \
                 libaprutil1-dbd-pgsql libaprutil1-dbd-sqlite3 libjson-c-dev
```

### Install GNUstep OBJ-C support (optional)

```bash
sudo apt install gnustep-devel gobjc \
  && ln -s /usr/lib/gcc/x86_64-linux-gnu/10/include/objc /usr/local/include/objc
```

### Get the latest version of Mongoose

```bash
git clone https://github.com/cesanta/mongoose.git mongoose
```

### Get latest version of MicroDevTools

```bash
git clone https://github.com/riccardovacirca/microdevtools.git microdevtools
```

### Get the latest verison of Apache Portable Runtime (optional)

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

### Get the latest version of JSON-c (optional)

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

### Create a new microservices-based project
#### Project structure

<pre>mongoose/
microdevtools/
myapp/
  api/
    helloworld/
      Makefile
      main.c
</pre>

```bash
mkdir -p myapp/api/helloworld
```

### Create a HelloWorld microservice in C

```bash
nano myapp/api/helloworld/helloworld.c
```

```c
#include "microdevtools.h"

int HelloWorldController(mdt_service_t *s) {
  mdt_http_response_hd_set(s->response, "Content-Type", "application/json");
  const char *msg = mdt_json_encode(s->pool, "Hello, World!", MDT_JSON_T_STRING);
  mdt_printf(s, "{\"err\":%s,\"log\":%s,\"out\":%s}", "false", "null", msg);
  return 200;
}

void mdt_handler(mdt_service_t *s) {
  mdt_route(s, "GET", "/api/helloworld", HelloWorldController);
}
```

```bash
nano myapp/api/helloworld/main.c
```

```c
#include "microdevtools.h"

mdt_dbd_pool_t *dbd_pool;
volatile sig_atomic_t server_run = 1;

void mdt_signal_exit(int signum) {
  if (signum == SIGTERM || signum == SIGINT) {
    server_run = 0;
  }
}

int main(int argc, char **argv) {
  struct sigaction sig_action;
  mdt_signal_handler(&sig_action, mdt_signal_exit);
  apr_status_t rv = apr_initialize();
  if (rv != APR_SUCCESS) exit(EXIT_FAILURE);
  apr_pool_t *mp;
  rv = apr_pool_create(&mp, NULL);
  if (rv != APR_SUCCESS) exit(EXIT_FAILURE);
  mdt_server_t *s = mdt_server_alloc(mp);
  if (s == NULL) exit(EXIT_FAILURE);
  char *er_msg;
  if (!mdt_server_init(mp, &s, argc, argv, &(er_msg))) exit(EXIT_FAILURE);
  dbd_pool = NULL;
  if (s->dbd_driver != NULL) {
    if (s->dbd_conn_s != NULL) {
      rv = apr_dbd_init(mp);
      if (rv == APR_SUCCESS) {
        if (!mdt_dbd_pool_alloc(mp)) exit(EXIT_FAILURE);
        if (!mdt_dbd_pool_init(mp, s->dbd_driver, s->dbd_conn_s)) {
          exit(EXIT_FAILURE);
        }
      }
    }
  }
  if (DAEMON) mdt_daemonize();
  if (MONGOOSE) {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    if (TLS && s->addr_s) {
      mg_http_listen(&mgr, s->addr, mdt_http_request_handler, NULL);
      mg_http_listen(&mgr, s->addr_s, mdt_http_request_handler, (void*)s);
    } else {
      mg_http_listen(&mgr, s->addr, mdt_http_request_handler, (void*)s);
    }
    while (server_run) mg_mgr_poll(&mgr, 1000);
    mg_mgr_free(&mgr);
  }
  if(dbd_pool) mdt_dbd_pool_destroy();
  mdt_server_destroy(s);
  apr_pool_destroy(mp);
  apr_terminate();
  return 0;
}
```

```bash
nano myapp/api/helloworld/Makefile
```

```makefile
CC:=clang
CFLAGS:=-std=gnu99 -D_MONGOOSE
INCLUDES:=-I. -I/usr/include/apr-1.0 -I/usr/include/json-c -I../../../mongoose -I../../../microdevtools
LDFLAGS:=-lapr-2 -ljson-c -lssl -lcrypto
SRC:=../../../mongoose/mongoose.c ../../../microdevtools/microdevtools.c helloworld.c main.c

# LIBS:=-L../../../apr-2/lib -L../../../json-c/lib
# LOAD_LIB:=LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:../../apr-2/lib:../../json-c/lib 

# TLS:=-DMG_TLS=MG_TLS_OPENSSL -D_TLS

all:
	$(eval CFLAGS:=$(CFLAGS) -D_DAEMON $(TLS))
	$(CC) $(CFLAGS) -o helloworld $(SRC) $(INCLUDES) $(LIBS) $(LDFLAGS)

debug:
	$(eval CFLAGS:=$(CFLAGS) $(TLS) -g -D_DEBUG)
	$(CC) $(CFLAGS) -o helloworld $(SRC) $(INCLUDES) $(LIBS) $(LDFLAGS)

run:
	$(LOAD_LIB) ./helloworld -h 0.0.0.0 -p 2310 -P 2443 -l helloworld.log

.PHONY: all debug run
```

To use a different installation of apr and json-c, uncomment the following lines
and set the correct path:

```makefile
# LIBS:=-L../../../apr-2/lib -L../../../json-c/lib
# LOAD_LIB:=LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:../../apr-2/lib:../../json-c/lib 
```

### Create a HelloWorld microservice in Objective-c

```bash
nano myapp/api/helloworld/helloworld.m
```

```c
#import "microdevtools.h"

int HelloWorldController(mdt_service_t *s) {
  @autoreleasepool {
    NSString *hello;
    mdt_http_response_hd_set(s->response, "Content-Type", "application/json");
    const char *msg = mdt_json_encode(s->pool, "Hello, World!", MDT_JSON_T_STRING);
    mdt_printf(s, "{\"err\":%s,\"log\":%s,\"out\":%s}", "false", "null", msg);
    return 200;
  }
}

void mdt_handler(mdt_service_t *s) {
  mdt_route(s, "GET", "/api/helloworld", HelloWorldController);
}
```

```bash
nano myapp/api/helloworld/main.m
```

```c
#import "microdevtools.h"

mdt_dbd_pool_t *dbd_pool;
volatile sig_atomic_t server_run = 1;

void mdt_signal_exit(int signum) {
  if (signum == SIGTERM || signum == SIGINT) {
    server_run = 0;
  }
}

int main(int argc, char **argv) {
  struct sigaction sig_action;
  mdt_signal_handler(&sig_action, mdt_signal_exit);
  apr_status_t rv = apr_initialize();
  if (rv != APR_SUCCESS) exit(EXIT_FAILURE);
  apr_pool_t *mp;
  rv = apr_pool_create(&mp, NULL);
  if (rv != APR_SUCCESS) exit(EXIT_FAILURE);
  mdt_server_t *s = mdt_server_alloc(mp);
  if (s == NULL) exit(EXIT_FAILURE);
  char *er_msg;
  if (!mdt_server_init(mp, &s, argc, argv, &(er_msg))) exit(EXIT_FAILURE);
  dbd_pool = NULL;
  if (s->dbd_driver != NULL) {
    if (s->dbd_conn_s != NULL) {
      rv = apr_dbd_init(mp);
      if (rv == APR_SUCCESS) {
        if (!mdt_dbd_pool_alloc(mp)) exit(EXIT_FAILURE);
        if (!mdt_dbd_pool_init(mp, s->dbd_driver, s->dbd_conn_s)) {
          exit(EXIT_FAILURE);
        }
      }
    }
  }
  if (DAEMON) mdt_daemonize();
  if (MONGOOSE) {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    if (TLS && s->addr_s) {
      mg_http_listen(&mgr, s->addr, mdt_http_request_handler, NULL);
      mg_http_listen(&mgr, s->addr_s, mdt_http_request_handler, (void*)s);
    } else {
      mg_http_listen(&mgr, s->addr, mdt_http_request_handler, (void*)s);
    }
    while (server_run) mg_mgr_poll(&mgr, 1000);
    mg_mgr_free(&mgr);
  }
  if(dbd_pool) mdt_dbd_pool_destroy();
  mdt_server_destroy(s);
  apr_pool_destroy(mp);
  apr_terminate();
  return 0;
}
```

```bash
nano myapp/api/helloworld/Makefile
```

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

# LIBS:=-L../../../apr-2/lib -L../../../json-c/lib
# LOAD_LIB:=LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:../../apr-2/lib:../../json-c/lib 

# TLS:=-DMG_TLS=MG_TLS_OPENSSL -D_TLS

all:
	$(eval CFLAGS:=$(CFLAGS) -D_DAEMON $(TLS))
	$(CC) $(CFLAGS) -o helloworld $(SRC) $(INCLUDES) $(LIBS) $(LDFLAGS)

debug:
	$(eval CFLAGS:=$(CFLAGS) $(TLS) -g -D_DEBUG)
	$(CC) $(CFLAGS) -o helloworld $(SRC) $(INCLUDES) $(LIBS) $(LDFLAGS)

run:
	$(LOAD_LIB) ./helloworld -h 0.0.0.0 -p 2310 -P 2443 -l helloworld.log

.PHONY: all debug run
```

To use a different installation of apr and json-c, uncomment the following lines
and set the correct path:

```makefile
# LIBS:=-L../../../apr-2/lib -L../../../json-c/lib
# LOAD_LIB:=LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:../../apr-2/lib:../../json-c/lib 
```

### Compile and run the HelloWorld microservice (debug version)

```bash
cd myapp/api/helloworld && make debug && make run
```

<code>TEST HTTP</code>

```bash
curl -i "http://localhost:2310/api/helloworld"
```

### Connect to a PostgreSQL database

Connect to a PostgreSQL database by starting the service with the following
additional arguments from the command line:

```
-d pgsql -D "hostaddr=127.0.0.1 host=localhost port=5432 user=bob password=secret dbname=test"
```

### Connect to a MySQL/MariaDB database

Connect to a MySQL/MariaDB database by starting the service with the following
additional arguments from the command line:

```
-d mysql -D "host=127.0.0.1,port=3306,user=bob,pass=secret,dbname=test"
```

### Enable TLS

Create and run a <code>myapp/api/helloworld/cert.sh</code> bash script:

```bash
#!/bin/bash
mkdir -p /tmp/microdevtools

if ! test -e "/tmp/microdevtools/ca_root.key"; then
  openssl genrsa -out /tmp/microdevtools/ca_root.key 4096 \
    && openssl req -new -x509 -days 365 -key /tmp/microdevtools/ca_root.key \
    -out /tmp/microdevtools/ca_root.crt -subj "/CN=MDT_ROOT_CA"
  rm -rf $1.key $1.crs $1.crt certs.h
fi

if ! test -e "$1.key"; then
  openssl genrsa -out $1.key 2048 \
    && openssl req -new -key $1.key -out $1.csr -subj "/CN=$1" \
    && openssl x509 -req -days 365 -in $1.csr -CA /tmp/microdevtools/ca_root.crt -CAkey /tmp/microdevtools/ca_root.key -set_serial 01 -out $1.crt
fi

ca_crt_file=/tmp/microdevtools/ca_root.crt
ca_c_variable_name=s_tls_ca
server_crt_file=$1.crt
server_crt_c_variable_name=s_tls_cert
server_key_file=$1.key
server_key_c_variable_name=s_tls_key

ca_crt_variable="const char *${ca_c_variable_name} ="
while IFS= read -r line; do
  ca_crt_variable="${ca_crt_variable}\n\"${line}\\\\n\""
done < "$ca_crt_file"
ca_crt_variable="${ca_crt_variable};"

server_crt_variable="const char *${server_crt_c_variable_name} ="
while IFS= read -r line; do
  server_crt_variable="${server_crt_variable}\n\"${line}\\\\n\""
done < "$server_crt_file"
server_crt_variable="${server_crt_variable};"

server_key_variable="const char *${server_key_c_variable_name} ="
while IFS= read -r line; do
  server_key_variable="${server_key_variable}\n\"${line}\\\\n\""
done < "$server_key_file"
server_key_variable="${server_key_variable};"

echo -e "#ifndef CERT_H" > certs.h
echo -e "#define CERT_H\n" >> certs.h
echo -e "#ifdef _TLS" >> certs.h
echo -e "#ifdef _TLS_TWOWAY" >> certs.h
echo -e "$ca_crt_variable" >> certs.h
echo -e "#endif\n" >> certs.h
echo -e "$server_crt_variable" >> certs.h
echo -e "$server_key_variable" >> certs.h
echo -e "#endif" >> certs.h
echo -e "#endif /* CERT_H */" >> certs.h
```

```bash
chmod +x cert.sh && ./cert.sh helloworld
```

Uncomment the following line in the service Makefile:

```makefile
# TLS:=-DMG_TLS=MG_TLS_OPENSSL -D_TLS
```

Compile and run the HelloWorld microservice (debug version)

```bash
make debug && make run
```

<code>TEST HTTPS</code>

```bash
curl -k -i "https://localhost:2443/api/helloworld"
```

### Create a simple Nginx API gateway

<code>/etc/nginx/sites-available/myapp.conf</code>

```bash
sudo nano /etc/nginx/sites-available/myapp.conf
```

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

```bash
sudo nano /etc/nginx/sites-available/myapp_hello_location.conf
```

```nginx
location /api/helloworld/ {
  rewrite ^/api/helloworld(.*) /api$1 break;
  proxy_pass http://myapp-helloworld;
}
```

<code>/etc/nginx/sites-available/myapp_*_upstream.conf</code>

```bash
sudo nano /etc/nginx/sites-available/myapp_*_upstream.conf
```

```nginx
upstream myapp-helloworld {
  server localhost:2310 fail_timeout=10s max_fails=3;
}
```

```bash
sudo nginx -t
```

```bash
sudo service nginx restart
```
