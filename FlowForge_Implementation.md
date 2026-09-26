# 🏛️ FlowForge — Master Technical Specification & Architecture Blueprint

<p align="center">
  <b>FlowForge: Workflow Automation, Smart File Organization & Resilience Engine</b><br>
  <i>Master Project Blueprint & Architectural Specification</i>
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

## 1. System Vision & Problem Statement

### 1.1 Problem Statement
Daily computer operations involve repetitive, multi-step file manipulation tasks:
- Sorting mixed downloads (`.pdf`, `.jpg`, `.mp4`, `.zip`) into organized folders.
- Standardizing filename formats for academic assignments.
- Creating course-specific directories and organizing files by type, size, or age.
- Creating file backups and verifying backup integrity.

Standard file managers perform isolated operations (copy, move, rename) independently. They lack the ability to group these operations into automated, reusable workflows with **intelligent error handling and self-recovery**.

### 1.2 Proposed Solution
**FlowForge** unifies three core capabilities into a single automated engine:

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

## 3. Core Architecture & Foundation Code (Team Lead Scope)

Below are the foundational data structures and base contracts that form the backbone of FlowForge.

### 3.1 Shared Core Data Structures (`FileInfo` & Enums)

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

### 3.2 Abstract Task Interface

```cpp
class Task {
protected:
    char taskName[64];
    FileInfo file;
    int maxRetries;
    int retryCount;

public:
    Task(const char* name, FileInfo f, int retries = 2) {
        strcpy(taskName, name);
        file = f;
        maxRetries = retries;
        retryCount = 0;
    }

    virtual TaskResult execute() = 0;
    virtual void describe() = 0;

    const char* getTaskName() { return taskName; }
    FileInfo getFile() { return file; }
    virtual ~Task() {}
};
```

---

### 3.3 Abstract Rule Interface

```cpp
class Rule {
protected:
    char destinationFolder[128];

public:
    Rule(const char* destFolder) {
        strcpy(destinationFolder, destFolder);
    }

    virtual int matches(FileInfo file) = 0;
    virtual void describe() = 0;

    const char* getDestination() { return destinationFolder; }
    virtual ~Rule();
};
```

---

### 3.4 Logger Subsystem

```cpp
class Logger {
private:
    char logFilePath[128];
    int entryCount;

public:
    Logger(const char* path = "FLOW.LOG") {
        strcpy(logFilePath, path);
        entryCount = 0;
    }

    void log(const char* message) {
        FILE* f = fopen(logFilePath, "a");
        if (f != NULL) {
            entryCount++;
            fprintf(f, "[Entry #%03d] %s\n", entryCount, message);
            fclose(f);
        }
    }

    void showLog() {
        char line[256];
        FILE* f = fopen(logFilePath, "r");
        if (f == NULL) return;
        while (fgets(line, sizeof(line), f) != NULL) {
            cout << line;
        }
        fclose(f);
    }

    void clearLog() {
        FILE* f = fopen(logFilePath, "w");
        if (f != NULL) {
            fclose(f);
            entryCount = 0;
        }
    }

    ~Logger() {}
};
```

---

### 3.5 Workflow Orchestrator (`WorkflowManager`)

```cpp
class WorkflowManager {
private:
    char workflowName[64];
    Task* taskList[20];
    int taskCount;
    Logger logger;

public:
    WorkflowManager(const char* name) : logger("FLOW.LOG") {
        strcpy(workflowName, name);
        taskCount = 0;
    }

    void addTask(Task* task) {
        if (taskCount < 20) taskList[taskCount++] = task;
    }

    void executeWorkflow() {
        for (int i = 0; i < taskCount; i++) {
            logger.log(taskList[i]->getTaskName());
            TaskResult res = taskList[i]->execute();
            // Routes to ResilienceEngine on FAILURE
        }
    }

    ~WorkflowManager() {
        for (int i = 0; i < taskCount; i++) delete taskList[i];
    }
};
```

---

## 4. Teammate Subsystem Logic & Algorithms (To Be Implemented)

All tasks, rules, and recovery engines below are specified via step-by-step algorithms for collaborators to implement.

### 4.1 Task Engine Algorithms

1. **`RenameTask` Algorithm**:
   ```text
   ALGORITHM: RenameTask::execute()
   1. IF file.isVirtual == 1 THEN:
        - Update file.name = newName in memory
        - RETURN SUCCESS
   2. Build srcFull = file.sourcePath + file.name
   3. Build destFull = file.sourcePath + newName
   4. Call rename(srcFull, destFull) from <stdio.h>
   5. IF rename succeeded -> RETURN SUCCESS
   6. ELSE -> RETURN FAILURE
   ```

2. **`MoveTask` Algorithm**:
   ```text
   ALGORITHM: MoveTask::execute()
   1. Build srcFull = file.sourcePath + file.name
   2. Build destFull = file.destPath + file.name
   3. Call rename(srcFull, destFull)
   4. IF successful -> RETURN SUCCESS
   5. ELSE (e.g., target directory missing) -> RETURN FAILURE
   ```

3. **`CreateFolderTask` Algorithm**:
   ```text
   ALGORITHM: CreateFolderTask::execute()
   1. Call mkdir(folderPath) from <dir.h>
   2. IF folder created successfully -> RETURN SUCCESS
   3. ELSE IF folder already exists -> RETURN SKIPPED
   4. ELSE -> RETURN FAILURE
   ```

4. **`BackupTask` Algorithm**:
   ```text
   ALGORITHM: BackupTask::execute()
   1. Open source file in binary read mode ("rb")
   2. Open backup destination file in binary write mode ("wb")
   3. Copy file data using stream buffer chunks
   4. Close both file streams
   5. RETURN SUCCESS
   ```

5. **`VerifyTask` Algorithm**:
   ```text
   ALGORITHM: VerifyTask::execute()
   1. Check file existence using access() from <io.h>
   2. Check file size > 0 bytes
   3. IF both pass -> RETURN SUCCESS
   4. ELSE -> RETURN FAILURE
   ```

---

### 4.2 SmartSort Rules Algorithms

1. **`FileTypeRule::matches()`**:
   ```text
   ALGORITHM: FileTypeRule::matches(file)
   - Perform string comparison: strcmp(file.extension, targetExtension)
   - IF extension matches -> RETURN 1 (True)
   - ELSE -> RETURN 0 (False)
   ```

2. **`SizeRule::matches()`**:
   ```text
   ALGORITHM: SizeRule::matches(file)
   - IF file.sizeBytes >= minSizeBytes -> RETURN 1 (True)
   - ELSE -> RETURN 0 (False)
   ```

3. **`AgeRule::matches()`**:
   ```text
   ALGORITHM: AgeRule::matches(file)
   - IF file.ageDays >= minAgeDays -> RETURN 1 (True)
   - ELSE -> RETURN 0 (False)
   ```

4. **`SmartSortEngine::getDestination()`**:
   ```text
   ALGORITHM: SmartSortEngine::getDestination(file)
   - FOR EACH rule IN rules array:
        - IF rule.matches(file) == 1 -> RETURN rule.getDestination()
   - RETURN default destination ("Unsorted")
   ```

---

### 4.3 Resilience Engine Recovery Algorithm

```text
ALGORITHM: ResilienceEngine::handle(task)
------------------------------------------
1. Determine ErrorType:
   - If dest directory is missing -> ERR_FOLDER_NOT_FOUND
   - If file does not exist      -> ERR_FILE_NOT_FOUND
   - Otherwise                  -> ERR_UNKNOWN

2. SWITCH ErrorType:
     CASE ERR_FOLDER_NOT_FOUND:
          - Execute autoFix(): mkdir(task.file.destPath)
          - Execute retryTask(): task.execute()
          - IF retry succeeded -> RETURN 1 (Recovered)
          - ELSE -> RETURN 0 (Abort)

     CASE ERR_FILE_NOT_FOUND:
          - Execute fallback(): try alternate directory path
          - IF fallback succeeded -> RETURN 1 (Recovered)
          - ELSE -> RETURN 0 (Abort)

     DEFAULT:
          - WHILE retryCount < maxRetries: re-execute task
          - IF succeeded -> RETURN 1 (Recovered)
          - ELSE -> RETURN 0 (Abort)
```

---

## 5. End-to-End Workflow Execution Trace

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

## 6. Comprehensive Academic OOP Mapping

| Course Syllabus Unit | Project Component & Class | Specific Implementation Mechanism |
| :--- | :--- | :--- |
| **Unit 1: OOP Fundamentals** | Abstract `Task` & `Rule` classes | Pure virtual functions (`= 0`) establishing system contracts. |
| **Unit 2: Classes & Objects** | All System Classes | Encapsulated private attributes, access modifiers, object parameters. |
| **Unit 3: Constructors & Destructors**| `Task`, `Rule`, `WorkflowManager` | Constructor initialization lists, default arguments, virtual destructors. |
| **Unit 4: Inheritance & Polymorphism**| Derived Tasks & Rules | Single public inheritance (`class RenameTask : public Task`), dynamic binding via `Task* taskList[20]`. |
| **Unit 5: File Handling** | `Logger` Subsystem | File stream modes (`"a"`, `"r"`, `"w"`), `fopen()`, `fprintf()`, `fgets()`, `fclose()`. |
| **Unit 6: Exception & Resilience** | `ResilienceEngine` | Error classification, auto-fix strategy dispatch, retry loops, fallback paths. |

---

## 7. Turbo C++ Environment Constraints & Engineering Solutions

1. **16-Bit Memory Limits (64 KB Segment)**: Pointer arrays (`Task* taskList[20]`) are statically bounded to prevent memory heap overflow.
2. **Virtual Sandbox Mode**: File struct includes `isVirtual` flag (`1` = virtual simulation, `0` = real DOS file) so workflow logic can be demonstrated cleanly even in restricted DOSBox environments.
3. **Borland Console Integration**: Interface utilizes `<conio.h>` utilities (`clrscr()`, `gotoxy()`, `textcolor()`, `getch()`) for text UI formatting.

---

*FlowForge Master Blueprint — Built for Object-Oriented Programming Course Evaluation.*
