# Linux Process Manager with AI

A C++17 ncurses-based Process Manager for Linux (WSL). It monitors system processes, tracks resource usage history, and uses a Rule-Based AI engine to suggest actions for anomalies (High CPU, Memory Leaks, Zombies).

## 🚀 Features

1.  **Process Discovery**: Lists PID, Name, State from `/proc`.
2.  **Resource Tracking**: Real-time CPU % and Memory RSS (Resident Set Size).
3.  **TUI Interface**: Flicker-free `ncurses` table with live updates.
4.  **Process Control**: `kill`, `pause`, `resume`, `priority`.
5.  **AI Engine**: Detects:
    - Sustained High CPU (>80% for 30s)
    - CPU Spikes (>40% jump)
    - Memory Leaks (Combustive growth for 60s)
    - Zombie Processes
6.  **Interactive AI**: Accept or ignore AI suggestions.

## 🛠️ Build & Run

**Requirements**:
- Linux or WSL (Windows Subsystem for Linux)
- `g++` (supports C++17)
- `make`
- `libncurses5-dev` / `libncursesw5-dev`

**Compilation**:
```bash
make
```

**Run**:
```bash
./process_manager
```

## 🧪 How to Test Features

### 1. Process Listing & TUI
- **Action**: Run `./process_manager`.
- **Expected**: A table of processes appears, updating every second. You can see your own shell (`bash`) and the manager itself.

### 2. Command System
Type commands at the bottom prompt `Command >`:

- **Run a process**:
    ```text
    run sleep 120
    ```
    *Result*: A new `sleep` process appears in the list with State `S`.

- **Pause a process**:
    - Find the PID of `sleep`.
    - Type: `pause <PID>`
    - *Result*: State changes to `T` (Stopped).

- **Resume a process**:
    - Type: `resume <PID>`
    - *Result*: State changes back to `S` or `R`.

- **Kill a process**:
    - Type: `kill <PID>`
    - *Result*: The process disappears from the list.

### 3. AI Rules & Warnings

The AI Engine analyzes history (last 60 seconds). Warnings appear in **RED** above the command line.

#### Test Case A: High CPU Warning
1.  **Simulate**: Run a CPU intensive task.
    ```bash
    # In another terminal:
    stress --cpu 1
    ```
    (Or just `yes > /dev/null &` if `stress` is not installed).
2.  **Wait**: ~30 seconds.
3.  **Result**: Warning appears: `[!] WARNINGS: HIGH_CPU (PID ...)`.

#### Test Case B: Zombie Process
1.  **Simulate**: Create a zombie.
    ```bash
    # In another terminal, run a script that creates a zombie
    (sleep 1 & exec /bin/sleep 10)
    ```
    (Or compile a C small program that forks and exits child).
2.  **Result**: Warning appears: `[!] WARNINGS: ZOMBIE (PID ...)`.

### 4. AI Interaction
You can respond to warnings using the `ai` command.

- **Ignore a warning**:
    ```text
    ai <PID> ignore
    ```
    *Result*: The red warning for that PID disappears.

- **Accept AI suggestion (Kill)**:
    ```text
    ai <PID> kill
    ```
    *Result*: The process is terminated.

## 📂 Project Structure

- `src/ProcessScanner`: Reads `/proc`.
- `src/RuleEngine`: Implements the 4 AI rules.
- `src/ProcessInfo`: Stores data and history (60s buffer).
- `src/ProcessController`: Handles `kill`, `fork`, `exec`.
- `src/TUI`: Render interface using `ncurses`.

## 📜 Command Reference

| Command | Syntax | Description |
| :--- | :--- | :--- |
| **Run** | `run <cmd> [args]` | Starts a new process (e.g., `run sleep 10`). |
| **Kill** | `kill <pid>` | Sends `SIGTERM` to stop a process. |
| **Pause** | `pause <pid>` | Sends `SIGSTOP` to freeze a process. State becomes `T`. |
| **Resume** | `resume <pid>` | Sends `SIGCONT` to unfreeze. State becomes `S` or `R`. |
| **Priority**| `priority <pid> <val>`| Sets nice value (e.g., `priority 123 10`). |
| **Sort** | `sort <cpu\|mem\|pid>` | Sorts the process list (e.g., `sort cpu`). |
| **Filter** | `filter <text>` | Filters list by name. Empty `filter` clears it. |
| **Info** | `info <pid>` | Shows detailed stats & history graph for a process. |
| **Tree** | `tree` | Toggles hierarchical process tree view. |
| **AI Ignore**| `ai <pid> ignore` | Silences AI warnings for this PID. |
| **AI Kill** | `ai <pid> kill` | Kills the process (shortcut for resolving warnings). |
| **AI Pause**| `ai <pid> pause` | Pauses the process. |
| **Quit** | `q` or `quit` | Exits the Process Manager. |
