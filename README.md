# MicroDevTools

Microservices DevTools

#### Table of Contents

[Install system dependencies](#install-system-dependencies)<br>
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

### Get the latest version of JSON-c (optional)

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

### Create a new microservices-based project
#### Project structure

<pre>mongoose/
microdevtools/
helloworld/
  Makefile
  main.c
</pre>

```bash
mkdir -p helloworld
```

### Create a HelloWorld microservice in C

```bash
nano helloworld/helloworld.c
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
```

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
```

### Compile and run the HelloWorld microservice (debug version)

```bash
cd helloworld && make debug && make run
```

<code>TEST HTTP</code>

```bash
curl -i "http://localhost:2310/api/helloworld"
```

### Connect to a PostgreSQL database

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
