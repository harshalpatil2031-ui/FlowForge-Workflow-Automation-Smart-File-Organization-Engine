# FlowForge — Implementation Guide
### Workflow Automation & Smart File Organization Engine
**Language**: Turbo C++ (Borland 3.0 / 3.1 Compatible)**  
**Paradigm**: Object-Oriented Programming  
**Team Size**: 3 Members  

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Folder & File Structure](#2-folder--file-structure)
3. [Architecture Overview](#3-architecture-overview)
4. [Core Data Structures](#4-core-data-structures)
5. [Class Definitions](#5-class-definitions)
   - 5.1 [Abstract Task Base Class](#51-abstract-task-base-class)
   - 5.2 [Concrete Task Classes](#52-concrete-task-classes)
   - 5.3 [Abstract Rule Base Class](#53-abstract-rule-base-class)
   - 5.4 [Concrete Rule Classes](#54-concrete-rule-classes)
   - 5.5 [SmartSort Engine](#55-smartsort-engine)
   - 5.6 [Workflow Manager](#56-workflow-manager)
   - 5.7 [Resilience Engine](#57-resilience-engine)
   - 5.8 [Logger](#58-logger)
6. [Execution Flow](#6-execution-flow)
7. [OOP Concepts Mapping](#7-oop-concepts-mapping)
8. [Turbo C++ Specific Notes](#8-turbo-c-specific-notes)
9. [UI Design (conio.h)](#9-ui-design-conioh)
10. [Task Checklist](#10-task-checklist)

---

## 1. Project Overview

**FlowForge** is a console-based workflow automation system built in Turbo C++.

It allows users to:
- Define a **sequence of file operations** as a single reusable **Workflow**
- Use **SmartSort** to automatically classify and organize files by type, size, or age
- Execute the entire workflow with one command
- Automatically recover from failures using the **Resilience Engine**
- Log every operation to `FLOW.LOG` for audit and review

> **Core Formula:**  
> `FlowForge = Workflow Automation + SmartSort + Failure Recovery`

---

## 2. Folder & File Structure

```
FlowForge/
│
├── main.cpp              ← Entry point, main menu UI
│
├── fileinfo.h            ← FileInfo struct (shared data model)
│
├── task.h                ← Abstract Task base class
├── tasks.h               ← All concrete Task class declarations
├── tasks.cpp             ← All concrete Task implementations
│
├── rule.h                ← Abstract Rule base class
├── rules.h               ← All concrete Rule class declarations
├── rules.cpp             ← All concrete Rule implementations
│
├── smartsort.h           ← SmartSort engine declaration
├── smartsort.cpp         ← SmartSort engine implementation
│
├── workflow.h            ← WorkflowManager declaration
├── workflow.cpp          ← WorkflowManager implementation
│
├── resilience.h          ← ResilienceEngine declaration
├── resilience.cpp        ← ResilienceEngine implementation
│
├── logger.h              ← Logger declaration
├── logger.cpp            ← Logger implementation
│
├── ui.h                  ← UI helper functions (conio.h wrappers)
├── ui.cpp                ← UI helper implementations
│
├── WORKFLOW.DAT          ← Saved workflow data (binary file)
└── FLOW.LOG              ← Execution log file (text file)
```

> **Turbo C++ Note**: Keep all filenames in UPPERCASE or short lowercase. Avoid spaces in filenames.

---

## 3. Architecture Overview

```
┌─────────────────────────────────────────────────────────┐
│                        main.cpp                         │
│                    (Menu & Entry Point)                  │
└──────────────────────────┬──────────────────────────────┘
                           │
           ┌───────────────▼───────────────┐
           │        WorkflowManager        │
           │  (Creates, Saves, Executes    │
           │        Workflows)             │
           └───────────────┬───────────────┘
                           │
          ┌────────────────▼────────────────┐
          │          Task Engine            │
          │  Task* taskList[MAX_TASKS]      │
          │  Calls task->execute() in loop  │
          └────┬──────────┬────────┬────────┘
               │          │        │
        ┌──────▼──┐  ┌────▼───┐  ┌▼──────────┐
        │Rename   │  │Move    │  │SmartSort  │
        │Task     │  │Task    │  │Task       │
        └─────────┘  └────────┘  └─────┬─────┘
                                        │
                              ┌─────────▼─────────┐
                              │   SmartSort Engine │
                              │  Rule* rules[]     │
                              │  FileTypeRule      │
                              │  SizeRule          │
                              │  AgeRule           │
                              └────────────────────┘
                           │
           ┌───────────────▼───────────────┐
           │       Resilience Engine       │
           │  Detects failure → Retry /    │
           │  AutoFix / Fallback / Abort   │
           └───────────────┬───────────────┘
                           │
           ┌───────────────▼───────────────┐
           │            Logger             │
           │  Writes all events to         │
           │  FLOW.LOG                     │
           └───────────────────────────────┘
```

---

## 4. Core Data Structures

### `FileInfo` struct — `fileinfo.h`

This is the **central data model** shared across all tasks and rules.  
Every task operates on a `FileInfo` object.

```cpp
// fileinfo.h
#ifndef FILEINFO_H
#define FILEINFO_H

struct FileInfo {
    char name[64];        // Original filename  e.g. "assignment.pdf"
    char extension[10];   // File extension     e.g. "pdf"
    char sourcePath[128]; // Current path       e.g. "C:\DOWNLOADS\"
    char destPath[128];   // Target path        e.g. "C:\DOCUMENTS\"
    long sizeBytes;       // File size in bytes e.g. 204800
    int  ageDays;         // Days since created e.g. 30
    int  isVirtual;       // 1 = simulated file, 0 = real file
};

#endif
```

> **Why `isVirtual`?**  
> During demos or when DOSBox filesystem access is not available, the system can operate on simulated virtual files. All OOP and workflow logic runs identically — only the final system call (rename/copy/mkdir) is skipped for virtual files.

---

### `TaskResult` enum — `task.h`

Every task returns a result after execution:

```cpp
enum TaskResult {
    SUCCESS,       // Task completed successfully
    FAILURE,       // Task failed, recovery required
    SKIPPED        // Task was skipped (e.g. file already exists)
};
```

---

### `ErrorType` enum — `resilience.h`

The Resilience Engine identifies failure types:

```cpp
enum ErrorType {
    ERR_FOLDER_NOT_FOUND,   // Destination folder missing
    ERR_FILE_NOT_FOUND,     // Source file missing
    ERR_PERMISSION_DENIED,  // Cannot access file/folder
    ERR_DISK_FULL,          // Not enough space
    ERR_UNKNOWN             // Unidentified error
};
```

---

## 5. Class Definitions

### 5.1 Abstract Task Base Class

**File**: `task.h`

```cpp
// task.h
#ifndef TASK_H
#define TASK_H

#include "fileinfo.h"

enum TaskResult { SUCCESS, FAILURE, SKIPPED };

class Task {
protected:
    char taskName[64];      // Human-readable name of the task
    FileInfo file;          // The file this task operates on
    int  maxRetries;        // Max retry attempts (default: 2)
    int  retryCount;        // Current retry attempt number

public:
    // Constructor
    Task(const char* name, FileInfo f, int retries = 2);

    // PURE VIRTUAL — every derived task MUST implement this
    virtual TaskResult execute() = 0;

    // PURE VIRTUAL — returns a description of what this task does
    virtual void describe() = 0;

    // Common getters
    const char* getTaskName();
    FileInfo    getFile();
    int         getRetryCount();

    // Virtual destructor (required for polymorphic deletion)
    virtual ~Task();
};

#endif
```

---

### 5.2 Concrete Task Classes

**File**: `tasks.h` (declarations) + `tasks.cpp` (implementations)

---

#### RenameTask
Renames a file from its current name to a new name.

```cpp
class RenameTask : public Task {
private:
    char newName[64];   // The new filename to assign

public:
    RenameTask(FileInfo f, const char* newName);

    TaskResult execute();   // Calls rename() or simulates it
    void describe();        // Prints: "Rename [old] -> [new]"
};
```

**Logic inside `execute()`:**
```
1. Build full source path: sourcePath + name
2. Build full dest path:   sourcePath + newName
3. If file.isVirtual == 1:
       Update file.name = newName
       Return SUCCESS
   Else:
       Call rename(srcPath, destPath) from <stdio.h>
       If rename() returns 0 → Return SUCCESS
       Else → Return FAILURE
```

---

#### CreateFolderTask
Creates a new directory at a specified path.

```cpp
class CreateFolderTask : public Task {
private:
    char folderPath[128];   // The path of folder to create

public:
    CreateFolderTask(FileInfo f, const char* path);

    TaskResult execute();   // Calls mkdir() or simulates it
    void describe();        // Prints: "Create Folder: [path]"
};
```

**Logic inside `execute()`:**
```
1. If file.isVirtual == 1:
       Print "[SIMULATED] Created folder: folderPath"
       Return SUCCESS
   Else:
       Call mkdir(folderPath) from <dir.h>
       If mkdir returns 0 → Return SUCCESS
       Else if folder already exists → Return SKIPPED
       Else → Return FAILURE
```

---

#### MoveTask
Moves a file from source path to destination path.

```cpp
class MoveTask : public Task {
public:
    MoveTask(FileInfo f);

    TaskResult execute();   // Uses rename() across paths or simulates
    void describe();        // Prints: "Move [file] to [destPath]"
};
```

**Logic inside `execute()`:**
```
1. Build srcFull  = file.sourcePath + file.name
2. Build destFull = file.destPath   + file.name
3. If file.isVirtual == 1:
       Update file.sourcePath = file.destPath
       Return SUCCESS
   Else:
       Call rename(srcFull, destFull)
       If success → Return SUCCESS
       Else → Return FAILURE
       (Resilience Engine will handle FAILURE)
```

---

#### BackupTask
Creates a copy of a file with a `_BAK` suffix at the destination.

```cpp
class BackupTask : public Task {
private:
    char backupPath[128];   // Where the backup is stored

public:
    BackupTask(FileInfo f, const char* backupPath);

    TaskResult execute();   // Copies file byte-by-byte using fopen/fwrite
    void describe();        // Prints: "Backup [file] to [backupPath]"
};
```

**Logic inside `execute()`:**
```
1. Build backup filename: file.name + "_BAK"
2. If file.isVirtual == 1:
       Print "[SIMULATED] Backup created: backupName"
       Return SUCCESS
   Else:
       Open source file with fopen() for reading (binary)
       Open dest   file with fopen() for writing (binary)
       Read source in chunks → Write to dest
       Close both files
       If successful → Return SUCCESS
       Else → Return FAILURE
```

---

#### VerifyTask
Verifies that a backup file exists and is non-empty.

```cpp
class VerifyTask : public Task {
private:
    char backupPath[128];   // Path of backup to verify

public:
    VerifyTask(FileInfo f, const char* backupPath);

    TaskResult execute();   // Checks file exists and size > 0
    void describe();        // Prints: "Verify Backup: [file]"
};
```

**Logic inside `execute()`:**
```
1. If file.isVirtual == 1:
       Print "[SIMULATED] Verification passed"
       Return SUCCESS
   Else:
       Use access() from <io.h> to check file exists
       Use filelength() or fseek+ftell to check size > 0
       If both pass → Return SUCCESS
       Else → Return FAILURE
```

---

#### SmartSortTask
Uses the SmartSort Engine to classify and move a file based on rules.

```cpp
class SmartSortTask : public Task {
private:
    SmartSortEngine* engine;   // Pointer to the rules engine

public:
    SmartSortTask(FileInfo f, SmartSortEngine* engine);

    TaskResult execute();   // Asks SmartSort engine for destination, then moves
    void describe();        // Prints: "SmartSort: [file]"
};
```

**Logic inside `execute()`:**
```
1. Pass file to engine->getDestination(file)
2. Engine returns matched destination folder path
3. Update file.destPath = returned path
4. Call MoveTask(file).execute() to perform the move
5. Return result of MoveTask
```

---

### 5.3 Abstract Rule Base Class

**File**: `rule.h`

```cpp
// rule.h
#ifndef RULE_H
#define RULE_H

#include "fileinfo.h"

class Rule {
protected:
    char destinationFolder[128];  // Where matching files go

public:
    Rule(const char* destFolder);

    // PURE VIRTUAL — each rule checks a different condition
    virtual int matches(FileInfo file) = 0;

    // Returns the destination for matched files
    const char* getDestination();

    virtual void describe() = 0;

    virtual ~Rule();
};

#endif
```

---

### 5.4 Concrete Rule Classes

**File**: `rules.h` (declarations) + `rules.cpp` (implementations)

---

#### FileTypeRule
Matches files based on their extension.

```cpp
class FileTypeRule : public Rule {
private:
    char extension[10];   // e.g. "pdf", "jpg", "mp4"

public:
    FileTypeRule(const char* ext, const char* destFolder);

    int  matches(FileInfo file);  // Returns 1 if file.extension == extension
    void describe();              // Prints: "*.ext → destFolder"
};
```

---

#### SizeRule
Matches files based on size threshold.

```cpp
class SizeRule : public Rule {
private:
    long minSizeBytes;   // Files >= this size are matched

public:
    SizeRule(long minBytes, const char* destFolder);

    int  matches(FileInfo file);  // Returns 1 if file.sizeBytes >= minSizeBytes
    void describe();              // Prints: "Size >= Xmb → destFolder"
};
```

---

#### AgeRule
Matches files older than a specified number of days.

```cpp
class AgeRule : public Rule {
private:
    int minAgeDays;   // Files older than this are matched

public:
    AgeRule(int days, const char* destFolder);

    int  matches(FileInfo file);  // Returns 1 if file.ageDays >= minAgeDays
    void describe();              // Prints: "Older than X days → destFolder"
};
```

---

### 5.5 SmartSort Engine

**File**: `smartsort.h` + `smartsort.cpp`

```cpp
class SmartSortEngine {
private:
    Rule*  rules[20];    // Array of Rule pointers (polymorphic)
    int    ruleCount;    // Number of active rules

public:
    SmartSortEngine();

    void        addRule(Rule* rule);            // Add a rule to the engine
    const char* getDestination(FileInfo file);  // Returns matched dest folder
                                                // Returns "" if no rule matches
    void        showAllRules();                 // Lists all rules to screen
    void        clearRules();                   // Removes all rules
    int         getRuleCount();

    ~SmartSortEngine();   // Deletes all rule pointers
};
```

**Logic inside `getDestination()`:**
```
For each rule in rules[0..ruleCount-1]:
    If rule->matches(file) == 1:
        Return rule->getDestination()
Return ""   ← no rule matched
```

---

### 5.6 Workflow Manager

**File**: `workflow.h` + `workflow.cpp`

```cpp
class WorkflowManager {
private:
    char   workflowName[64];       // Name given by user
    Task*  taskList[20];           // Array of Task pointers
    int    taskCount;              // Number of tasks added
    ResilienceEngine* recovery;    // Handles failures
    Logger*           logger;      // Logs every event

public:
    WorkflowManager(const char* name, ResilienceEngine* re, Logger* log);

    void addTask(Task* task);          // Appends a task to taskList
    void removeTask(int index);        // Removes task at position index
    void showTasks();                  // Lists all tasks with index numbers

    void executeWorkflow();            // Main execution loop (see below)

    void saveToFile(const char* path); // Saves workflow metadata to WORKFLOW.DAT
    void loadFromFile(const char* path);// Loads workflow from WORKFLOW.DAT

    const char* getName();
    int         getTaskCount();

    ~WorkflowManager();   // Deletes all task pointers
};
```

**Logic inside `executeWorkflow()`:**
```
For each task in taskList[0..taskCount-1]:
    logger->log("Executing: " + task->getTaskName())
    Print task->describe()

    result = task->execute()

    If result == SUCCESS:
        logger->log("SUCCESS: " + task->getTaskName())
        Print "✓ Done"

    If result == FAILURE:
        logger->log("FAILURE: " + task->getTaskName())
        recovered = recovery->handle(task)

        If recovered == 1:
            logger->log("RECOVERED: " + task->getTaskName())
            Print "↻ Recovered"
        Else:
            logger->log("ABORTED: " + task->getTaskName())
            Print "✗ Aborted"
            Break   ← Stop workflow

    If result == SKIPPED:
        logger->log("SKIPPED: " + task->getTaskName())
        Print "→ Skipped"

Print "WORKFLOW COMPLETE"
```

---

### 5.7 Resilience Engine

**File**: `resilience.h` + `resilience.cpp`

```cpp
class ResilienceEngine {
private:
    int maxRetries;   // Global max retries (default: 3)

    ErrorType detectError(Task* task);   // Inspects task to identify error type
    int retryTask(Task* task);           // Retries task->execute()
    int autoFix(Task* task, ErrorType e);// Auto-creates folder, etc.
    int fallback(Task* task);            // Attempts alternative approach
    int abort(Task* task);               // Safely stops, logs abort

public:
    ResilienceEngine(int retries = 3);

    // Main entry point — called by WorkflowManager on FAILURE
    int handle(Task* task);
    // Returns 1 if recovered, 0 if aborted
};
```

**Logic inside `handle()`:**
```
1. errorType = detectError(task)

2. If errorType == ERR_FOLDER_NOT_FOUND:
       autoFix(task, ERR_FOLDER_NOT_FOUND)  → creates missing folder
       Return retryTask(task)

3. If errorType == ERR_FILE_NOT_FOUND:
       Return fallback(task)   → tries alternate source path

4. If errorType == ERR_PERMISSION_DENIED:
       Print "Cannot fix permission error"
       Return abort(task)

5. If errorType == ERR_DISK_FULL:
       Print "Disk full — cannot recover"
       Return abort(task)

6. Default:
       Return retryTask(task)  → generic retry
```

---

### 5.8 Logger

**File**: `logger.h` + `logger.cpp`

```cpp
class Logger {
private:
    char logFilePath[128];   // Path to FLOW.LOG
    int  entryCount;         // Total log entries written

public:
    Logger(const char* path = "FLOW.LOG");

    void log(const char* message);   // Appends timestamped entry to log file
    void showLog();                  // Reads and prints FLOW.LOG to screen
    void clearLog();                 // Deletes log file contents
    int  getEntryCount();

    ~Logger();
};
```

**Log Entry Format (inside FLOW.LOG):**
```
[Entry #001] Executing: RenameTask
[Entry #002] SUCCESS:   RenameTask
[Entry #003] Executing: MoveTask
[Entry #004] FAILURE:   MoveTask
[Entry #005] RECOVERED: MoveTask (AutoFix: Folder created)
[Entry #006] Executing: BackupTask
[Entry #007] SUCCESS:   BackupTask
```

---

## 6. Execution Flow

### Full Example: "Organize & Backup Downloads"

```
User selects "Run Workflow"
         │
         ▼
WorkflowManager::executeWorkflow()
         │
         ├──► Task 1: SmartSortTask::execute()
         │         │── SmartSortEngine::getDestination(file)
         │         │── FileTypeRule matches "pdf" → "C:\DOCS\"
         │         │── MoveTask::execute() → SUCCESS ✓
         │         └── Logger logs entry
         │
         ├──► Task 2: RenameTask::execute()
         │         │── rename() called → SUCCESS ✓
         │         └── Logger logs entry
         │
         ├──► Task 3: CreateFolderTask::execute()
         │         │── mkdir() → folder exists → SKIPPED →
         │         └── Logger logs entry
         │
         ├──► Task 4: MoveTask::execute()
         │         │── rename() fails → FAILURE ✗
         │         │── ResilienceEngine::handle() called
         │         │     ├── detectError() → ERR_FOLDER_NOT_FOUND
         │         │     ├── autoFix() → mkdir(destPath)
         │         │     └── retryTask() → SUCCESS ✓ (Recovered)
         │         └── Logger logs entry
         │
         ├──► Task 5: BackupTask::execute()
         │         │── fopen/fwrite → SUCCESS ✓
         │         └── Logger logs entry
         │
         └──► Task 6: VerifyTask::execute()
                   │── access() + file size check → SUCCESS ✓
                   └── Logger logs entry

══════════════════════════════
  ✅  WORKFLOW COMPLETED
══════════════════════════════
```

---

## 7. OOP Concepts Mapping

| OOP Concept | Where Used in FlowForge |
| :--- | :--- |
| **Abstraction** | `Task` (abstract) hides execution details behind `execute()`. `Rule` (abstract) hides matching logic behind `matches()`. |
| **Inheritance** | `RenameTask`, `MoveTask`, `BackupTask`, `VerifyTask`, `CreateFolderTask`, `SmartSortTask` all inherit from `Task`. `FileTypeRule`, `SizeRule`, `AgeRule` inherit from `Rule`. |
| **Polymorphism** | `WorkflowManager` calls `task->execute()` on an array of `Task*` pointers. `SmartSortEngine` calls `rule->matches()` on an array of `Rule*` pointers. Different behaviors, same interface. |
| **Encapsulation** | All class data members are `private`. Accessed through `public` getter/setter methods. `FileInfo` details are contained within tasks. |
| **Constructors** | Every class has a constructor that initializes its members properly (e.g. `RenameTask(FileInfo f, const char* newName)` initializes both `file` and `newName`). |
| **Destructors** | `WorkflowManager`, `SmartSortEngine` use virtual destructors to free all dynamically allocated `Task*` and `Rule*` objects. |
| **File Handling** | `Logger` writes to `FLOW.LOG`. `WorkflowManager` saves/loads from `WORKFLOW.DAT`. `BackupTask` does byte-by-byte file copy using `fopen`/`fread`/`fwrite`. |
| **Error Handling** | `ResilienceEngine` detects error types and executes recovery strategies. `TaskResult` enum (`SUCCESS`/`FAILURE`/`SKIPPED`) propagates outcomes. |

---

## 8. Turbo C++ Specific Notes

> [!IMPORTANT]
> These are critical differences from modern C++. Read before writing any code.

### Headers to Use
| Purpose | Turbo C++ Header |
| :--- | :--- |
| Console I/O | `<conio.h>` |
| String functions | `<string.h>` |
| File operations | `<stdio.h>` |
| Directory operations | `<dir.h>` |
| File access check | `<io.h>` |
| Math functions | `<math.h>` |
| Standard utilities | `<stdlib.h>` |

> **Do NOT use**: `<iostream>`, `<string>`, `<vector>`, `<filesystem>`, `<fstream>` — these are modern C++ headers not fully supported in Turbo C++.  
> **Use instead**: `<iostream.h>`, `<fstream.h>` (note the `.h` extension).

### Filename Rules
- Keep filenames to **8 characters max** (e.g., `WORKFLOW.DAT`, `FLOW.LOG`).
- No spaces in filenames or paths in DOS mode.
- Use `\` as path separator in strings (escaped as `\\` in C++): e.g., `"C:\\DOCS\\"`.

### No `std::` Namespace
Turbo C++ does not use `std::`. Write:
```cpp
// Correct for Turbo C++:
cout << "Hello";
cin  >> input;

// NOT:
std::cout << "Hello";
```

### Dynamic Arrays — Use Pointer Arrays
Since `std::vector` is not available:
```cpp
Task* taskList[20];   // Array of 20 Task pointers
int   taskCount = 0;

// Add a task:
taskList[taskCount++] = new RenameTask(file, "newname.txt");

// Execute all tasks:
for (int i = 0; i < taskCount; i++) {
    taskList[i]->execute();
}

// Free memory:
for (int i = 0; i < taskCount; i++) {
    delete taskList[i];
}
```

### Virtual Functions — Required for Polymorphism
Turbo C++ supports virtual functions. Always use `virtual` in the base class and `= 0` for pure virtual:
```cpp
virtual TaskResult execute() = 0;   // Pure virtual ✓
```

---

## 9. UI Design (conio.h)

The main interface uses `<conio.h>` for a colorful text menu.

### Color Scheme

| Element | Text Color | Background |
| :--- | :--- | :--- |
| Header bar | `YELLOW` | `BLUE` |
| Menu options | `WHITE` | `BLACK` |
| Success messages | `GREEN` | `BLACK` |
| Failure messages | `RED` | `BLACK` |
| Recovery messages | `CYAN` | `BLACK` |
| Log entries | `LIGHTGRAY` | `BLACK` |

### Main Menu Layout
```
╔══════════════════════════════════════════╗
║          F L O W F O R G E              ║
║   Workflow Automation & Smart Sort       ║
╠══════════════════════════════════════════╣
║  1. Create New Workflow                  ║
║  2. View Workflows                       ║
║  3. Execute Workflow                     ║
║  4. Manage SmartSort Rules               ║
║  5. View Execution Log                   ║
║  6. Clear Log                            ║
║  0. Exit                                 ║
╚══════════════════════════════════════════╝
Enter choice:
```

### Key `conio.h` Functions to Use
```cpp
clrscr();                          // Clear screen
textcolor(YELLOW);                 // Set text color
textbackground(BLUE);              // Set background color
gotoxy(col, row);                  // Move cursor to position
cprintf("text");                   // Print with current color
getch();                           // Get single keypress (no Enter needed)
```

---

## 10. Task Checklist

### ✅ Foundation (To be completed first — by project lead)

- [ ] Create `fileinfo.h` — define `FileInfo` struct and `TaskResult` enum
- [ ] Create `task.h` — define abstract `Task` base class with pure virtual `execute()` and `describe()`
- [ ] Create `rule.h` — define abstract `Rule` base class with pure virtual `matches()` and `describe()`
- [ ] Create `logger.h` + `logger.cpp` — Logger class skeleton (just file open/close + stub `log()`)
- [ ] Create `ui.h` + `ui.cpp` — `drawBox()`, `printHeader()`, `printMenu()`, `printSuccess()`, `printError()` helper functions using `<conio.h>`
- [ ] Create `main.cpp` — main menu loop with placeholder case handlers (`cout << "Coming soon..."`)
- [ ] Ensure full project **compiles without errors** before distributing

---

### 📋 To Do

**Task Engine**
- [ ] Implement `RenameTask::execute()` — use `rename()` from `<stdio.h>`; handle virtual mode
- [ ] Implement `RenameTask::describe()` — print old → new name
- [ ] Implement `CreateFolderTask::execute()` — use `mkdir()` from `<dir.h>`; handle "already exists"
- [ ] Implement `CreateFolderTask::describe()`
- [ ] Implement `MoveTask::execute()` — use `rename()` across paths
- [ ] Implement `MoveTask::describe()`
- [ ] Implement `BackupTask::execute()` — byte-by-byte file copy using `fopen`/`fread`/`fwrite`
- [ ] Implement `BackupTask::describe()`
- [ ] Implement `VerifyTask::execute()` — check file exists with `access()` and size > 0
- [ ] Implement `VerifyTask::describe()`
- [ ] Implement `SmartSortTask::execute()` — query `SmartSortEngine`, then call `MoveTask`
- [ ] Implement `SmartSortTask::describe()`

**SmartSort Rules Engine**
- [ ] Implement `FileTypeRule::matches()` — compare `file.extension` with target extension using `strcmp()`
- [ ] Implement `FileTypeRule::describe()`
- [ ] Implement `SizeRule::matches()` — compare `file.sizeBytes >= minSizeBytes`
- [ ] Implement `SizeRule::describe()`
- [ ] Implement `AgeRule::matches()` — compare `file.ageDays >= minAgeDays`
- [ ] Implement `AgeRule::describe()`
- [ ] Implement `SmartSortEngine::addRule()` — append to `rules[]` array
- [ ] Implement `SmartSortEngine::getDestination()` — loop through rules, return first match
- [ ] Implement `SmartSortEngine::showAllRules()` — print all rules using `describe()`
- [ ] Implement `SmartSortEngine::clearRules()` — delete all rule pointers and reset count
- [ ] Implement `SmartSortEngine::~SmartSortEngine()` destructor

**Workflow Manager**
- [ ] Implement `WorkflowManager::addTask()` — append task to `taskList[]`
- [ ] Implement `WorkflowManager::removeTask()` — remove task at index, shift array
- [ ] Implement `WorkflowManager::showTasks()` — print all tasks with numbering
- [ ] Implement `WorkflowManager::executeWorkflow()` — main execution loop with recovery integration
- [ ] Implement `WorkflowManager::saveToFile()` — write workflow name + task names to `WORKFLOW.DAT`
- [ ] Implement `WorkflowManager::loadFromFile()` — read and reconstruct workflow from `WORKFLOW.DAT`
- [ ] Implement `WorkflowManager::~WorkflowManager()` destructor — free all task pointers

**Resilience Engine**
- [ ] Implement `ResilienceEngine::detectError()` — inspect task state, return `ErrorType`
- [ ] Implement `ResilienceEngine::retryTask()` — call `task->execute()` up to `maxRetries` times
- [ ] Implement `ResilienceEngine::autoFix()` — create missing folder for `ERR_FOLDER_NOT_FOUND`
- [ ] Implement `ResilienceEngine::fallback()` — attempt alternate path or skip gracefully
- [ ] Implement `ResilienceEngine::abort()` — log abort and return 0
- [ ] Implement `ResilienceEngine::handle()` — top-level recovery dispatcher

**Logger**
- [ ] Implement `Logger::log()` — append formatted entry to `FLOW.LOG` using `fopen` (append mode)
- [ ] Implement `Logger::showLog()` — read `FLOW.LOG` line by line and print to screen
- [ ] Implement `Logger::clearLog()` — open `FLOW.LOG` in write mode (truncates it)
- [ ] Implement `Logger::~Logger()` destructor

**Main Menu & UI**
- [ ] Implement UI helpers: `drawBox()`, `printHeader()`, `printSuccess()`, `printError()`, `printRecovery()`
- [ ] Wire up Menu Option 1: Create New Workflow (prompt name, add tasks interactively)
- [ ] Wire up Menu Option 2: View Workflows (show saved workflow names and tasks)
- [ ] Wire up Menu Option 3: Execute Workflow (run selected workflow, show live step-by-step output)
- [ ] Wire up Menu Option 4: Manage SmartSort Rules (add/remove rules interactively)
- [ ] Wire up Menu Option 5: View Execution Log (show FLOW.LOG contents)
- [ ] Wire up Menu Option 6: Clear Log

**Testing**
- [ ] Test RenameTask with a real file and a virtual file
- [ ] Test MoveTask — trigger `ERR_FOLDER_NOT_FOUND` and verify auto-fix works
- [ ] Test SmartSort with 5 different file types (pdf, jpg, mp4, mp3, zip)
- [ ] Test BackupTask + VerifyTask as a pair
- [ ] Test full "Organize and Backup Downloads" workflow end-to-end
- [ ] Test full "Assignment Management" workflow end-to-end
- [ ] Verify FLOW.LOG has correct entries after each run
- [ ] Test saveToFile + loadFromFile round-trip

---

*FlowForge — Built with C++ and OOP principles.*
