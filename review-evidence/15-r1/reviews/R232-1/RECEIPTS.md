R232-1 reproducible receipts

Review root: `$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232`.
Checkout: `$VALIDATION_STORAGE/reviews/r232-18-r1`.
Head `76906b83eb54ac29040fe72fd910d34727188694`; base `deca300c9eb4863fa13383b45ba6cb6fd0828671`; tree `a34065c39ea41aeb5daae9ecae3763f632f27484`.

The reviewer scripts are standalone Python using subprocess, not product changes. Every shell invocation starts with `rtk`. `receipts/commands/*/command.json` records each principal command’s argv, working directory, sanitizer-environment assumptions, exit and duration; `stdout` and `stderr` are separate raw streams. `receipts/commands.json` indexes 89 principal validation commands. Python’s assertions are the comparison oracles and abort on an unexpected result. Object `nm` and ELF `readelf` captures are retained in JSON, including raw output, with their collection algorithm in the scripts.

Builds run sequentially, with eight jobs each. The inherited service cgroup’s memory.max is 12884901888 bytes (12 GiB). Do not replace this with a virtual-address limit: ASan needs a large virtual address space. No other build or BDD process is launched by these scripts. `receipts/host.txt` and `resource-bound.json` record the host and limit. Initial and final state records preserve tracked hashes/modes and raw index hash.

Reproduction assumes the exact base/head Git objects exist in the checkout, CMake/GCC/Make/binutils/Python are installed, and the same optional-tool availability (no tshark) applies. Initialize only the two actual dependencies:

```bash
rtk git -C $VALIDATION_STORAGE/reviews/r232-18-r1 submodule update --init --recursive --jobs 4 external/googletest external/rapidyaml
```

The scripts use absolute ROOT and an OUT derived from their own directory. To reproduce without overwriting archived receipts, copy `review.py` and `checks.py` to a new disposable directory outside the checkout and invoke those copies, with the same dependency pins. The commands below show the original review location. `prep` exports source with git archive and copies only exact pinned dependency bytes into the exported source trees. It records per-file SHA-256/modes and asserts copy equality; no dependency points into another author’s checkout.

```bash
rtk proxy python3 $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/review.py prep
rtk proxy python3 $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/review.py matrix
rtk proxy python3 $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/review.py probes
rtk proxy python3 $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/review.py mutations
rtk proxy python3 $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/checks.py registrations
rtk proxy python3 $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/checks.py exits
rtk proxy python3 $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/checks.py consumers
```

Expected script exits are all 0. Some child commands must fail: mutation CTest runs exit 8; direct fatal probes and rejected recovery wrappers exit 1; absent-runtime links exit 2; the unmodified direct test runner exits 8 against a deliberately broken helper fixture. These statuses are retained and asserted, not hidden.

`matrix` configures both source exports for empty plus all four standard build types, with BUILD_SHARED_LIBS unset and ON. The empty type is genuinely omitted from the command, not replaced with a default. For compile comparison it keys every entry by object output, verifies source identity and exact sanitizer population, and strips only the one expected sanitizer token from each changed empty-type entry before comparing token order. All explicit-type existing command strings and all existing link strings match after replacing only source/build directory roots. The flags-file comparison removes only the authorized empty-type sanitizer additions. No optimizer/debug flag normalization is used.

`probes` builds only the three probe targets in each of the five ON configurations, runs only the seven probe cases serially with verbose logging, and records object references. `mutations` uses small standalone projects with exact copied head probe sources and actual helpers. The reverted helper is exact base; PRIVATE/INTERFACE patches alter only the two target_compile_options scopes. Empty-type expected failures are asserted by exact case name. The reverted helper is also checked in Debug.

`registrations` builds all base/head default targets solely for object/runtime inspection and complete discovery. It compares all 285 original test name/command/property records and order, ignoring only backtrace indexes and normalizing source/build roots. This is not a full-suite execution. `exits` runs direct faults both with and without UBSan’s fatal option, exercises wrapper rejection, invokes the unmodified direct unit runner on the deliberately broken standalone fixture, and removes runtime link flags in a separate fixture. `consumers` checks tests-off and a real embedded Session consumer, on base/head in empty/Debug, comparing all commands and running the built outputs.

Principal artifact map:

| Path | Contents |
| --- | --- |
| `receipts/matrix/`, `matrix-summary.json` | Full normalized before/after compile population and assertions for ten base/head pairs |
| `receipts/generated/` | 2,336 raw generated files: compile databases, caches, link commands, flags, CTest registrations/logs from matrix, mutations and consumers |
| `receipts/objects/`, `runtime/` | Actual object symbol references and separate dynamic runtime dependencies |
| `receipts/registration-compare.json` | Original 285 preserved; seven additions and their exact properties |
| `receipts/mutation-*.patch`, `mutation-*-summary.json` | Exact final mutation and failure populations |
| `receipts/harness-sources/` | Standalone mutation and embedded-consumer source fixtures |
| `receipts/consumer-summary.json`, `unchanged-domains.json` | Consumer boundaries and source/byte/mode invariants |
| `receipts/commands/`, `commands.json`, `*-driver.log` | Raw command streams, commands, exits and execution summaries |
| `receipts/{initial,final}-state.json`, `checkout-preservation.json` | Tracked/index/mode preservation and exact identity |
| `receipts/submodule-init.log`, `final-submodules.txt`, `*-source-manifest.json` | Public origins, pins and dependency copy byte evidence |
| `public/` | Raw issue/PR/job metadata and logs; public author/manager archive, with roles preserved |
| `receipts/public-integrity.json`, `public-blob-verification.json`, `evidence-archive-delta.json` | Public SHA-256/Git blob checks and old/new archive comparison |
| `receipts/harness-notes.txt` | Reviewer harness corrections; no product failures |

`fetch_public.py` reproduces the public read-only API retrieval, including the pinned evidence archive and raw hosted logs. It is a durable equivalent of the collection commands used during review; run it only into a fresh review directory if preserving the original live metadata snapshot. Public issue/PR state may change; the archive and reviewed source remain pinned. All 3,106 public evidence blobs were verified against their Git SHA-1 IDs and all 3,104 published manifest SHA-256 entries matched. Author artifacts were not treated as independent runs.

`work/` holds the disposable source exports, builds and planted-fault fixtures. It is excluded from SHA256SUMS to avoid requiring archival of compiled binaries. The source archives, pins/manifests, scripts, raw generated files and test receipts are sufficient to reproduce the checks. `SHA256SUMS` covers REPORT.md, this guide, the ledger, scripts, public evidence and receipts. No reviewed-checkout artifacts need to be archived beyond the explicitly recorded submodule state.
