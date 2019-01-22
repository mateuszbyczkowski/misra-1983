ping-pong algorithm
=========
Ping-Pong algorithm (Misra 1983).


Compile and run:
-----------------------
```bash
mpic++ header.h main.cpp -std=c++11 -pthread
mpirun -n 5 ./a.out
```

Message lost simulation turns on press 'p' for ping and 'k' for pong.
You can be only in one lost message mode, ping or pong.