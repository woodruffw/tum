tum
===

A **t**iny **u**seless **m**achine.

*In progress.*

## Background

I deal with assembly on a daily basis, but I've never tried to develop my own ISA before (the
closest I've come was a [busy beaver](https://en.wikipedia.org/wiki/Busy_beaver)
[implementation](https://github.com/woodruffw/busybeaver) as a college freshman).

This repo is just a place for me to learn by doing. My implementations will inevitably be full
of inefficiencies and design flaws, but hopefully they'll get better over time.

## Design

See [DESIGN.md](DESIGN.md) for architecture details.

## Building

tum is written in C99-compatible C and runs on POSIX systems.

To build it, just run the Makefile:

```sh
$ make
```

## Usage

tum has two components: an assembler (`tasm`) and the machine itself (`tmachine`).

**Warning**: The assembler is extremely primitive -- it does just enough lifting to translate
basic operations into the binary format used by `tmachine`. See the [example/](example/) directory
for some of the things you can feed it.

Both communicate via `stdin` and `stdout`, so usage looks something like this:

```sh
$ tasm < program.s > program.bin
$ tmachine < program.bin
```

or more briefly and without an intermediate file:

```sh
$ tasm < program.s | tmachine
```
