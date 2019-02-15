# acohttp
toy http static file server based on the awesome coroutine library [libaco](https://github.com/hnes/libaco), some code borrowed from [zaver](https://github.com/zyearn/zaver). 

## current status
- [x] serve file with sendfile
- [ ] persistent connection
- [ ] parse headers
- [ ] prevent buffer overflow

## Compile
on a linux machine with gcc installed, run:

```
make

```

## Usage

```
./acohttp -p 3000
```

```
curl localhost:3000/filename
```
