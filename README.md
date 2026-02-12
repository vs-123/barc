# BARC

A dead-simple, BWT + MTF + RLE file-compressor archive tool in C.

## FEATURES

- **BURROWS-WHEELER TRANSFORM IMPLEMENTATION** -- Utilises BWT to rearrange input into runs of similar characters. Uses basic `qsort` for cyclic shift

- **MOVE-TO-FRONT STAGE** -- Performs standard MTF transformation after BWT to convert local character frequency into small integer values, prepares stream for redundancy reduction

- **RUN-LENGTH ENCODING** -- Custom RLE layer is applied MTF output, handles runs longer than 255 bytes by splitting them into chunks, keeps logic simple

- **FILE ARCHIVING** -- Supports ability to group multiple files into a single barc container. Includes basic metadata to facilitate restoration

- **SINGLE-PASS EXTRACTION** -- Reverses transform pipeline using an inverse BWT algorithm. Restores file with a `BARC_EXT_` prefix

- **CASE-INSENSITIVE CLI** -- Basic command-line interface for compression and extraction, ignores flag casing for user convenience

- **STANDARD C** -- Written in pure C99 using only the standard library. No OS-specific/POSIX/external libraries were used to keep this portable

- **<500 SLOC** -- This project was written in less than 500 lines of code including blank newlines

## LIMITATIONS

- **QSORT OVER SA-IS** -- Standard `qsort` was used on a suffix-pointer array instead of SA-IS because implementing SA-IS would triple the SLOC and would definitely not keep this dead-simple. It hits `O(n^2 log n)` in the worst case, and that happens more frequently on repetition-heavy data because qsort struggles with them

- **HOT-LOOP OVERHEAD** -- `bwtcmp` function performs modular arithmetic inside the hot loop. I agree that this potentially inhibits compiler optimisations like SIMD auto-vectorisation and also increases branch misprediction overhead, but I used this approach anyways to keep it simple (C's speed is a real advantage here). A more performant approach according to me would involve string doubling or padding

## LICENSE

This project is licensed under the GNU Affero General Public License version 3.0 or later.

**NO WARRANTY PROVIDED**

For full terms, see `LICENSE` file or visit **https://www.gnu.org/licenses/agpl-3.0.en.html**.
