# Bash Scripts

## How the shell parses strings

### About quote

| Quoting Style | Variable Expansion | Word Splitting | Pathname Expansion(Globbing) |
|---------------|--------------------|----------------|------------------------------|
| no quote      | ✅                 | ✅             | ✅                           |
| double quote  | ✅                 | ❌             | ❌                           |
| single quote  | ❌                 | ❌             | ❌                           |


### Unquoted: The most dangerous form

```bash
n="hello world"
printf '%s\n' $n
```

Many beginners think this becomes `printf '%s\n' "hello world"`, but it actually becomes
`printf '%s\n' hello world`. This happens because the shell performs variable expansion
followed by word splitting.

``` bash
x="*"
printf '%s\n' $x
```

If the current directory contains:
```text
a.txt
b.txt
main.c
```
then `*` is expanded by pathname expansion (globbing), so the command becomes:

```bash
printf '%s\n' a.txt b.txt main.c
```

### Double quote: Preserve the value as one argument

```bash
x="hello world"
printf '%s\n' "$x"
```

Variable expansion still happens, but no word splitting and no globbing, so the output is `hello world`
Likewise:
```bash
x="*"
printf '%s\n' "$x"
```
Output is `*`

### Single quote: completely literal
```bash
x="hello world"
printf '%s\n' '$x'
```
Output is `$x`
