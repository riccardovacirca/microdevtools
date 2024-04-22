# MicroDevTools

Microservices DevTools

<!-- #### Table of Contents

[Install](#install)<br>
[Create a new microservices-based project](#create-a-new-microservices-based-project)<br>
[Install GNUstep OBJ-C support (optional)](#install-gnustep-obj-c-support-optional)<br>
[Create a new microservices-based project](#create-a-new-microservices-based-project)<br>
[Get the latest verison of Apache Portable Runtime](#get-the-latest-verison-of-apache-portable-runtime)<br>
[Get the latest version of JSON-c](#get-the-latest-version-of-json-c)<br>
[Get the latest version of Mongoose](#get-the-latest-version-of-mongoose)<br>
[Get latest version of MicroDevTools](#get-latest-version-of-microdevtools)<br>
[Create a HelloWorld microservice in C](#create-a-helloworld-microservice-in-c)<br>
[Create a HelloWorld microservice in Objective-c](#create-a-helloworld-microservice-in-objective-c)<br>
[Compile and run the HelloWorld microservice (debug version)](#compile-and-run-the-helloworld-microservice-debug-version)<br>
[Connect to a PostgreSQL database](#connect-to-a-postgresql-database)<br>
[Connect to a MySQL/MariaDB database](#connect-to-a-mysql-mariadb-database)<br>
[Connect to a SQLite3 database](#connect-to-a-sqlite3-database)<br>
[Enable TLS](#enable-tls)<br>
[Create a simple Nginx API gateway](#create-a-simple-nginx-api-gateway)<br>
 -->
## Install

```bash
sudo apt install \
  clang make curl git python autoconf libtool-bin libexpat1-dev \
  cmake libssl-dev libmariadb-dev libpq-dev libsqlite3-dev \
  unixodbc-dev libapr1-dev libaprutil1-dev libaprutil1-dbd-mysql \
  libaprutil1-dbd-pgsql libaprutil1-dbd-sqlite3 libjson-c-dev
```
```bash
git clone https://github.com/cesanta/mongoose.git mongoose
```
```bash
git clone https://github.com/riccardovacirca/microdevtools.git microdevtools
```
<!-- #### GNUstep OBJ-C support (optional)
```bash
sudo apt install gnustep-devel gobjc \
  && ln -s /usr/lib/gcc/x86_64-linux-gnu/10/include/objc /usr/local/include/objc
```
### The latest verison of Apache Portable Runtime (optional)
```bash
git clone https://github.com/apache/apr.git apr
```
```bash
mkdir -p apr-2 \
  && cd apr \
  && ./buildconf \
  && ./configure --prefix=/tmp/apr --with-mysql --with-pgsql --with-sqlite3 --with-odbc \
  && make \
  && make install \
  && mv /tmp/apr/include/apr-2 ../apr-2/include \
  && mv /tmp/apr/lib ../apr-2 \
  && rm -rf /tmp/apr \
  && cd ..
```
#### The latest version of JSON-c (optional)
```bash
git clone https://github.com/json-c/json-c.git json-c
```
```bash
mkdir -p json-c \
  && mkdir jsonc \
  && cd jsonc \
  && cmake ../json-c -DCMAKE_INSTALL_PREFIX=/tmp/jsonc \
  && make \
  && make install \
  && mv /tmp/jsonc/include/json-c ../json-c/include \
  && mv /tmp/jsonc/lib ../json-c/lib \
  && rm -rf /tmp/jsonc \
  && cd .. \
  && rm -rf jsonc /tmp/jsonc
```
-->
## Create a microservice
```bash
mkdir -p helloworld && nano helloworld/helloworld.c
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
nano helloworld/main.c
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
nano helloworld/Makefile
```
```makefile
CC:=clang
CFLAGS:=-std=gnu99 -D_MONGOOSE
INCLUDES:=-I. -I../mongoose -I../microdevtools -I /usr/include/apr-1.0 -I/usr/include/json-c
LDFLAGS:=-lapr-1 -laprutil-1 -ljson-c -lssl -lcrypto
SRC:=../mongoose/mongoose.c ../microdevtools/microdevtools.c helloworld.c main.c

all:
	$(eval CFLAGS:=$(CFLAGS) -D_DAEMON)
	$(CC) $(CFLAGS) -o helloworld $(SRC) $(INCLUDES) $(LIBS) $(LDFLAGS)

debug:
	$(eval CFLAGS:=$(CFLAGS) -g -D_DEBUG)
	$(CC) $(CFLAGS) -o helloworld $(SRC) $(INCLUDES) $(LIBS) $(LDFLAGS)

run:
	./helloworld -h 0.0.0.0 -p 2310 -P 2443 -l helloworld.log

.PHONY: all debug run
```

<!-- ```makefile
CC:=clang
CFLAGS:=-std=gnu99 -D_MONGOOSE
INCLUDES:=-I. -I../mongoose -I../microdevtools
LDFLAGS:=-lapr-1 -laprutil-1 -ljson-c -lssl -lcrypto
SRC:=../mongoose/mongoose.c ../microdevtools/microdevtools.c helloworld.c main.c

EXTRA_INCLUDES:=-I /usr/include/apr-1.0 -I/usr/include/json-c

# EXTRA_INCLUDES:=-I../apr-2/include -I../json-c/include
# EXTRA_LIBS:=-L../apr-2/lib -L../json-c/lib
# LD_LIBRARY_LOAD:=LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:../apr-2/lib:../json-c/lib 
# TLS:=-DMG_TLS=MG_TLS_OPENSSL -D_TLS

RUN:=./helloworld -h 0.0.0.0 -p 2310 -P 2443 -l helloworld.log

ifdef dbd
  ifeq ($(dbd),pgsql)
    RUN:=$(RUN) -d pgsql -D "hostaddr=127.0.0.1 host=localhost port=5432 user=bob password=secret dbname=test"
  endif
  ifeq ($(dbd),mysql)
    RUN:=$(RUN) -d mysql -D "host=127.0.0.1,port=3306,user=bob,pass=secret,dbname=test"
  endif
  ifeq ($(dbd),sqlite3)
    RUN:=$(RUN) -d sqlite3 -D "test.sqlite"
  endif
endif

all:
ifneq ($(wildcard fs.c),)
	$(eval SRC:=$(SRC) fs.c)
	$(eval CFLAGS:=$(CFLAGS) -DMG_ENABLE_PACKED_FS=1 -D_FS)
endif
	$(eval CFLAGS:=$(CFLAGS) -D_DAEMON $(TLS))
	$(CC) $(CFLAGS) -o helloworld $(SRC) $(INCLUDES) $(EXTRA_INCLUDES) \
	$(LIBS) $(EXTRA_LIBS) $(LDFLAGS)

debug:
ifneq ($(wildcard fs.c),)
	$(eval SRC:=$(SRC) fs.c)
	$(eval CFLAGS:=$(CFLAGS) -DMG_ENABLE_PACKED_FS=1 -D_FS)
endif
	$(eval CFLAGS:=$(CFLAGS) $(TLS) -g -D_DEBUG)
	$(CC) $(CFLAGS) -o helloworld $(SRC) $(INCLUDES) $(EXTRA_INCLUDES) \
	$(LIBS) $(EXTRA_LIBS) $(LDFLAGS)

fs:
	clang -o pack ../mongoose/test/pack.c
	./pack	fs/* > fs.c

run:
	$(LD_LIBRARY_LOAD) $(RUN)

.PHONY: all debug run
``` -->
<!-- 
To use a different installation of apr and json-c, uncomment the following lines
and set the correct path:

```makefile
# EXTRA_INCLUDES:=-I../apr-2/include -I../json-c/include
# EXTRA_LIBS:=-L../apr-2/lib -L../json-c/lib
# LD_LIBRARY_LOAD:=LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:../apr-2/lib:../json-c/lib 
# TLS:=-DMG_TLS=MG_TLS_OPENSSL -D_TLS
```

### Create a HelloWorld microservice in Objective-c

```bash
nano helloworld/helloworld.m
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
nano helloworld/main.m
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
nano helloworld/Makefile
```

```makefile
CC:=clang
CFLAGS:=-D_MONGOOSE -D_NATIVE_OBJC_EXCEPTIONS -fconstant-string-class=NSConstantString
INCLUDES:=-I. -I../mongoose -I../microdevtools -I `gnustep-config --variable=GNUSTEP_SYSTEM_HEADERS`
LIBS:=-L `gnustep-config --variable=GNUSTEP_SYSTEM_LIBRARIES`
LDFLAGS:=-lapr-1 -laprutil-1 -ljson-c -lssl -lcrypto -lgnustep-base -lobjc
SRC:=../mongoose/mongoose.c ../microdevtools/microdevtools.c helloworld.m main.m

EXTRA_INCLUDES:=-I /usr/include/apr-1.0 -I/usr/include/json-c

# EXTRA_INCLUDES:=-I../apr-2/include -I../json-c/include
# EXTRA_LIBS:=-L../apr-2/lib -L../json-c/lib
# LD_LIBRARY_LOAD:=LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:../apr-2/lib:../json-c/lib 
# TLS:=-DMG_TLS=MG_TLS_OPENSSL -D_TLS

RUN:=./helloworld -h 0.0.0.0 -p 2310 -P 2443 -l helloworld.log

ifdef dbd
  ifeq ($(dbd),pgsql)
    RUN:=$(RUN) -d pgsql -D "hostaddr=127.0.0.1 host=localhost port=5432 user=bob password=secret dbname=test"
  endif
  ifeq ($(dbd),mysql)
    RUN:=$(RUN) -d mysql -D "host=127.0.0.1,port=3306,user=bob,pass=secret,dbname=test"
  endif
  ifeq ($(dbd),sqlite3)
    RUN:=$(RUN) -d sqlite3 -D "test.sqlite"
  endif
endif

all:
ifneq ($(wildcard fs.c),)
	$(eval SRC:=$(SRC) fs.c)
	$(eval CFLAGS:=$(CFLAGS) -DMG_ENABLE_PACKED_FS=1 -D_FS)
endif
	$(eval CFLAGS:=$(CFLAGS) -D_DAEMON $(TLS))
	$(CC) $(CFLAGS) -o helloworld $(SRC) $(INCLUDES) $(EXTRA_INCLUDES) \
	$(LIBS) $(EXTRA_LIBS) $(LDFLAGS)

debug:
ifneq ($(wildcard fs.c),)
	$(eval SRC:=$(SRC) fs.c)
	$(eval CFLAGS:=$(CFLAGS) -DMG_ENABLE_PACKED_FS=1 -D_FS)
endif
	$(eval CFLAGS:=$(CFLAGS) $(TLS) -g -D_DEBUG)
	$(CC) $(CFLAGS) -o helloworld $(SRC) $(INCLUDES) $(EXTRA_INCLUDES) \
	$(LIBS) $(EXTRA_LIBS) $(LDFLAGS)

fs:
	clang -o pack ../mongoose/test/pack.c
	./pack	fs/* > fs.c

run:
	$(LD_LIBRARY_LOAD) $(RUN)

.PHONY: all debug run
```

To use a different installation of apr and json-c, uncomment the following lines
and set the correct path:

```makefile
# EXTRA_INCLUDES:=-I../apr-2/include -I../json-c/include
# EXTRA_LIBS:=-L../apr-2/lib -L../json-c/lib
# LD_LIBRARY_LOAD:=LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:../apr-2/lib:../json-c/lib 
``` -->

## Make and run
```bash
cd helloworld && make debug && make run
```
<code>TEST HTTP</code>
```bash
curl -i "http://localhost:2310/api/helloworld"
```

<!-- ### Connect to a PostgreSQL database

```bash
sudo apt install postgresql \
  && sudo systemctl start postgresql \
  && sudo -u postgres psql
```

```sql
CREATE USER bob WITH PASSWORD 'secret';
CREATE DATABASE test OWNER bob;
GRANT ALL PRIVILEGES ON DATABASE test TO bob;
\q
```

```bash
psql -U bob -d test -h localhost
```

```sql
CREATE TABLE users (
  id SERIAL PRIMARY KEY,
  username VARCHAR(50) UNIQUE NOT NULL,
  email VARCHAR(100) UNIQUE NOT NULL,
  pass VARCHAR(100) NOT NULL
);
```

```sql
INSERT INTO users (username, email, pass)
VALUES 
  ('user1', 'user1@example.com', 'password1'),
  ('user2', 'user2@example.com', 'password2'),
  ('user3', 'user3@example.com', 'password3');
```

```bash
cd helloworld && make debug && make run dbd=pgsql
```

### Connect to a MySQL/MariaDB database

```bash
sudo apt install mariadb-server && sudo mysql
```

```sql
create database test;
create user 'bob'@'localhost' identified by 'secret';
grant all on test.* to 'bob'@'localhost';
```

```bash
mysql -u bob -p -h localhost
```

```sql
CREATE TABLE users (
  id INT AUTO_INCREMENT PRIMARY KEY,
  username VARCHAR(50) UNIQUE NOT NULL,
  email VARCHAR(100) UNIQUE NOT NULL,
  pass VARCHAR(100) NOT NULL
);

INSERT INTO users (user, email, pass) VALUES
  ('user1', 'user1@example.com', 'password1'),
  ('user2', 'user2@example.com', 'password2'),
  ('user3', 'user3@example.com', 'password3');

\q
```

```bash
cd helloworld \
  && make debug \
  && make run dbd=mysql
```

### Connect to a SQLite3 database

```bash
sudo apt install sqlite3 \
  && sqlite3 test.sqlite
```

```sql
CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    pass VARCHAR(100) NOT NULL
);

INSERT INTO users (username, email, pass) VALUES
    ('user1', 'user1@example.com', 'password1'),
    ('user2', 'user2@example.com', 'password2'),
    ('user3', 'user3@example.com', 'password3');

.exit
```

```bash
cd helloworld \
  && make debug \
  && make run dbd=sqlite3
```

### Enable TLS (optional)

```bash
nano helloworld/certs.sh
```

```bash
#!/bin/bash
mkdir -p /tmp/microdevtools

if ! test -e "/tmp/microdevtools/ca_root.key"; then
  openssl genrsa -out /tmp/microdevtools/ca_root.key 4096
  openssl req -new -x509 -days 365 -key /tmp/microdevtools/ca_root.key \
    -out /tmp/microdevtools/ca_root.crt -subj "/CN=MDT_ROOT_CA"
  rm -rf $1.key $1.crs $1.crt certs.h
fi

if ! test -e "$1.key"; then
  openssl genrsa -out $1.key 2048
  openssl req -new -key $1.key -out $1.csr -subj "/CN=$1"
  openssl x509 -req -days 365 -in $1.csr -CA /tmp/microdevtools/ca_root.crt \
    -CAkey /tmp/microdevtools/ca_root.key -set_serial 01 -out $1.crt
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
chmod +x certs.sh && ./certs.sh helloworld
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

### Compile and install the microservice as a system service

To build the microservice as a system daemon simply use <code>make</code> instead of <code>make debug</code> from the command line.

```bash
sudo cp helloworld /usr/bin/helloworld \
  && sudo chown -R root:root /usr/bin/helloworld \
  && sudo mkdir -p /var/log/helloworld
```

```bash
sudo nano /etc/init.d/helloworld
```

```bash
#!/bin/sh
### BEGIN INIT INFO
# Provides:          helloworld
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Simple helloworld program
# Description:       This script runs the helloworld program.
### END INIT INFO

# Variables
HELLOWORLD_BIN="/usr/bin/helloworld -h 0.0.0.0 -p 2310 -P 2443 -l /var/log/helloworld/helloworld.log"
PIDFILE="/var/run/helloworld.pid"

do_start() {
  if [ -f "$PIDFILE" ]; then
    echo "The helloworld service is already running."
    exit 1
  fi

  # Start the helloworld program
  $HELLOWORLD_BIN &
  # Store the process ID in the PID file
  echo $! > "$PIDFILE"
  echo "Started the helloworld service."
}

do_stop() {
  if [ ! -f "$PIDFILE" ]; then
    echo "The helloworld service is not running."
    exit 1
  fi

  # Read the process ID from the PID file
  PID=$(cat "$PIDFILE")
  # Terminate the helloworld program
  kill "$PID"
  # Remove the PID file
  rm "$PIDFILE"
  echo "Stopped the helloworld service."
}

do_restart() {
  do_stop
  sleep 1
  do_start
}

do_status() {
  if [ -f "$PIDFILE" ]; then
    PID=$(cat "$PIDFILE")
    if ps -p "$PID" > /dev/null; then
      echo "The helloworld service is running (PID: $PID)."
    else
      echo "The helloworld service is not running."
    fi
  else
    echo "The helloworld service is not running."
  fi
}

case "$1" in
  start)
    do_start
    ;;
  stop)
    do_stop
    ;;
  restart)
    do_restart
    ;;
  status)
    do_status
    ;;
  *)
    echo "Usage: $0 {start|stop|restart|status}"
    exit 1
    ;;
esac
```

```bash
sudo chown -R root:root /etc/init.d/helloworld \
  && sudo chmod +x /etc/init.d/helloworld
```

```bash
sudo nano /etc/systemd/system/helloworld.service
```

```bash
[Unit]
Description=hello service
After=network.target
StartLimitIntervalSec=0

[Service]
Type=forking
Restart=always
RestartSec=1
User=root
ExecStart=/usr/bin/helloworld -h 0.0.0.0 -p 2310 -P 2443 -l /var/log/helloworld/helloworld.log
RemainAfterExit=true

[Install]
WantedBy=multi-user.target
```

```bash
sudo chown -R root:root /etc/systemd/system/helloworld.service \
  && sudo systemctl daemon-reload
```

```bash
sudo systemctl start helloworld
```

Make the microservice a startup script:

```bash
sudo update-rc.d hello defaults
```

Create a script to uninstall the microservice:

```bash
nano uninstall.sh
```

```bash
#!/bin/sh
service helloworld stop \
  && update-rc.d -f helloworld remove \
  && rm -rf /etc/systemd/system/helloworld \
  && rm -rf /usr/bin/helloworld \
  && rm -rf /etc/init.d/helloworld \
  && systemctl daemon-reload
```

### Create a simple Nginx API gateway

```bash
sudo nano /etc/nginx/sites-available/myapp.conf
```

```
include /etc/nginx/sites-available/myapp_*_upstream.conf;
server {
  listen 80;
  server_name remote.host;
  include /etc/nginx/sites-available/myapp_*_location.conf;
}
```

```bash
sudo ln -s /etc/nginx/sites-available/myapp.conf /etc/nginx/sites-enabled/myapp.conf
```

```bash
sudo nano /etc/nginx/sites-available/myapp_helloworld_location.conf
```

```
location /api/helloworld {
  proxy_pass http://myapp-helloworld;
}
```

```bash
sudo nano /etc/nginx/sites-available/myapp_helloworld_upstream.conf
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

```bash
make debug && make run
```

Add the <code>remote.host</code> host to the <code>/etc/hosts</code> file and run the following command from an external host:

```bash
curl -i http://remote.host/api/helloworld
``` 



-->




<!--






#!/bin/bash

# # netsrv
# Lightweight microservices framework

# ### Install common development tools
# ```bash
# sudo apt install clang make curl git python autoconf libtool-bin libexpat1-dev cmake
# sudo apt install libssl-dev libmariadb-dev libpq-dev libsqlite3-dev unixodbc-dev
# ```

# ### Install GNUstep OBJ-C support
# ```bash
# sudo apt install gnustep-devel gobjc
# ln -s /usr/lib/gcc/x86_64-linux-gnu/10/include/objc /usr/local/include/objc
# ```

# ### Install apr-2 dependencies
# ```bash
# git clone https://github.com/apache/apr.git apr
# cd apr && ./buildconf
# ./configure --prefix=/tmp/apr-install --with-mysql --with-pgsql --with-sqlite3 --with-odbc
# make && make install
# mv /tmp/apr-install/include/apr-2 /path/to/netsrv/.tools/include
# mv /tmp/apr-install/lib /path/to/netsrv/.tools/lib
# rm -rf /tmp/apr-install
# ```

# ### Install json-c dependencies
# ```bash
# git clone https://github.com/json-c/json-c.git json-c
# mkdir json-c-build && cd json-c-build
# cmake ../json-c -DCMAKE_INSTALL_PREFIX=/tmp/json-c-install
# make && make install
# mv /tmp/json-c-install/include/json-c /path/to/netsrv/.tools/include
# mv /tmp/json-c-install/lib /path/to/netsrv/.tools/lib
# rm -rf /tmp/json-c-install
# ```

# ### Install Mongoose dependencies
# ```bash
# git clone https://github.com/cesanta/mongoose.git mongoose
# cp ./mongoose/mongoose.h /path/to/netsrv/.tools/include
# cp ./mongoose/mongoose.c /path/to/netsrv/.tools/src

# ```

# ### Nginx local API gateway

# #### /etc/nginx/sites-available/netsrv_gateway.conf
# ```bash
# include /etc/nginx/sites-available/ns_*_upstream.conf;
# server {
#   listen 80;
#   server_name example.local;
#   include /etc/nginx/sites-available/ns_*_location.conf;
#   location / {
#     root /var/www/html/ns-webapp;
#   }
# }
# ```
# ## Nginx local service config

# #### /etc/nginx/sites-available/helloworld_location.conf
# ```bash
# location /api/helloworld/ {
#   rewrite ^/api/helloworld(.*) /api$1 break;
#   proxy_pass https://ns-helloworld;
#   proxy_set_header SSL_CLIENT_CERT $ssl_client_cert;
#   proxy_ssl_certificate /path/to/client.crt;
#   proxy_ssl_certificate_key /path/to/client.key;
#   proxy_ssl_trusted_certificate /path/to/ca.crt;
#   proxy_ssl_verify on;
#   proxy_ssl_verify_depth 2;
#   proxy_ssl_name remote.host;
# }
# ```

# #### /etc/nginx/sites-available/helloworld_upstream.conf
# ```bash
# upstream ns-helloworld {
#   server remote.host:8081 fail_timeout=10s max_fails=3;
#   server remote.host:8082 fail_timeout=10s max_fails=3;
#   server remote.host:8083 fail_timeout=10s max_fails=3;
# }
# ```
# test:
# curl -k -c "/tmp/cookie.txt" -b "/tmp/cookie.txt" --key ".tools/certs/client.key" --cert ".tools/certs/client.crt" -i "https://localhost:2443/api/hello"


# #include "microdevtools.h"

# int HelloWorldController(ns_service_t *s) {
#   ns_http_response_hd_set(s->response, "Content-Type", "application/json");
#   const char *msg = ns_json_encode(s->pool, "Hello, World!", NS_JSON_T_STRING);
#   ns_printf(s, "{\"err\":%s,\"log\":%s,\"out\":%s}", "false", "null", msg);
#   return 200;
# }
# void ns_handler(ns_service_t *s) {
#   ns_route(s, "GET", "/api/hello", HelloWorldController);
# }

# ns_dbd_pool_t *dbd_pool;
# volatile sig_atomic_t server_run = 1;
# void ns_signal_exit(int signum) {
#   if (signum == SIGTERM || signum == SIGINT) {
#     server_run = 0;
#   }
# }

# int main(int argc, char **argv) {
#   struct state_t {
#     struct flag_t {
#       int input, init, pool, server, dbd, dbd_pool;
#     } flag;
#     int error;
#     ns_server_t *server;
#     char *er_msg;
#     apr_pool_t *pool;
#     struct mg_mgr mgr;
#     struct sigaction sig_action;
#   } st = {
#     .flag.input = 0, .flag.init = 0, .flag.pool = 0, .flag.server = 0,
#     .flag.dbd = 0, .flag.dbd_pool = 0, .error = 0, .server = NULL,
#     .er_msg = NULL
#   };
#   do {
#     ns_signal_handler(&(st.sig_action), ns_signal_exit);
#     st.flag.input = argv != NULL && argc > 1;
#     if ((st.error = !st.flag.input)) break;
#     {
#       apr_status_t rv;
#       rv = apr_initialize();
#       st.flag.init = rv == APR_SUCCESS;
#       if ((st.error = !st.flag.init)) break;
#       rv = apr_pool_create(&(st.pool), NULL);
#       st.flag.pool = rv == APR_SUCCESS;
#       if ((st.error = !st.flag.pool)) break;
#     }
#     st.server = ns_server_alloc(st.pool);
#     st.flag.server = st.server != NULL;
#     if ((st.error = !st.flag.server)) break;
#     st.flag.server = ns_server_init(
#       st.pool, &(st.server), argc, argv, &(st.er_msg));
#     if ((st.error = !st.flag.server)) break;
#     if (DEBUG) {
#       ns_log((st.server)->logger, "INFO", "Server starting...");
#       ns_log((st.server)->logger, "INFO", "Server initialized");
#     }
#     if ((st.server)->dbd_driver != NULL) {
#       if ((st.server)->dbd_conn_s != NULL) {
#         apr_status_t rv;
#         rv = apr_dbd_init(st.pool);
#         st.flag.dbd = rv == APR_SUCCESS;
#         if ((st.error = !st.flag.dbd)) break;
#       }
#     }
#     dbd_pool = NULL;
#     if (st.flag.dbd) {
#       st.flag.dbd_pool = ns_dbd_pool_alloc(st.pool);
#       if ((st.error = !st.flag.dbd_pool)) break;
#       if (DEBUG) {
#         ns_log((st.server)->logger, "INFO", "DBD connection pool allocated");
#       }
#       st.flag.dbd_pool = ns_dbd_pool_init(st.pool, (st.server)->dbd_driver,
#                                           (st.server)->dbd_conn_s);
#       if ((st.error = !st.flag.dbd_pool)) break;
#       if (DEBUG) {
#         ns_log((st.server)->logger, "INFO", "DBD connection pool initialized");
#       }
#     }
#     if (DAEMON) {
#       ns_daemonize();
#       if (DEBUG) {
#         ns_log((st.server)->logger, "INFO", "Service daemon");
#       }
#     }
#     if (MONGOOSE) {
#       mg_mgr_init(&(st.mgr));
#       if (DEBUG) {
#         ns_log((st.server)->logger, "INFO", "HTTP server initialized");
#       }
#       if (TLS) {
#         if ((st.server)->addr_s) {
#           printf("%s\n", (st.server)->addr);
#           printf("%s\n", (st.server)->addr_s);
          
#           mg_http_listen(&(st.mgr), (st.server)->addr,
#                          ns_http_request_handler, NULL);
#           mg_http_listen(&(st.mgr), (st.server)->addr_s,
#                          ns_http_request_handler, (void*)(st.server));
#         }
#       } else {
#         mg_http_listen(&(st.mgr), (st.server)->addr,
#                        ns_http_request_handler, (void*)(st.server));
#       }
#       if (DEBUG) {
#         ns_log((st.server)->logger, "INFO", "HTTP server listening...");
#       }
#       while (server_run) {
#         mg_mgr_poll(&(st.mgr), 1000);
#       }
#       mg_mgr_free(&(st.mgr));
#     }
#   } while (0);
#   if (st.error) {
#     if (!st.flag.input) {
#       ns_log((st.server)->logger, "ERROR", "Invalid input");
#     } else if (!st.flag.init) {
#       ns_log((st.server)->logger, "ERROR", "Environment initialization error");
#     } else if (!st.flag.pool) {
#       ns_log((st.server)->logger, "ERROR", "Memory pool allocation error");
#     } else if (!st.flag.server) {
#       if (st.server == NULL) {
#         ns_log((st.server)->logger, "ERROR", "Server allocation error");
#       } else if (st.er_msg != NULL) {
#         ns_log((st.server)->logger, "ERROR", st.er_msg);
#       } else {
#         ns_log((st.server)->logger, "ERROR", "Server initialization error");
#       } 
#     } else if (!st.flag.dbd) {
#       ns_log((st.server)->logger, "ERROR", "DBD initialization failure");
#     } else if (!st.flag.dbd_pool) {
#       ns_log((st.server)->logger, "ERROR", "DBD pool initialization failure");
#     } else {
#       ns_log((st.server)->logger, "ERROR", "General error");
#     }
#   }
#   if (st.flag.init) {
#     if (st.flag.pool) {
#       ns_dbd_pool_destroy();
#       ns_server_destroy(st.server);
#       apr_pool_destroy(st.pool);
#     }
#     apr_terminate();
#   }
#   return 0;
# }

name=""
lang="c"
port=""

# ------------------------------------------------------------------------------

function usage {
  echo "Usage: mdt [options]"
  echo "  -b, --build     Build MicroDevTools locally"
  echo "  -i, --install   Install MicroDevTools"
  echo "  -u, --uninstall Uninstall MicroDevTools"
  echo "  -c, --create    Create a new application"
  exit 0
}

# ------------------------------------------------------------------------------

function build {
  CURRDIR=$(pwd);
  if ! test -e "${CURRDIR}/.tools/.env"; then
    exit 0
  fi
  source .tools/.env
  if [ ! -d "${APR}" ]; then
    echo "${APR} does not exist."
    exit 0
  fi
  if [ ! -d "${JSONC}" ]; then
    echo "${JSONC} does not exist."
    exit 0
  fi
  if [ ! -d "${MONGOOSE}" ]; then
    echo "${MONGOOSE} does not exist."
    exit 0
  fi
  if [ ! -d "${MICRODEVTOOLS}" ]; then
    echo "${MICRODEVTOOLS} does not exist."
    exit 0
  fi
  rm -rf ${CURRDIR}/.tools/apr-2 && mkdir -p ${CURRDIR}/.tools/apr-2
  rm -rf ${CURRDIR}/.tools/json-c && mkdir -p ${CURRDIR}/.tools/json-c
  rm -rf ${CURRDIR}/.tools/mongoose && mkdir -p ${CURRDIR}/.tools/mongoose
  rm -rf ${CURRDIR}/.tools/microdevtools && mkdir -p ${CURRDIR}/.tools/microdevtools
  cd $APR && ./buildconf && ./configure --prefix=/tmp/apr-install \
    --with-mysql --with-pgsql --with-sqlite3 --with-odbc && make && \
    make install && mv /tmp/apr-install/include/apr-2 \
    ${CURRDIR}/.tools/apr-2/include && mv /tmp/apr-install/lib \
    ${CURRDIR}/.tools/apr-2 && rm -rf /tmp/apr-install
  cd $JSONC && cd .. && rm -rf json-c-build && mkdir json-c-build \
    && cd json-c-build && cmake ../$(basename "$JSONC") \
    -DCMAKE_INSTALL_PREFIX=/tmp/json-c-install && make && make install \
    && mv /tmp/json-c-install/include/json-c ${CURRDIR}/.tools/json-c/include \
    && mv /tmp/json-c-install/lib ${CURRDIR}/.tools/json-c/lib \
    && rm -rf /tmp/json-c-install
  cp $MONGOOSE/mongoose.* ${CURRDIR}/.tools/mongoose
  cp $MICRODEVTOOLS/microdevtools.* ${CURRDIR}/.tools/microdevtools
  cat <<EOF > ${CURRDIR}/.gitignore
.gitignore
.tools/apr-2
.tools/json-c
.tools/mongoose
.tools/microdevtools
EOF
  exit 0
}

# ------------------------------------------------------------------------------

# Example: $ sudo mdt -i
function install {
  CURRDIR=$(pwd)
  cp ${CURRDIR}/mdt /usr/local/bin/mdt
  exit 0
}

# ------------------------------------------------------------------------------

# Example: $ sudo mdt -u
function uninstall {
  CURRDIR=$(pwd);
  rm -rf /usr/local/bin/mdt
  exit 0
}

# ------------------------------------------------------------------------------

# Example: $ mkdir myapp && cd myappp && mdt -c
function create {
  CURRDIR=$(pwd)
  if [ -d ".tools" ]; then
    echo "Application already exists."
  fi
  mkdir ${CURRDIR}/.tools
  cat <<EOF > ${CURRDIR}/.tools/.env
APR=../apr
JSONC=../json-c
MONGOOSE=../mongoose
MICRODEVTOOLS=../microdevtools
EOF
  echo 
  echo "The application has been created."
  echo "Modify .tools/.env with the paths of APR, Json-c, and Mongoose"
  echo "then execute the command mdt -b."
  echo ""
  exit 0
}

# ------------------------------------------------------------------------------

function service {
  CURRDIR=$(pwd)
  if [ -d "${name}" ]; then
    echo "Service '${name}' already exists."
  fi
  if [[ -n $name ]]; then
    mkdir -p ${name}
    if test -e "${CURRDIR}/${name}/Makefile"; then
      exit 0;
    fi
    if [ "$lang" = "objc" ]; then
      if ! test -e "${CURRDIR}/${name}/main.m"; then
        cat <<EOF > ${CURRDIR}/${name}/main.m
#import "microdevtools.h"
int HelloWorldController(ns_service_t *s) {
  @autoreleasepool {
    ns_http_response_hd_set(s->response, "Content-Type", "application/json");
    const char *msg = ns_json_encode(s->pool, "Hello from ${name}!", NS_JSON_T_STRING);
    ns_printf(s, "{\"err\":%s,\"log\":%s,\"out\":%s}", "false", "null", msg);
    return 200;
  }
}
void ns_handler(ns_service_t *s) {
  ns_route(s, "GET", "/api/hello", HelloWorldController);
}
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
EOF
        cat <<EOF > ${CURRDIR}/${name}/Makefile
HOST:=0.0.0.0
PORT:=2310
SSL_PORT:=2443
LOG:=${name}.log
DBD_DRIVER:=mysql
DBD_CONN_S:="host=localhost,port=3306,user=test,pass=test,dbname=test"

CC:=clang
#-DMG_TLS=MG_TLS_OPENSSL -D_TLS -D_TLS_TWOWAY
CFLAGS:=-std=gnu99 -D_MONGOOSE -D_NATIVE_OBJC_EXCEPTIONS -fconstant-string-class=NSConstantString
INCLUDES:=-I. -I../.tools -I../.tools/apr-2/include -I../.tools/json-c/include -I../.tools/mongoose -I../.tools/microdevtools -I \`gnustep-config --variable=GNUSTEP_SYSTEM_HEADERS\`
LIBS:=-L../.tools/apr-2/lib -L../.tools/json-c/lib -L \`gnustep-config --variable=GNUSTEP_SYSTEM_LIBRARIES\`
LDFLAGS:=-lapr-2 -ljson-c -lssl -lcrypto -lgnustep-base -lobjc
SRC:=../.tools/mongoose/mongoose.c ../.tools/microdevtools/microdevtools.c main.m

all: debug

debug:
	\$(eval CFLAGS:=-g -D_DEBUG \$(CFLAGS))
	\$(CC) \$(CFLAGS) -o ${name} \$(SRC) \$(INCLUDES) \$(LIBS) \$(LDFLAGS)

release:
	\$(eval CFLAGS:=-D_DAEMON \$(CFLAGS))
	\$(CC) \$(CFLAGS) -o ${name} \$(SRC) \$(INCLUDES) \$(LIBS) \$(LDFLAGS)

run:
	LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:../.tools/apr-2/lib:../.tools/json-c/lib \\
	\$(VALGRIND) ./${name} -h \$(HOST) -p \$(PORT) -P \$(SSL_PORT) \\
	-l \$(LOG) -d \$(DBD_DRIVER) -D \$(DBD_CONN_S)

.PHONY: debug release run
EOF
      fi
    fi
    if [ "$lang" = "c" ]; then
      if ! test -e "${name}/${name}.c"; then
        cat <<EOF > ${CURRDIR}/${name}/main.c
#include "microdevtools.h"
int HelloWorldController(ns_service_t *s) {
  ns_http_response_hd_set(s->response, "Content-Type", "application/json");
  const char *msg = ns_json_encode(s->pool, "Hello from ${name}!", NS_JSON_T_STRING);
  ns_printf(s, "{\"err\":%s,\"log\":%s,\"out\":%s}", "false", "null", msg);
  return 200;
}
void ns_handler(ns_service_t *s) {
  ns_route(s, "GET", "/api/hello", HelloWorldController);
}
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
EOF
        cat <<EOF > ${CURRDIR}/${name}/Makefile
HOST:=0.0.0.0
PORT:=2310
SSL_PORT:=2443
LOG:=${name}.log
DBD_DRIVER:=mysql
DBD_CONN_S:="host=localhost,port=3306,user=test,pass=test,dbname=test"

CC:=clang
#-DMG_TLS=MG_TLS_OPENSSL -D_TLS -D_TLS_TWOWAY
CFLAGS:=-std=gnu99 -D_MONGOOSE
INCLUDES:=-I. -I.. -I../.tools/apr-2/include -I../.tools/json-c/include -I../.tools/mongoose -I../.tools/microdevtools
LIBS:=-L../.tools/apr-2/lib -L../.tools/json-c/lib
LDFLAGS:=-lapr-2 -ljson-c -lssl -lcrypto
SRC:=../.tools/mongoose/mongoose.c ../.tools/microdevtools/microdevtools.c main.c

all: debug

debug:
	\$(eval CFLAGS:=-g -D_DEBUG \$(CFLAGS))
	\$(CC) \$(CFLAGS) -o ${name} \$(SRC) \$(INCLUDES) \$(LIBS) \$(LDFLAGS)

release:
	\$(eval CFLAGS:=-D_DAEMON \$(CFLAGS))
	\$(CC) \$(CFLAGS) -o ${name} \$(SRC) \$(INCLUDES) \$(LIBS) \$(LDFLAGS)

run:
	LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:../.tools/apr-2/lib:../.tools/json-c/lib \\
	\$(VALGRIND) ./${name} -h \$(HOST) -p \$(PORT) -P \$(SSL_PORT) \\
	-l \$(LOG) -d \$(DBD_DRIVER) -D \$(DBD_CONN_S)

.PHONY: debug release run
EOF
      fi
    fi
  else
    usage
  fi
}

# ------------------------------------------------------------------------------

function cert {
  NAME=$1

  CURRDIR=$(pwd);
  mkdir -p .tools/cert

  if ! test -e ".tools/cert/ca_root.key"; then
    openssl genrsa -out .tools/cert/ca_root.key 4096
    openssl req -new -x509 -days 365 -key .tools/cert/ca_root.key -out .tools/cert/ca_root.crt -subj "/CN=NETSERV_ROOT_CA"
    echo ".tools/cert" >> .gitignore
    awk -i inplace '!seen[$0]++' .gitignore
  fi

  if ! test -e ".tools/cert/client.key"; then
    openssl genrsa -out .tools/cert/client.key 2048
    openssl req -new -key .tools/cert/client.key -out .tools/cert/client.csr -subj "/CN=client"
    openssl x509 -req -days 365 -in .tools/cert/client.csr -CA .tools/cert/ca_root.crt -CAkey .tools/cert/ca_root.key -set_serial 01 -out .tools/cert/client.crt
  fi

  if ! test -e ".tools/cert/${NAME}.key"; then
    openssl genrsa -out .tools/cert/${NAME}.key 2048
    openssl req -new -key .tools/cert/${NAME}.key -out .tools/cert/${NAME}.csr -subj "/CN=${NAME}"
    openssl x509 -req -days 365 -in .tools/cert/${NAME}.csr -CA .tools/cert/ca_root.crt -CAkey .tools/cert/ca_root.key -set_serial 01 -out .tools/cert/${NAME}.crt
  fi

  ca_crt_file=.tools/cert/ca_root.crt
  ca_c_variable_name=s_tls_ca
  server_crt_file=.tools/cert/${NAME}.crt
  server_crt_c_variable_name=s_tls_cert
  server_key_file=.tools/cert/${NAME}.key
  server_key_c_variable_name=s_tls_key

  # ca_crt
  ca_crt_variable="const char *${ca_c_variable_name} ="
  while IFS= read -r line; do
    ca_crt_variable="${ca_crt_variable}\n\"${line}\\\\n\""
  done < "$ca_crt_file"
  ca_crt_variable="${ca_crt_variable};"

  # server_crt
  server_crt_variable="const char *${server_crt_c_variable_name} ="
  while IFS= read -r line; do
    server_crt_variable="${server_crt_variable}\n\"${line}\\\\n\""
  done < "$server_crt_file"
  server_crt_variable="${server_crt_variable};"

  # server_key
  server_key_variable="const char *${server_key_c_variable_name} ="
  while IFS= read -r line; do
    server_key_variable="${server_key_variable}\n\"${line}\\\\n\""
  done < "$server_key_file"
  server_key_variable="${server_key_variable};"

  echo -e "#ifndef CERT_H" > ${CURRDIR}/api/${NAME}/certs.h
  echo -e "#define CERT_H\n" >> ${CURRDIR}/api/${NAME}/certs.h
  echo -e "#ifdef _TLS" >> ${CURRDIR}/api/${NAME}/certs.h
  echo -e "#ifdef _TLS_TWOWAY" >> ${CURRDIR}/api/${NAME}/certs.h
  echo -e "$ca_crt_variable" >> ${CURRDIR}/api/${NAME}/certs.h
  echo -e "#endif\n" >> ${CURRDIR}/api/${NAME}/certs.h
  echo -e "$server_crt_variable" >> ${CURRDIR}/api/${NAME}/certs.h
  echo -e "$server_key_variable" >> ${CURRDIR}/api/${NAME}/certs.h
  echo -e "#endif" >> ${CURRDIR}/api/${NAME}/certs.h
  echo -e "#endif /* CERT_H */" >> ${CURRDIR}/api/${NAME}/certs.h

  exit 0
}

# ------------------------------------------------------------------------------

function deb_make_instance {
  NAME=$1
  DIR=$2
  PORT=$3
  DBD=$4
  CONN_S=$5
  echo "systemctl enable ${NAME}_${PORT}.service" >> ${DIR}/DEBIAN/postinst
  echo "systemctl start ${NAME}_${PORT}.service" >> ${DIR}/DEBIAN/postinst
  echo "if systemctl is-active ${NAME}_${PORT}.service >/dev/null; then" >> ${DIR}/DEBIAN/prerm
  echo "systemctl stop ${NAME}_${PORT}.service" >> ${DIR}/DEBIAN/prerm
  echo "fi" >> ${DIR}/DEBIAN/prerm
    
  cat <<EOF > ${DIR}/etc/systemd/system/${NAME}_${PORT}.service
[Unit]
Description={NAME} service
After=network.target
StartLimitIntervalSec=0

[Service]
Type=forking
Restart=always
RestartSec=1
User=root
ExecStart=/usr/bin/{NAME} -h 0.0.0.0 -p {PORT} -l ./{NAME}.log -d {DBD} -D "{CONN_S}"
RemainAfterExit=true

[Install]
WantedBy=multi-user.target
EOF

  sed -i "s/{NAME}/${NAME}/g" ${DIR}/etc/systemd/system/${NAME}_${PORT}.service
  sed -i "s/{PORT}/${PORT}/g" ${DIR}/etc/systemd/system/${NAME}_${PORT}.service
  sed -i "s/{DBD}/${DBD}/g" ${DIR}/etc/systemd/system/${NAME}_${PORT}.service
  sed -i "s/{CONN_S}/${CONN_S}/g" ${DIR}/etc/systemd/system/${NAME}_${PORT}.service
}

function deb {

  if [ $# -eq 0 ]; then
    exit 0
  fi

  NAME=$1
  VERS=$2
  PORT_1=$3
  PORT_2=$4
  PORT_3=$5

  DIR=/tmp/${NAME}
  DESC="NetServ HTTP microservice for user management"
  MANT="Riccardo Vacirca<rvacirca23@gmail.com>"
  HOME="http:\/\/riccardovacirca.com"
  DBD="mysql"
  CONN_S="host=127.0.0.1,port=3306,user=test,pass=test,dbname=test"

  mkdir -p ${DIR}/DEBIAN
  mkdir -p ${DIR}/etc
  # mkdir -p ${DIR}/etc/nginx/sites-available
  mkdir -p ${DIR}/etc/systemd/system
  mkdir -p ${DIR}/usr/bin
  mkdir -p ${DIR}/usr/lib

  cat <<EOF > ${DIR}/DEBIAN/control
Source: {NAME}
Section: devel
Priority: optional
Maintainer: {MANT}
Standards-Version: {VERS}
Build-Depends: debhelper (>= 7)
Homepage: {HOME}
Package: {NAME}
Version: {VERS}
Essential: no
Architecture: amd64
Depends: libapr1 (>= 1.6.5), libaprutil1 (>= 1.6.1), libssl1.1 (>= 1.1.0)
Description: {DESC}
EOF

  cat <<EOF > ${DIR}/DEBIAN/postinst
#!/bin/bash
sudo ln -sf /etc/nginx/sites-available/ns_gateway.conf /etc/nginx/sites-enabled/ns_gateway.conf
chown -R root:root /usr/bin/{NAME}
systemctl daemon-reload
EOF

  cat <<EOF > ${DIR}/DEBIAN/postrm
#!/bin/bash
systemctl daemon-reload
EOF

  cat <<EOF > ${DIR}/DEBIAN/prerm
#!/bin/bash
set -e
EOF

  sed -i "s/{NAME}/${NAME}/g" ${DIR}/DEBIAN/control
  sed -i "s/{VERS}/${VERS}/g" ${DIR}/DEBIAN/control
  sed -i "s/{DESC}/${DESC}/g" ${DIR}/DEBIAN/control
  sed -i "s/{MANT}/${MANT}/g" ${DIR}/DEBIAN/control
  sed -i "s/{HOME}/${HOME}/g" ${DIR}/DEBIAN/control

  sed -i "s/{NAME}/${NAME}/g" ${DIR}/DEBIAN/postinst
  chmod +x ${DIR}/DEBIAN/postinst
  chmod +x ${DIR}/DEBIAN/prerm
  chmod +x ${DIR}/DEBIAN/postrm

  cp .tools/builds/${NAME} ${DIR}/usr/bin/${NAME}

  deb_make_instance "${NAME}" "${DIR}" "${PORT_1}" "${DBD}" "${CONN_S}"
  deb_make_instance "${NAME}" "${DIR}" "${PORT_2}" "${DBD}" "${CONN_S}"
  deb_make_instance "${NAME}" "${DIR}" "${PORT_3}" "${DBD}" "${CONN_S}"

  mkdir -p .tools/dist
  dpkg-deb --build ${DIR} .tools/dist/${NAME}-${VERS}_amd64.deb
  #  rm -rf /tmp/${NAME}
  echo "done."
  echo
}

# ------------------------------------------------------------------------------

function gateway {
  CURRDIR=$(pwd);
  cat <<EOF > /etc/nginx/sites-available/netsrv_gateway.conf
include ${CURRDIR}/api/ns_*_upstream.conf;
server {
  listen 80;
  server_name local.host;
  include ${CURRDIR}/api/ns_*_location.conf;
  location / {
    root /var/www/html;
  }
}
EOF
  ln -s /etc/nginx/sites-available/netsrv_gateway.conf /etc/nginx/sites-enabled
  echo "127.0.0.1 local.host" >> /etc/hosts
  echo "127.0.0.1 remote.host" >> /etc/hosts
}

# ------------------------------------------------------------------------------

function client {
  NAME=$1
  DATA=$2
  CURRDIR=$(pwd);
  echo "curl -k -c \"/tmp/cookie.txt\" -b \"/tmp/cookie.txt\" \
    --key \".tools/cert/client.key\" --cert \".tools/cert/client.crt\" \
    -i \"http://local.host/api/${NAME}\""
  echo ""
  curl -k -c "/tmp/cookie.txt" -b "/tmp/cookie.txt" \
    --key ".tools/cert/client.key" --cert ".tools/cert/client.crt" \
    -i "http://local.host/api/${NAME}"
}

# ------------------------------------------------------------------------------

function main {
  local cmd
  while [[ $# -gt 0 ]]; do
    case "$1" in
      -c|--create)
        cmd="create"
        ;;
      -b|--build)
        cmd="build"
        ;;
      -i|--install)
        cmd="install"
        ;;
      -u|--uninstall)
        cmd="uninstall"
        ;;
      -s|--service)
        cmd="service"
        ;;
      -n|--name)
        name="$2"
        shift
        ;;
      -l|--lang)
        lang="$2"
        shift
        ;;
      -p|--port)
        port="$2"
        shift
        ;;
      *)
        usage
        ;;
    esac
    shift
  done
  if [[ -n $cmd ]]; then
    $cmd
  else
    usage
  fi
}

main "$@"




-------------------------------------------------------
#include "microdevtools.h"

#define SECRET_KEY "my_secret_key"

int SignInController(mdt_service_t *s) {

  struct state_t {
    struct flag_t {
      int ok_request, ok_rset, ok_user, ok_token, ok_cookies;
    } flag;
    int error;
  } st = {
    .flag.ok_request = 0, .flag.ok_user = 0, .flag.ok_token = 0,
    .flag.ok_cookies = 0, .error = 1
  };

  do {
    apr_table_t *user, *valid_req;
    const char *id, *username, *token, *cookies;
    const char sql[] = "select id, username from users where username=%s and password=%s";
    apr_array_header_t *rset;
    mdt_request_validator_t v[2] = {
      {"username", MDT_REQUEST_T_STRING, MDT_REQUEST_F_NONE},
      {"password", MDT_REQUEST_T_STRING, MDT_REQUEST_F_MD5},
    };

    valid_req = mdt_http_request_validate_args(s->request, v, 2);
    st.flag.ok_request = mdt_table_nelts(valid_req) == 2;
    if (!st.flag.ok_request) break;

    rset = mdt_dbd_prepared_select(s->pool, s->dbd, sql, valid_req);
    st.flag.ok_rset = s->dbd->err <= 0;
    if (!st.flag.ok_rset) break;

    st.flag.ok_user = rset != NULL && rset->nelts > 0;
    if (!st.flag.ok_user) break;
    user = mdt_dbd_record(rset, 0);
    st.flag.ok_user = user != NULL;
    if (!st.flag.ok_user) break;

    id = apr_table_get(user, "id");
    username = apr_table_get(user, "username");
    mdt_printf(s, "%s %s\n", id, username);

    token = mdt_jwt_token_create(s->pool, user, SECRET_KEY);
    st.flag.ok_token = token != NULL;
    if (!st.flag.ok_token) break;

    cookies = apr_psprintf(s->pool, "access_token=%s Path=/", (const char*)token);
    st.flag.ok_cookies = cookies != NULL;
    if (!st.flag.ok_cookies) break;
    mdt_http_response_hd_set(s->response, "Set-Cookie", (const char*)cookies);

    st.error = 0;
  } while (0);
  
  if (st.error) {
    if (!st.flag.ok_request) {
      mdt_printf(s, "%s\n", "Invalid request.");
    } else if (!st.flag.ok_rset) {
      mdt_printf(s, "%s %d\n", s->dbd->er_msg != NULL ? s->dbd->er_msg : "DBD error.", s->dbd->err);
    } else if (!st.flag.ok_user) {
      mdt_printf(s, "%s\n", "User not found.");
    } else if (!st.flag.ok_token) {
      mdt_printf(s, "%s\n", "JWT token error.");
    } else if (!st.flag.ok_cookies) {
      mdt_printf(s, "%s\n", "Cookies error.");
    } else {
      mdt_printf(s, "%s\n", "General error.");
    }
  }

  return 200;
}

int HelloController(mdt_service_t *s) {
  mdt_http_response_hd_set(s->response, "Content-Type", "text/plain");
  const char *msg = apr_pstrdup(s->pool, "Hello, World!");
  mdt_printf(s, "%s\n", msg);
  return 200;
}

void mdt_handler(mdt_service_t *s) {
  mdt_route(s, "GET", "/api/hello", HelloController);
  mdt_route(s, "POST", "/api/signin", SignInController);
}



-->