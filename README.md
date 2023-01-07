# wdsp
WDSP by Warren Pratt, NR0V
DSP Library originally written for Windows
Ported to Linux and Android by John Melton g0orx/n6lyt

# Additions by Ram VU2JXN

Two new noise reduction algorithms have been added.

1. RNNoise based noise reduction.
2. libspecbleach based noise reduction.

In order to use it, you need to install forks of these two libraries:

1. [rnnoise](https://github.com/vu3rdd/rnnoise)

The buffering branch has some changes to make it work with wdsp's frame size.

2. [libspecbleach](https://github.com/lucianodato/libspecbleach)

Follow the instructions on these above libraries to build and install it and then build the wdsp.

