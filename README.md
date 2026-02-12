# BARC

A dead-simple, Burrows-Wheeler Transform + Move-To-Front Transform + Run-Length Encoding file-compressor archive tool written in C. This project is written in pure C99, no external libraries and no OS-specific/POSIX libraries were used.

## BUILD INSTRUCTIONS + USAGE

Clone this repository and `cd` into it:

```
   %  git clone https://github.com/vs-123/barc.git
   %  cd barc
```

Create a build directory, and use `cmake` to build it:

```
   %  mkdir -p build/
   %  cd build
   %  cmake ..
   %  cmake --build .
```

You may now use the compiled binary:

```
   %  ./barc
   === BARC ===
   [USAGE] ./barc [OPTION]
   
   [OPTIONS]
      -c, --compress <ARCHIVE> <FILE> [<FILE>]...   CREATE ARCHIVE <ARCHIVE> FROM FILE(S)
      -x, --extract <ARCHIVE>                       EXTRACT FILE(S) FROM <ARCHIVE>
      -h, -?, --help                                PRINT THIS HELP MESSAGE AND EXIT
      -i, --info                                    VIEW INFORMATION ABOUT THIS PROGRAM
   
   [EXAMPLE]
      ./barc -c archive.barc file1.txt file2.txt
      ./barc -x archive.barc
   
   [NOTE] FLAGS ARE CASE-INSENSITIVE
```

## FEATURES

- **BURROWS-WHEELER TRANSFORM IMPLEMENTATION** -- Utilises BWT to rearrange input into runs of similar characters. Uses basic `qsort` for cyclic shift

- **LOCAL ADAPTIVE MOVE-TO-FRONT STAGE** -- Performs standard MTF transformation after BWT to convert local character frequency into small integer values, prepares stream for redundancy reduction

- **CHUNKED RUN-LENGTH ENCODING** -- Custom RLE layer is applied MTF output, handles runs longer than 255 bytes by splitting them into chunks, keeps logic simple

- **FILE ARCHIVING** -- Supports ability to group multiple files into a single barc container. Includes basic metadata to facilitate restoration

- **DETERMINISTIC SINGLE-PASS EXTRACTION** -- Reverses transform pipeline using an inverse BWT algorithm. Restores file with a `BARC_EXT_` prefix

- **CASE-INSENSITIVE CLI** -- Basic command-line interface for compression and extraction, ignores flag casing for user convenience

- **PORTABLE STANDARD C** -- Written in pure C99 using only the standard library. No OS-specific/POSIX/external libraries were used to keep this portable

- **<500 SLOC** -- This project was written in less than 500 lines of code, including blank lines

## LIMITATIONS

- **QSORT OVER SA-IS** -- Standard `qsort` was used on a suffix-pointer array instead of SA-IS because implementing SA-IS would triple the SLOC and would definitely not keep this dead-simple. It hits `O(n^2 log n)` in the worst case, and that happens more frequently on repetition-heavy data because qsort struggles with them

- **HOT-LOOP OVERHEAD** -- `bwtcmp` function performs modular arithmetic inside the hot loop. I agree that this potentially inhibits compiler optimisations like SIMD auto-vectorisation and also increases branch misprediction overhead, but I used this approach anyways to keep it simple (C's speed is a real advantage here). A more performant approach according to me would involve string doubling or padding

## LICENSE

This project is licensed under the GNU Affero General Public License version 3.0 or later.

**NO WARRANTY PROVIDED**

For full terms, see `LICENSE` file or visit **https://www.gnu.org/licenses/agpl-3.0.en.html**.
