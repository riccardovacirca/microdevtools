# microdevtools
Microservices Developer Tools

## System dependencies
```bash
apt install clang make curl git python autoconf libtool-bin libexpat1-dev \
            cmake libssl-dev libmariadb-dev libpq-dev libsqlite3-dev \
            unixodbc-dev
```

## Local dependencies
APR (Apache Portable Runtime) (https://github.com/apache/apr.git)<br>
Json-c (https://github.com/json-c/json-c.git)<br>
Mongoose (https://github.com/cesanta/mongoose.git)

## Install
Clone the required dependencies and edit the file <code>.tools/.env</code> to
set the paths relative to each dependency, then execute
```bash
make
sudo make install
```

## Create a new microservice
```bash
mdt --service --name my_service --lang c
```
