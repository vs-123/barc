# BARC

A dead-simple, BWT + MTF + RLE file-compressor archive tool in C.

## FEATURES

- **BURROWS-WHEELER TRANSFORM IMPLEMENTATION** -- Utilises BWT to rearrange input into runs of similar characters. Uses basic `qsort` for cyclic shift

- **MOVE-TO-FRONT STAGE** -- Performs standard MTF transformation after BWT to convert local character frequency into small integer values, prepares stream for redundancy reduction

- **RUN-LENGTH ENCODING** -- Custom RLE layer is applied MTF output, handles runs longer than 255 bytes by splitting them into chunks, keeps logic simple

- **FILE ARCHIVING** -- Supports ability to group multiple files into a single barc container. Includes basic metadata to facilitate restoration

- **SINGLE-PASS EXTRACTION** -- Reverses transform pipeline using an inverse BWT algorithm. Restores file with a `BARC_EXT_` prefix

- **CASE-INSENSITIVE CLI** -- Basic command-line interface for compression and extraction, ignores flag casing for user convenience

- **STANDARD C** -- Written in pure C99 using only the standard library. No OS-specific/POSIX/external libraries were used.

## LICENSE

This project is licensed under the GNU Affero General Public License version 3.0 or later.

**NO WARRANTY PROVIDED**

For full terms, see `LICENSE` file or visit **https://www.gnu.org/licenses/agpl-3.0.en.html**.
