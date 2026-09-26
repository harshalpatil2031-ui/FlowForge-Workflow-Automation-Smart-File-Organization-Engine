# 🏛️ FlowForge — Master Technical Specification & Architecture Blueprint

<p align="center">
  <b>FlowForge: Workflow Automation, Smart File Organization & Resilience Engine</b><br>
  <i>Master Project Blueprint & Implementation Guide for Turbo C++</i>
</p>

---

## 📑 Document Metadata

| Attribute | Specification |
| :--- | :--- |
| **Document Version** | `v1.0.0-master` |
| **Target Compiler** | Turbo C++ 3.0 / 3.1 (Borland 16-bit C++) |
| **Language Standard** | C++98 / Classic C++ with Borland Extensions |
| **Architecture Pattern** | Command Pattern + Strategy Pattern + Resilience State Machine |
| **Document Scope** | Complete End-to-End System Blueprint for All Team Modules |

---

## 1. System Vision & Complete Problem Statement

### 1.1 Problem Statement
In daily computer usage, users perform repetitive, multi-step file operations:
- Sorting mixed downloads (`.pdf`, `.jpg`, `.mp4`, `.zip`) into organized folders.
- Standardizing filename formats for academic assignments.
- Creating course-specific directories and organizing files by type, size, or age.
- Creating file backups and verifying backup integrity.

Existing file managers execute individual operations (copy, move, rename) in isolation. They lack the ability to group these operations into automated, reusable workflows with **intelligent error handling and self-recovery**.

### 1.2 Proposed Solution
**FlowForge** is a complete workflow automation engine combining three foundational systems:

1. **Workflow Engine**: Combines multiple file tasks into a single executable, reusable pipeline.
2. **SmartSort Engine**: Rule-based automatic file classification by extension, size, or creation age.
3. **Resilience Engine**: Automated failure detection, directory auto-creation, retry execution, and fallback strategies.

> [!NOTE]
> **Core Mathematical Model**:  
> $$\text{FlowForge} = \text{Workflow Engine} \cup \text{SmartSort Engine} \cup \text{Resilience Engine}$$

---

## 2. Master System Architecture

```text
┌────────────────────────────────────────────────────────────────────────┐
│                              FLOWFORG.CPP                              │
│                      (Console UI & Main Menu)                          │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │
                    ┌───────────────▼───────────────┐
                    │        WorkflowManager        │
                    │    Pipeline Orchestrator      │
                    └───────────────┬───────────────┘
                                    │
          ┌─────────────────────────┼─────────────────────────┐
          │                         │                         │
┌─────────▼─────────┐     ┌─────────▼─────────┐     ┌─────────▼─────────┐
│    Task Engine    │     │  SmartSort Engine │     │ Resilience Engine │
│ Abstract `Task`   │     │ Abstract `Rule`   │     │ Exception & Auto  │
│ Base Class &      │     │ Base Class &      │     │ Recovery State    │
│ Concrete Tasks    │     │ Rule Evaluator    │     │ Machine           │
└───────────────────┘     └───────────────────┘     └───────────────────┘
          │                         │                         │
          └─────────────────────────┼─────────────────────────┘
                                    │
                    ┌───────────────▼───────────────┐
                    │            Logger             │
                    │    Persistent Audit Stream    │
                    └───────────────────────────────┘
```

---

## 3. Comprehensive Subsystem Specifications

### 3.1 Module 1: Shared Core & Data Models (`fileinfo.h`)

Every component operates on the `FileInfo` data structure and outcome enumerations:

```cpp
struct FileInfo {
    char name[64];         // Original filename (e.g. "assignment.pdf")
    char extension[10];    // File extension (e.g. "pdf")
    char sourcePath[128];  // Origin directory path (e.g. "C:\\DOWNLOADS\\")
    char destPath[128];    // Target directory path (e.g. "C:\\DOCUMENTS\\")
    long sizeBytes;        // File size in bytes
    int  ageDays;          // Creation age in days
    int  isVirtual;        // 1 = Virtual sandbox file, 0 = Physical DOS file
};

enum TaskResult { SUCCESS, FAILURE, SKIPPED };
enum ErrorType  { ERR_FOLDER_NOT_FOUND, ERR_FILE_NOT_FOUND, ERR_PERMISSION_DENIED, ERR_UNKNOWN };
```

---

### 3.2 Module 2: Task Engine Subsystem

The Task Engine implements the **Command Pattern**. All tasks inherit from the abstract `Task` base class.

```cpp
class Task {
protected:
    char taskName[64];
    FileInfo file;
    int maxRetries;
    int retryCount;

public:
    Task(const char* name, FileInfo f, int retries = 2);
    
    // Pure Virtual Interfaces
    virtual TaskResult execute() = 0;
    virtual void describe() = 0;

    const char* getTaskName();
    FileInfo    getFile();
    virtual ~Task();
};
```

#### Detailed Specification of All Concrete Tasks:

1. **`RenameTask`**:
   - *Purpose*: Renames `file.name` to `newName`.
   - *Logic*: Builds full source and destination paths using `strcpy` and `strcat`. In Virtual mode (`isVirtual == 1`), updates `file.name` in memory. In Physical mode, calls `rename(srcFull, destFull)` from `<stdio.h>`. Returns `SUCCESS` or `FAILURE`.

2. **`MoveTask`**:
   - *Purpose*: Relocates a file from `file.sourcePath` to `file.destPath`.
   - *Logic*: Constructs full source path (`sourcePath + name`) and target path (`destPath + name`). Calls `rename(srcFull, destFull)` across directory paths. If target directory does not exist, returns `FAILURE` to trigger the Resilience Engine.

3. **`CreateFolderTask`**:
   - *Purpose*: Ensures target directories exist before file transfer.
   - *Logic*: Calls `mkdir(folderPath)` from `<dir.h>`. If directory already exists, returns `SKIPPED`. If successfully created, returns `SUCCESS`. If permission/path error occurs, returns `FAILURE`.

4. **`BackupTask`**:
   - *Purpose*: Creates a duplicate backup file with a `_BAK` suffix.
   - *Logic*: Opens source file in binary read mode (`"rb"`) and backup destination file in binary write mode (`"wb"`). Performs chunked stream copying via buffer. Returns `SUCCESS` if file copied completely.

5. **`VerifyTask`**:
   - *Purpose*: Confirms backup integrity after copy operations.
   - *Logic*: Checks file existence using `access()` from `<io.h>` and verifies `filelength() > 0`. Returns `SUCCESS` if file exists and is non-empty; returns `FAILURE` otherwise.

6. **`SmartSortTask`**:
   - *Purpose*: Dynamically classifies a file and moves it to its categorized folder.
   - *Logic*: Queries `SmartSortEngine::getDestination(file)`. Sets `file.destPath` to matched destination, then delegates execution to `MoveTask(file).execute()`.

---

### 3.3 Module 3: SmartSort Rules Subsystem

The SmartSort Engine implements the **Strategy Pattern**. Rules evaluate file conditions dynamically.

```cpp
class Rule {
protected:
    char destinationFolder[128];

public:
    Rule(const char* destFolder);

    virtual int matches(FileInfo file) = 0;
    virtual void describe() = 0;

    const char* getDestination();
    virtual ~Rule();
};
```

#### Detailed Specification of All Concrete Rules:

1. **`FileTypeRule`**:
   - *Logic*: Compares `file.extension` against rule extension using `strcmp()`. Matches `.pdf` $\rightarrow$ Documents, `.jpg` $\rightarrow$ Pictures, `.mp4` $\rightarrow$ Videos, `.zip` $\rightarrow$ Archives.

2. **`SizeRule`**:
   - *Logic*: Compares `file.sizeBytes >= minSizeBytes`. Matches files exceeding size threshold (e.g., files $> 100\text{ MB}$) $\rightarrow$ Large Files Folder.

3. **`AgeRule`**:
   - *Logic*: Compares `file.ageDays >= minAgeDays`. Matches old files (e.g., files $> 30\text{ days}$) $\rightarrow$ Archive Folder.

4. **`SmartSortEngine`**:
   - *Logic*: Manages array `Rule* rules[20]`. Method `getDestination(file)` scans active rules sequentially and returns `rule->getDestination()` for the first rule returning `matches(file) == 1`.

---

### 3.4 Module 4: Resilience & Recovery Subsystem

The `ResilienceEngine` implements automated self-recovery for failed tasks.

```cpp
class ResilienceEngine {
private:
    int maxRetries;

    ErrorType detectError(Task* task);
    int retryTask(Task* task);
    int autoFix(Task* task, ErrorType e);
    int fallback(Task* task);
    int abort(Task* task);

public:
    ResilienceEngine(int retries = 3);
    int handle(Task* task); // Main recovery dispatcher
};
```

#### Resilience Recovery State Machine Algorithm:

```text
ALGORITHM: ResilienceEngine::handle(Task* task)
-----------------------------------------------
1. Determine ErrorType = detectError(task)

2. SWITCH ErrorType:
     CASE ERR_FOLDER_NOT_FOUND:
          a. Call autoFix(task, ERR_FOLDER_NOT_FOUND)
             -> Executes mkdir(task.file.destPath) to create missing directory
          b. Call retryTask(task) -> Re-executes task.execute()
          c. IF retry succeeded -> RETURN 1 (Recovered)
          d. ELSE               -> RETURN 0 (Abort)

     CASE ERR_FILE_NOT_FOUND:
          a. Call fallback(task) -> Attempts alternative source directory
          b. IF fallback succeeded -> RETURN 1 (Recovered)
          c. ELSE                  -> RETURN 0 (Abort)

     DEFAULT:
          a. WHILE task.retryCount < maxRetries:
               - Increment task.retryCount
               - IF task.execute() == SUCCESS -> RETURN 1 (Recovered)
          b. Call abort(task) -> RETURN 0 (Abort)
```

---

### 3.5 Module 5: Logger Subsystem

The `Logger` class maintains a persistent, timestamped audit log of all system events.

```cpp
class Logger {
private:
    char logFilePath[128];
    int entryCount;

public:
    Logger(const char* path = "FLOW.LOG");
    void log(const char* message);   // Appends timestamped log entry ("a" mode)
    void showLog();                  // Reads and displays log file ("r" mode)
    void clearLog();                 // Resets log file contents ("w" mode)
    int  getEntryCount();
    ~Logger();
};
```

---

### 3.6 Module 6: Workflow Orchestrator & Persistence

The `WorkflowManager` acts as the pipeline orchestrator, connecting tasks, logger, and resilience handling.

```cpp
class WorkflowManager {
private:
    char workflowName[64];
    Task* taskList[20];
    int taskCount;
    Logger logger;

public:
    WorkflowManager(const char* name);
    void addTask(Task* task);
    void removeTask(int index);
    void showTasks();
    void executeWorkflow();
    void saveToFile(const char* path);  // Saves workflow data to WORKFLOW.DAT
    void loadFromFile(const char* path); // Loads workflow data from WORKFLOW.DAT
    const char* getName();
    int getTaskCount();
    ~WorkflowManager(); // Deletes dynamically allocated task pointers
};
```

---

## 4. End-to-End Workflow Execution Trace

### Example Workflow: "Organize & Backup Downloads"

```text
User triggers: WorkflowManager::executeWorkflow()
  │
  ├──► Step 1: SmartSortTask::execute()
  │         ├── Query SmartSortEngine for "assignment.pdf"
  │         ├── FileTypeRule matches extension "pdf" -> target "C:\DOCS\"
  │         ├── MoveTask::execute() -> SUCCESS ✓
  │         └── Logger logs: "SUCCESS: SmartSortTask"
  │
  ├──► Step 2: CreateFolderTask::execute()
  │         ├── Check directory "C:\DOCS\BACKUP\"
  │         ├── Folder already exists -> SKIPPED →
  │         └── Logger logs: "SKIPPED: CreateFolderTask"
  │
  ├──► Step 3: MoveTask::execute()
  │         ├── Relocate file to "C:\DOCS\BACKUP\"
  │         ├── Target folder missing -> FAILURE ✗
  │         ├── Trigger ResilienceEngine::handle(MoveTask)
  │         │     ├── detectError() -> ERR_FOLDER_NOT_FOUND
  │         │     ├── autoFix() -> mkdir("C:\DOCS\BACKUP\")
  │         │     └── retryTask() -> SUCCESS ✓ (RECOVERED)
  │         └── Logger logs: "RECOVERED: MoveTask"
  │
  ├──► Step 4: BackupTask::execute()
  │         ├── Perform binary copy to "assignment_BAK.pdf" -> SUCCESS ✓
  │         └── Logger logs: "SUCCESS: BackupTask"
  │
  └──► Step 5: VerifyTask::execute()
            ├── Verify existence & byte size (> 0 bytes) -> SUCCESS ✓
            └── Logger logs: "SUCCESS: VerifyTask"

============================================================
           ✅ WORKFLOW COMPLETED SUCCESSFULLY
============================================================
```

---

## 5. Comprehensive Academic OOP Mapping

| Course Syllabus Unit | Project Component & Class | Specific Implementation Mechanism |
| :--- | :--- | :--- |
| **Unit 1: OOP Fundamentals** | Abstract `Task` & `Rule` classes | Pure virtual functions (`= 0`) establishing system contracts. |
| **Unit 2: Classes & Objects** | All System Classes | Encapsulated private attributes, access modifiers, object parameters. |
| **Unit 3: Constructors & Destructors**| `Task`, `Rule`, `WorkflowManager` | Constructor initialization lists, default arguments, virtual destructors. |
| **Unit 4: Inheritance & Polymorphism**| Derived Tasks & Rules | Single public inheritance (`class RenameTask : public Task`), dynamic binding via `Task* taskList[20]`. |
| **Unit 5: File Handling** | `Logger` Subsystem | File stream modes (`"a"`, `"r"`, `"w"`), `fopen()`, `fprintf()`, `fgets()`, `fclose()`. |
| **Unit 6: Exception & Resilience** | `ResilienceEngine` | Error classification, auto-fix strategy dispatch, retry loops, fallback paths. |

---

## 6. Turbo C++ Environment Constraints & Engineering Solutions

1. **16-Bit Memory Limits (64 KB Segment)**: Pointer arrays (`Task* taskList[20]`) are statically bounded to prevent memory heap overflow.
2. **Virtual Sandbox Mode**: File struct includes `isVirtual` flag (`1` = virtual simulation, `0` = real DOS file) so workflow logic can be demonstrated cleanly even in restricted DOSBox environments.
3. **Borland Console Integration**: Interface utilizes `<conio.h>` utilities (`clrscr()`, `gotoxy()`, `textcolor()`, `getch()`) for text UI formatting.

---

*FlowForge Master Blueprint — Built for Object-Oriented Programming Course Evaluation.*
