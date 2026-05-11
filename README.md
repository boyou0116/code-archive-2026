# code-archive-2026

A collection of C programming exercises and examples.

## cjson-examples

Six progressively complex examples using the [cJSON](https://github.com/DaveGamble/cJSON) library to parse JSON files in C.

| Example | JSON structure    | What it demonstrates                                                       |
|---------|-------------------|----------------------------------------------------------------------------|
| 1       | Array of objects  | Parse an array and extract string fields (`firstName`, `lastName`)         |
| 2       | Flat object       | Read a string, number, and boolean from a top-level object                 |
| 3       | Array of numbers  | Iterate over a numeric array (`ports`)                                     |
| 4       | Array of objects  | Parse objects with mixed types (string + number)                           |
| 5       | Nested object     | Access fields inside a nested `network` object                             |
| 6       | Mixed (all types) | Combine all the above: scalar fields, a numeric array, and a nested object |

### Building

Each example must be compiled from its own directory so it can find its `.json` file at runtime.

```sh
cd cjson-examples
gcc 1.c -o 1 -lcjson && ./1
gcc 2.c -o 2 -lcjson && ./2
# etc.
```

**Dependency:** `libcjson-dev` (e.g. `sudo apt install libcjson-dev` on Debian/Ubuntu).

## kr-exercises

Exercises from *The C Programming Language* by Kernighan & Ritchie.

| File           | Description                                                                                                          |
|----------------|----------------------------------------------------------------------------------------------------------------------|
| `word-count.c` | Counts lines, words, and characters from stdin (based on K&R §1.5). Also prints each word truncated to 7 characters. |

### Building

```sh
gcc kr-exercises/word-count.c -o word-count
echo "hello world" | ./word-count
```
