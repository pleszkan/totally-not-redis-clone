# Roadmap: kvstore alongside *A Tour of C++* (3rd ed.)

Project: an in-memory key-value store with a command-line interface (a mini Redis).
Commands grow over time: `SET`, `GET`, `DEL`, `INCR`, `KEYS`, `EXPIRE`, `TTL`, ...

Rule: after each chapter, build only what that chapter makes possible. Later-chapter types
(`std::string`, `std::vector`, `std::unordered_map`) are fine to use as black boxes early on;
the book uses them from Chapter 1 too.

Build flags for every stage:
`clang++ -std=c++20 -Wall -Wextra -fsanitize=address,undefined -g`

---

## Part 1: Language core

### Ch 1 The Basics
**You can now:** write functions, use types/constants/references, branch and loop.
**Build:** a REPL in one `main.cpp`. Read a line, split it into words, handle `SET k v`,
`GET k`, `DEL k`, `EXIT`. Store data in an `std::unordered_map<std::string, std::string>`
(black box). Print `OK` / `(nil)`.
**Practice:** pass strings by `const&`; use `const` and `constexpr` wherever possible.

### Ch 2 User-Defined Types
**You can now:** model data with `struct`, `class`, `enum class`.
**Build:** `enum class CommandType { Set, Get, Del, Exit, Unknown }` and
`struct Command { CommandType type; std::string key, value; }`. Split the code into
`parse(line) -> Command` and `execute(Command, map)`.
**Skip for now:** unions. You'll use `std::variant` in Ch 15 instead.

### Ch 3 Modularity
**You can now:** split code into headers and source files, use namespaces, choose how to pass and return values.
**Build:** the project layout:
```
src/  main.cpp  command.h/.cpp  store.h/.cpp
Makefile
```
Put everything in `namespace kv`. Return `Command` by value. Use structured bindings
(`auto [ok, msg] = ...`). Add a Makefile with the sanitizer flags.

### Ch 4 Error Handling
**You can now:** throw and catch exceptions, state invariants, use `assert` / `static_assert`.
**Build:** parse errors (`wrong number of arguments`, `unknown command`) as a `kv::ParseError`
exception, caught in the REPL, which prints `ERR ...`. Add `assert` for invariants such as
"key is non-empty". Compare this with returning an error result, and write down which you prefer.

### Ch 5 Classes (you are here)
**You can now:** write concrete types, abstract interfaces and class hierarchies.
**Build:**
- Turn the loose map into `class Store` (private data, `set/get/del`, `const` getters).
- Add an abstract `Storage` interface (`virtual ~Storage() = default; virtual ... = 0`)
  with a `MemoryStorage` implementation, so the backend can change later.
- Optional: make each command a class (`SetCommand`, `GetCommand`) with a virtual
  `execute()`. Hold them as `std::unique_ptr<Command>`.

### Ch 6 Essential Operations
**You can now:** control construction, copying and moving, and use RAII.
**Build:**
- `class LogFile`, an RAII wrapper that opens a file in its constructor and closes it in its
  destructor. Make it move-only (delete the copy operations, define the move operations).
- `operator<<` for `Command` (for debugging output).
- **Side exercise:** write your own `Vector<T>` or `UniquePtr<T>` with the rule of five, and test
  it under AddressSanitizer.

---

## Part 2: Generic programming

### Ch 7 Templates
**You can now:** write class and function templates.
**Build:** a generic `Cache<K, V>` (for example a fixed-capacity cache) and use it in the store.
Write a function template `parse_as<T>(std::string_view)` for converting arguments.

### Ch 8 Concepts and Generic Programming
**You can now:** constrain templates with concepts; write variadic templates.
**Build:** constrain `Cache<K, V>` (`K` must be hashable and equality-comparable). Write a variadic
`log(args...)` helper that prints any number of arguments.

---

## Part 3: The standard library

### Ch 9 Library Overview
**Build:** nothing. Read it, then decide between `#include` and `import std;` (the appendix
explains the module option; headers are the safe default on macOS clang).

### Ch 10 Strings and Regular Expressions
**You can now:** use `std::string`, `std::string_view`, `std::regex`.
**Build:** replace the hand-rolled splitting with a proper tokenizer that returns
`std::vector<std::string_view>`. Support quoted values (`SET name "John Smith"`). Add
`KEYS <pattern>` using `std::regex`.

### Ch 11 Input and Output
**You can now:** use streams, formatting, file I/O and `std::filesystem`.
**Build:** persistence.
- `SAVE` writes all entries to a file; startup loads it.
- Then switch to an append-only log: every write command is appended to the file and replayed on startup.
- Use `std::filesystem` for the data directory and file existence checks.
- Define `operator>>` / `operator<<` for the on-disk record format.

### Ch 12 Containers
**You can now:** choose between `vector`, `list`, `map`, `unordered_map`.
**Build:** keep `unordered_map` for lookups, and add a `std::map` index so `KEYS` returns sorted
results and `SCAN prefix` works as a range query (`lower_bound`). Use a `std::list` plus map
for an LRU eviction policy in `Cache`.

### Ch 13 Algorithms
**You can now:** use iterators and standard algorithms with lambdas.
**Build:** implement `KEYS`, `COUNT`, `DELPREFIX` and `FLUSHEXPIRED` with `std::find_if`, `std::copy_if`,
`std::count_if`, `std::erase_if`, `std::sort`. No hand-written loops where an algorithm exists.

### Ch 14 Ranges
**You can now:** build lazy pipelines with views.
**Build:** rewrite `KEYS` / `SCAN` as pipelines (`| filter | transform | take`), for example
`SCAN prefix LIMIT n`. Compare the readability with the Ch 13 version.

### Ch 15 Pointers and Containers
**You can now:** use `unique_ptr`, `shared_ptr`, `span`, `optional`, `variant`, `any`.
**Build:**
- Values become `std::variant<std::string, std::int64_t, double>`; add `TYPE key`, `INCR`, `APPEND`.
- `get` returns `std::optional<Value>` instead of a sentinel.
- Use `std::visit` for printing and arithmetic.
- Decide ownership explicitly: who owns the `Storage`? (`unique_ptr` in `Store`.)

### Ch 16 Utilities
**You can now:** use `chrono`, `source_location`, `move`/`forward`, type functions.
**Build:**
- Key expiry: `EXPIRE key seconds`, `TTL key` with `std::chrono::steady_clock`; expired keys are
  treated as missing on access.
- A logger that records `std::source_location`.
- Review your code for needless copies and use `std::move` where appropriate.

### Ch 17 Numerics
**You can now:** use math functions, random numbers, `numeric_limits`.
**Build:**
- Overflow-checked `INCR` / `DECR` using `std::numeric_limits`.
- A random-data generator (`<random>`) for stress tests and a small benchmark of millions of operations.

### Ch 18 Concurrency
**You can now:** use threads, mutexes, atomics, futures and coroutines.
**Build:**
- Make `Store` thread-safe with `std::shared_mutex` (many readers, one writer).
- A background thread that purges expired keys (`std::jthread` + stop token).
- Run the same stress test from Ch 17 from several threads, with ThreadSanitizer
  (`-fsanitize=thread`; it can't be combined with address sanitizer in the same build).
- A TCP front-end so a client like `nc localhost 6380` can talk to the store. The book doesn't cover
  sockets, so this part uses POSIX sockets or the Asio library.

### Ch 19 History and Compatibility
**Build:** nothing. Read the C/C++ compatibility section before you touch C libraries or raw sockets.

---

## Checkpoints
After finishing each chapter's stage, commit (or copy the folder) and write a three-line note:
what compiled, what surprised you, what you'd do differently. By Ch 18 you'll have a
working networked, persistent, thread-safe store and a record of how you got there.
