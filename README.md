# Parallel algorithms

## Установка зависимостей
* `sudo apt update`
* `sudo apt -y install cmake`
* `sudo apt install g++`

## Установка OpenCilk

* Выполнять из родительской директории (`/home/ubuntu/`)
* `git clone -b opencilk/v1.0 https://github.com/OpenCilk/infrastructure`
* `infrastructure/tools/get $(pwd)/opencilk`
* `infrastructure/tools/build $(pwd)/opencilk $(pwd)/build`
* `git clone git@github.com:KokorinIlya/parallel-algorithms.git`
* `cd $(pwd)/build`
* `cmake -DCMAKE_INSTALL_PREFIX=/home/ubuntu/parallel-algorithms/cilk -P cmake_install.cmake`

## Установка PCTL
* Выполнять из родительской директории (`/home/ubuntu`)
* `wget https://raw.githubusercontent.com/deepsea-inria/pctl/master/script/get.sh`
* `chmod +x ./get.sh`
* `./get.sh $(pwd)/parallel-algorithms/cilk`

## Установка Google Tests

* Выполнять в директории проекта (`/home/ubuntu/parallel-algorithms/cilk/`)
* `git clone https://github.com/google/googletest.git tests/lib`

## Компиляция с использованием CMake

* Выполнять в директории проекта (`/home/ubuntu/parallel-algorithms/cilk/`)
* `CC=bin/clang CXX=bin/clang++ cmake .`
* `make`

## Запуск тестов

* Выполнять в директории проекта (`/home/ubuntu/parallel-algorithms/cilk/`)
* `./tests/run_tests.out`

## Конфигурация запуска
* `CILK_NWORKERS=N ./<bench_name>.out`
* Пример: `CILK_NWORKERS=16 ./bench_sort.out`
