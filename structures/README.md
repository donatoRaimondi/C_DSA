# Data structures

Each data structure lives in its own directory together with its header,
implementation, tests, documentation, and build rules.

## Directory layout

```text
structures/
├── Makefile
├── README.md
├── list/
│   ├── Makefile
│   ├── README.md
│   ├── list.h
│   ├── list.c
│   └── test_list.c
├── ordered_list/
│   ├── Makefile
│   ├── README.md
│   ├── ordered_list.h
│   ├── ordered_list.c
│   └── test_ordered_list.c
├── stack/
│   ├── Makefile
│   ├── README.md
│   ├── stack.h
│   ├── stack.c
│   └── test_stack.c
├── queue/
│   ├── Makefile
│   ├── README.md
│   ├── queue.h
│   ├── queue.c
│   └── test_queue.c
├── set/
│   ├── Makefile
│   ├── README.md
│   ├── set.h
│   ├── set.c
│   └── test_set.c
├── dictionary/
    ├── Makefile
    ├── README.md
    ├── dictionary.h
    ├── dictionary.c
│   └── test_dictionary.c
└── b_tree/
    ├── Makefile
    ├── README.md
    ├── b_tree.h
    ├── b_tree.c
    └── test_b_tree.c
```

## Run every structure's tests

From the `structures` directory:

```sh
make test
make sanitize
```

Remove all generated test executables with:

```sh
make clean
```

You can also work with one structure independently:

```sh
cd list
make test
make sanitize
```

See [`list/README.md`](list/README.md),
[`ordered_list/README.md`](ordered_list/README.md),
[`stack/README.md`](stack/README.md), [`queue/README.md`](queue/README.md),
[`set/README.md`](set/README.md), and
[`dictionary/README.md`](dictionary/README.md), and
[`b_tree/README.md`](b_tree/README.md) for module-specific API and testing
instructions.

## Add another structure

Create a directory containing that structure's files and a Makefile with
`test`, `sanitize`, and `clean` targets. For example:

```text
structures/
├── list/
├── queue/
└── stack/
```

Then add its directory name to `STRUCTURES` in the root Makefile:

```make
STRUCTURES := list queue stack
```

After that, the root `make test`, `make sanitize`, and `make clean` commands
will include the new structure automatically.
