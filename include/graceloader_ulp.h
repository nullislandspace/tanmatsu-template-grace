// SPDX-License-Identifier: MIT
// Graceloader ULP API -- the ESP32-P4 low power core (LP core) as a pathfinding coprocessor
//
// The LP core is a third physical core in the P4: RV32IMAC, no FPU, no FreeRTOS,
// no C library beyond what the LP ROM provides, and it runs *concurrently* with
// both HP cores while the system is awake. It has 32 KiB of its own RAM (LPRAM,
// 0x50108000..0x50110000) and it can read the internal HP SRAM, but it cannot
// touch PSRAM or flash at all. Everything it works on therefore has to be put
// into the slice of LPRAM that ESP-IDF reserves for it
// (CONFIG_ULP_COPROC_RESERVE_MEM) before it is asked to start.
//
// This header defines the one mailbox the HP side and the LP-side program share
// inside that reserved slice. It is the whole ABI between an app's ULP program
// and graceloader, and it is compiled by both sides: the app builds it into its
// ULP binary with the .ulp_mbox section attribute, graceloader reaches the same
// bytes through graceloader_ulp_mailbox(). See claudeplans/pathfinding.md in the
// SynthMiner repository for the algorithm the mailbox is shaped around.
//
// Two things about the mailbox address are worth knowing before using it:
//
//   - The ULP binary is linked at absolute addresses, not position independent:
//     its sections land at fixed offsets inside the reserved LPRAM slice. The
//     mailbox is therefore not "somewhere in the blob", it is at a fixed address
//     that both sides can compute independently. See graceloader_ulp_mailbox().
//   - CONFIG_ULP_COPROC_RESERVE_MEM is part of that ABI. Change it in
//     graceloader's sdkconfig and every app's ULP binary has to be rebuilt,
//     because the linker script the app builds against derives the mailbox
//     address from it.
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Three things compile this file, and only one of them is an ESP-IDF program:
//
//   - graceloader itself (main/ulp.c), which implements the calls below;
//   - an app's ULP binary, which is bare metal RV32 with no C library to
//     speak of and only ever touches the structs;
//   - a host check that runs the same solver the ULP binary runs, to prove
//     the algorithm without a badge.
//
// The last two define GRACELOADER_ULP_LAYOUT_ONLY before including and get
// the mailbox layout and nothing else. That is not a nicety: the layout IS
// the ABI, and the whole point of sharing one header is that the app's ULP
// binary, the host check and graceloader cannot disagree about where a byte
// sits. The esp_err.h below drags in the whole toolchain's libc headers,
// which is exactly what a host check compiling against a vendored copy of
// them must not do.
#ifndef GRACELOADER_ULP_LAYOUT_ONLY
#include "esp_err.h"

// True when this build has an LP core at all. An app that wants to say "no
// coprocessor here" out loud rather than fail every call with
// ESP_ERR_NOT_SUPPORTED asks this instead of guessing from the target.
#if defined(CONFIG_ULP_COPROC_TYPE_LP_CORE)
#define GRACELOADER_ULP_AVAILABLE 1
#else
#define GRACELOADER_ULP_AVAILABLE 0
#endif
#endif  // GRACELOADER_ULP_LAYOUT_ONLY

#ifdef __cplusplus
extern "C" {
#endif

// ---------------------------------------------------------------------------
// ABI identity
//
// The LP-side program writes GRACELOADER_ULP_ABI_MAGIC into the mailbox as the
// first thing main() does. graceloader_ulp_start() waits for it, so an app that
// loads the wrong binary, a binary built against a different reserve size, or a
// binary that crashes before main() gets a clean error instead of a mailbox
// that quietly never answers.
// ---------------------------------------------------------------------------
#define GRACELOADER_ULP_ABI_MAGIC 0x4C505547u  // "GULP"
#define GRACELOADER_ULP_ABI_VERSION 1u

// Size of the mailbox carved out of the reserved LPRAM by the ULP linker
// script. The ULP build passes this to the linker as GRACELOADER_ULP_MBOX_BYTES;
// graceloader_ulp_mailbox() subtracts it from the start of the IDF shared-memory
// block, which is the last thing in the reserve. Both sides land on the same
// address without either one hardcoding it.
#define GRACELOADER_ULP_MBOX_BYTES 1024

// ---------------------------------------------------------------------------
// The pathfinding window
//
// A cube of SIDE^3 cells handed over to the LP core, in window-local
// coordinates. Cell (x, y, z) is index ((y * SIDE) + z) * SIDE + x, so x is the
// fastest-varying axis and a horizontal step is a +-1 on the index.
//
// 8^3 = 512 cells is the size the LP core's 32 KiB of RAM and its ~17.5 MHz
// clock make comfortable; see the memory and time budget in pathfinding.md.
// ---------------------------------------------------------------------------
#define GRACELOADER_ULP_PATH_SIDE 8
#define GRACELOADER_ULP_PATH_CELLS (GRACELOADER_ULP_PATH_SIDE * GRACELOADER_ULP_PATH_SIDE * GRACELOADER_ULP_PATH_SIDE)
#define GRACELOADER_ULP_PATH_COLUMNS (GRACELOADER_ULP_PATH_SIDE * GRACELOADER_ULP_PATH_SIDE)

// base_mask[] in the request below is a bitmap of the columns, so it is a
// number of bytes rather than a number of entries.
#define GRACELOADER_ULP_PATH_BASE_BYTES (GRACELOADER_ULP_PATH_COLUMNS / 8)

// How many waypoints the LP core hands back. The mob walks to the first one,
// and the next request goes out once it has passed it, so a short list is fine
// and a long one would only be stale by the time it is used.
#define GRACELOADER_ULP_PATH_MAX_NODES 8

#define GRACELOADER_ULP_PATH_IDX(x, y, z) ((((y) * GRACELOADER_ULP_PATH_SIDE) + (z)) * GRACELOADER_ULP_PATH_SIDE + (x))

// ---------------------------------------------------------------------------
// What a cell is made of
//
// Two bits per cell, chosen so that "can a mob stand here" is a lookup rather
// than a switch. BLOCKED is the catch-all for everything the game does not want
// a mob to walk into: fences, closed doors, fire, cactus, a block the mob would
// take damage standing in. The LP core does not know what a cactus is; the game
// decides that when it fills the map in.
// ---------------------------------------------------------------------------
typedef enum {
    GRACELOADER_ULP_MAT_AIR = 0,     // empty, mob walks through
    GRACELOADER_ULP_MAT_SOLID = 1,   // full block, not walkable
    GRACELOADER_ULP_MAT_WATER = 2,   // walkable, costs more (see cost_mat)
    GRACELOADER_ULP_MAT_BLOCKED = 3, // walkable shape, the game says no
} graceloader_ulp_material_t;

// ---------------------------------------------------------------------------
// Who is asking
//
// Only used to pick the cost table; the search itself is the same for every
// mob. A zombie should wade through water to reach the player, a cow should
// walk around it if any dry route exists, and both are the same search with
// different numbers in cost_mat.
// ---------------------------------------------------------------------------
typedef enum {
    GRACELOADER_ULP_KIND_HOSTILE = 0,  // zombie, skeleton: water is not a problem
    GRACELOADER_ULP_KIND_PASSIVE = 1,  // cow, pig: water is expensive, drops are scary
    GRACELOADER_ULP_KIND_FOLLOWER = 2, // tamed/leashed pet following the player
    GRACELOADER_ULP_KIND_COUNT = 3
} graceloader_ulp_kind_t;

// ---------------------------------------------------------------------------
// Step costs
//
// All costs are integers, in arbitrary units: the LP core has no FPU and a
// fixed-point one would only be a slower integer one. The heuristic the search
// uses is the cheapest possible step cost times the remaining distance, so the
// cheapest entry in cost_mat has to be the true minimum or the search stops
// being optimal.
//
// cost_mat[] is the cost of entering a cell of that material. cost_step[] is
// the extra cost of the height change, indexed by how far the mob steps up or
// down; "perilous drop" is a cost, not a wall, so a mob will take a short drop
// rather than give up, but will walk further to avoid a long one.
// ---------------------------------------------------------------------------
typedef enum {
    GRACELOADER_ULP_STEP_FLAT = 0,   // same level
    GRACELOADER_ULP_STEP_DOWN1 = 1,  // one block down
    GRACELOADER_ULP_STEP_UP1 = 2,    // one block up (a step the mob can climb)
    GRACELOADER_ULP_STEP_DOWN2 = 3,  // two blocks down
    GRACELOADER_ULP_STEP_UP2 = 4,    // two blocks up: not climbable, impassable
    GRACELOADER_ULP_STEP_DROP3 = 5,  // three or more down: fall damage territory
    GRACELOADER_ULP_STEP_COUNT = 6
} graceloader_ulp_step_t;

// A cost of GRACELOADER_ULP_COST_IMPASSABLE in either table means "no".
#define GRACELOADER_ULP_COST_IMPASSABLE 0xFFu

// ---------------------------------------------------------------------------
// Request: HP core -> LP core
// ---------------------------------------------------------------------------
typedef struct {
    // Written last. The LP core only starts a search when req_seq differs from
    // res.res_seq, so the map, the goal and the costs can all be filled in
    // first and the request published with one 32 bit store.
    uint32_t req_seq;

    uint8_t kind;             // graceloader_ulp_kind_t
    uint8_t side;             // GRACELOADER_ULP_PATH_SIDE
    uint8_t start[3];         // window-local cell the mob is standing in
    uint8_t goal[3];          // window-local cell the mob wants to reach
    uint8_t flags;
    uint8_t pad;

    uint8_t cost_mat[4];      // indexed by graceloader_ulp_material_t
    uint8_t cost_step[GRACELOADER_ULP_STEP_COUNT];

    // A search that expands this many cells gives up and reports
    // GRACELOADER_ULP_PATH_NOT_FOUND. It bounds the worst case, which matters
    // because the LP core is a 17.5 MHz core sharing memory with the game.
    uint16_t max_expansions;

    uint8_t occ[GRACELOADER_ULP_PATH_CELLS];   // graceloader_ulp_material_t per cell

    // What is directly BELOW the window, one bit per column (bit z * SIDE + x
    // of byte (z * SIDE + x) / 8): set when the block one under the bottom
    // layer of the window is solid.
    //
    // The occupancy map says what every cell in the window is made of, and a
    // cell is only somewhere a mob can stand if there is something solid
    // under it. That is answerable for every layer but the lowest one, whose
    // floor is outside the window and therefore outside the map. base_mask is
    // the window's own foundation: without it the search has to treat the
    // bottom layer as bottomless, which is a map whose ground stops one block
    // above where the ground actually is.
    uint8_t base_mask[GRACELOADER_ULP_PATH_BASE_BYTES];
} graceloader_ulp_path_req_t;

// THERE IS NO "HEIGHT OF THE GROUND" FIELD, and that is deliberate. Two bits
// per cell say what a cell is made of and nothing about how high the ground
// is, and the ends of a route usually need that: the player is a body standing
// IN a cell with their feet at its floor, and a mob shoved off a bank is
// standing in mid-air. The search answers it itself, by walking up the column
// of the cell it was given until it finds one it would be willing to route
// through -- eight lookups, twice per request, which is nothing next to the
// search itself.
//
// The alternative was a floor_h[] table the game filled in, and it is gone
// because it made the game re-implement "can a mob stand here" to feed a
// question only the search asks. Two definitions of standable, one on each
// side of a core boundary, is two definitions that drift.

// req.flags
#define GRACELOADER_ULP_FLAG_ALLOW_WATER 0x01u  // clear to treat water as impassable
#define GRACELOADER_ULP_FLAG_ALLOW_DROP 0x02u   // clear to treat drops as impassable

// ---------------------------------------------------------------------------
// Reply: LP core -> HP core
// ---------------------------------------------------------------------------
typedef enum {
    GRACELOADER_ULP_PATH_IDLE = 0,      // nothing solved yet
    GRACELOADER_ULP_PATH_FOUND = 1,     // path[] holds path_len waypoints
    GRACELOADER_ULP_PATH_NOT_FOUND = 2, // searched the window, no route
    GRACELOADER_ULP_PATH_NO_START = 3,  // start cell is not standable
    GRACELOADER_ULP_PATH_NO_GOAL = 4,   // goal cell is not standable
    GRACELOADER_ULP_PATH_BAD_REQUEST = 5, // side/indices/cost table nonsense
    GRACELOADER_ULP_PATH_LIMIT = 6,     // hit max_expansions before finishing
} graceloader_ulp_path_status_t;

typedef struct {
    // Set to req.req_seq when the rest of this struct is valid. The HP side
    // polls it; there is no interrupt from the LP core to the HP core in this
    // design, on purpose: the game only looks once per tick anyway.
    uint32_t res_seq;

    uint16_t status;      // graceloader_ulp_path_status_t
    uint8_t path_len;
    uint8_t path[GRACELOADER_ULP_PATH_MAX_NODES * 3];  // window-local x,y,z, nearest first

    uint16_t expansions;  // cells closed by the search
    uint16_t generated;   // neighbours considered
    uint16_t path_cost;   // total cost of the returned path
    uint16_t _reserved;

    // LP core mcycle difference across the search. At the ~17.5 MHz RC_FAST
    // clock graceloader runs the LP core at, cycles / 17.5 == microseconds.
    uint32_t cycles;
} graceloader_ulp_path_res_t;

// ---------------------------------------------------------------------------
// Control block: LP core -> HP core, written by the ULP program itself
// ---------------------------------------------------------------------------
typedef struct {
    uint32_t magic;          // GRACELOADER_ULP_ABI_MAGIC once main() has run
    uint32_t version;        // GRACELOADER_ULP_ABI_VERSION
    uint32_t solves;         // searches finished since the blob booted
    uint32_t errors;         // searches that ended in a status other than FOUND
    uint32_t wakeups;        // times the LP core woke up for something
    uint32_t last_exception; // exception cause, if the LP core trapped
    uint32_t boot_cycles;    // mcycle at main(), for spotting a re-boot
    uint32_t busy;           // 1 while a search is in flight
} graceloader_ulp_ctl_t;

// ---------------------------------------------------------------------------
// The mailbox
//
// The .ulp_mbox section attribute is what puts this at the fixed address the
// linker script carves out; a ULP program must define exactly one object with
// it. On the HP side the same address comes from graceloader_ulp_mailbox().
// ---------------------------------------------------------------------------
typedef struct {
    graceloader_ulp_ctl_t ctl;
    graceloader_ulp_path_req_t req;
    graceloader_ulp_path_res_t res;
    uint8_t spare[GRACELOADER_ULP_MBOX_BYTES - sizeof(graceloader_ulp_ctl_t) -
                  sizeof(graceloader_ulp_path_req_t) - sizeof(graceloader_ulp_path_res_t)];
} graceloader_ulp_mbox_t;

#if defined(GRACELOADER_ULP_MBOX_CHECK)
_Static_assert(sizeof(graceloader_ulp_mbox_t) <= GRACELOADER_ULP_MBOX_BYTES,
               "ULP mailbox does not fit the space the linker script carves out");
_Static_assert(GRACELOADER_ULP_PATH_CELLS == 512, "ULP path window is not 8^3");
#endif

// Everything below is the HP-side API, which only an ESP-IDF program can
// call. A ULP binary and a host check stop at the mailbox above.
#ifndef GRACELOADER_ULP_LAYOUT_ONLY

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

// Load a ULP binary from a file on the SD card or in appfs. The path is read
// into a scratch buffer and handed to ulp_lp_core_load_binary(), which stops the
// LP core, zeroes the whole reserved slice and copies the image in. Only one
// binary can be resident at a time; loading a second one replaces the first.
//
// Returns ESP_ERR_INVALID_SIZE if the file is bigger than the reserved slice,
// which is the usual sign that the app built its ULP binary against a different
// CONFIG_ULP_COPROC_RESERVE_MEM than graceloader was configured with.
esp_err_t graceloader_ulp_load_file(const char* path);

// Load a ULP binary from memory, for an app that already has it in a buffer.
esp_err_t graceloader_ulp_load_binary(const uint8_t* image, size_t len);

// Start the loaded binary and wait for it to report its ABI magic.
//
// timeout_ms bounds the wait; 0 uses the default. Returns ESP_ERR_NOT_FOUND if
// the mailbox never gets its magic, which means the blob is missing, is the
// wrong one, or trapped before main().
esp_err_t graceloader_ulp_start(uint32_t timeout_ms);

// Put the LP core to sleep. The blob stays resident, so graceloader_ulp_start()
// can be called again without loading anything. An app that only wants the
// coprocessor while its game loop runs stops it between loops.
esp_err_t graceloader_ulp_stop(void);

// True while the LP core is awake and running the loaded blob.
bool graceloader_ulp_is_running(void);

// True once a blob is resident in LPRAM (loaded, whether or not it is running).
bool graceloader_ulp_is_loaded(void);

// The reserved CONFIG_ULP_COPROC_RESERVE_MEM, so an app can check its own ULP
// binary against it before trying to load it.
size_t graceloader_ulp_reserve_bytes(void);

// The mailbox, at its fixed address inside the reserved LPRAM slice. NULL if
// this build has no LP core. The pointer stays valid across load/start/stop;
// only the bytes inside it change.
graceloader_ulp_mbox_t* graceloader_ulp_mailbox(void);

// ---------------------------------------------------------------------------
// Pathfinding
//
// Fill in the request the LP core will read, publish it, and pick the answer up
// on a later tick. Nothing here blocks: a search over the 8^3 window takes a
// few milliseconds on the LP core while the HP core carries on rendering, and
// the game only looks at the result when it next ticks.
// ---------------------------------------------------------------------------

// The request slot to fill in. Writing through this pointer writes straight into
// LPRAM; the LP core does not look at it until graceloader_ulp_path_submit().
graceloader_ulp_path_req_t* graceloader_ulp_path_request(void);

// Publish the request and wake the LP core. Returns ESP_ERR_INVALID_STATE if no
// blob is loaded or the LP core is stopped, and ESP_ERR_BUSY if the previous
// request has not been answered yet (poll or drop it first).
esp_err_t graceloader_ulp_path_submit(void);

// True and *out filled in if the request published by the last
// graceloader_ulp_path_submit() has an answer. False if the LP core is still
// working on it, in which case the caller should look again next tick.
bool graceloader_ulp_path_poll(graceloader_ulp_path_res_t* out);

// Drop an outstanding request without waiting for it. The next submit() gets a
// fresh sequence number and the stale answer is ignored.
void graceloader_ulp_path_abort(void);

// The LP core's own counters, plus what graceloader counted on the HP side.
// Handy for the "is the ULP actually doing anything" question, which is the
// first question any ULP bring-up has.
typedef struct {
    bool loaded;
    bool running;
    uint32_t reserve_bytes;
    uint32_t submits;        // graceloader_ulp_path_submit() calls
    uint32_t polls;          // graceloader_ulp_path_poll() calls
    uint32_t answers;        // polls that found an answer
    uint32_t timeouts;       // submits whose answer never arrived
    uint32_t last_res_seq;
    graceloader_ulp_ctl_t ctl;  // copied out of the mailbox
} graceloader_ulp_stats_t;

void graceloader_ulp_get_stats(graceloader_ulp_stats_t* out);

#endif  // GRACELOADER_ULP_LAYOUT_ONLY

#ifdef __cplusplus
}
#endif
