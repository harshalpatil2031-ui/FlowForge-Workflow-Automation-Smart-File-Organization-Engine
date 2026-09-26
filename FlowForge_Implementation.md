# 🛠️ FlowForge — Technical Architecture & Logic Specification

> **Project Title**: FlowForge — Workflow Automation & Smart File Organization Engine  
> **Target Environment**: Turbo C++ (16-bit Borland C++)  
> **Document Purpose**: System Architecture, Algorithm Specifications, & OOP Design Guide  

---

## 📌 Executive Summary

**FlowForge** is designed as a modular, event-driven engine that automates multi-step file management tasks while maintaining high system resilience.

Instead of performing manual, isolated file manipulations, FlowForge encapsulates individual operations into discrete **Task** units, chains them into a sequential **Workflow**, evaluates files against user-defined **SmartSort Rules**, and routes runtime exceptions to a **Resilience Engine** for automatic self-recovery.

---

## 🏗️ System Architecture & Data Flow

```
                      ┌─────────────────────────┐
                      │    User Interface UI    │
                      │   Interactive Menu Shell│
                      └────────────┬────────────┘
                                   │
                    ┌──────────────▼──────────────┐
                    │       WorkflowManager       │
                    │   Pipeline Orchestrator     │
                    └──────────────┬──────────────┘
                                   │
          ┌────────────────────────┼────────────────────────┐
          │                        │                        │
┌─────────▼─────────┐    ┌─────────▼─────────┐    ┌─────────▼─────────┐
│    Task Engine    │    │  SmartSort Engine │    │ Resilience Engine │
│ Polymorphic Task  │    │ Dynamic Rule      │    │ Error Detection & │
│ Pipeline          │    │ Evaluation        │    │ Recovery Pipeline │
└───────────────────┘    └───────────────────┘    └───────────────────┘
          │                        │                        │
          └────────────────────────┼────────────────────────┘
                                   │
                    ┌──────────────▼──────────────┐
                    │       Logger Subsystem      │
                    │   Persistent Audit Trail    │
                    └─────────────────────────────┘
```

---

## 🧬 OOP Architectural Principles

| OOP Concept | Conceptual Role | System Mechanism |
| :--- | :--- | :--- |
| **Abstraction** | Contract Definition | Base classes `Task` and `Rule` define pure virtual interface signatures (`execute()`, `describe()`, `matches()`), exposing *what* operations do while hiding *how* they are implemented. |
| **Inheritance** | Behavioral Specialization | Concrete task classes (`RenameTask`, `MoveTask`, `BackupTask`, `VerifyTask`, `CreateFolderTask`, `SmartSortTask`) derive from `Task`. Concrete rules (`FileTypeRule`, `SizeRule`, `AgeRule`) derive from `Rule`. |
| **Polymorphism** | Dynamic Execution | `WorkflowManager` holds an array of abstract base pointers (`Task* taskList[20]`). Calling `taskList[i]->execute()` dynamically resolves to the appropriate derived task logic at runtime. |
| **Encapsulation** | State Protection | Internal file states (`FileInfo`), retry counters, and log paths are kept `protected` or `private`, accessible only via public getters and operational methods. |
| **File Handling** | Persistent Audit | The `Logger` subsystem uses C file streams in append (`"a"`), read (`"r"`), and write (`"w"`) modes to maintain `FLOW.LOG`. |

---

## 🧠 Core System Logic & Algorithms

### 1. Workflow Execution Algorithm
The `WorkflowManager` orchestrates task execution sequentially, interacting with the `Logger` and `ResilienceEngine` on failure:

```text
ALGORITHM: ExecuteWorkflow(Workflow)
-----------------------------------
1. FOR EACH task IN Workflow.taskList (from index 0 to taskCount - 1):
     a. Log starting state: Logger.log("Executing: " + task.name)
     b. Print task description to screen
     c. Set status = task.execute()
     
     d. IF status == SUCCESS THEN:
          - Log success: Logger.log("SUCCESS: " + task.name)
          - Continue to next task
          
     e. ELSE IF status == FAILURE THEN:
          - Log failure: Logger.log("FAILURE: " + task.name)
          - Call recovery: recovered = ResilienceEngine.handle(task)
          
          - IF recovered == TRUE THEN:
               - Log recovery: Logger.log("RECOVERED: " + task.name)
               - Continue to next task
          - ELSE:
               - Log abort: Logger.log("ABORTED: " + task.name)
               - TERMINATE workflow execution
               
     f. ELSE IF status == SKIPPED THEN:
          - Log skip: Logger.log("SKIPPED: " + task.name)
          - Continue to next task

2. Log completion: Logger.log("WORKFLOW COMPLETE")
```

---

### 2. Resilience Engine Recovery Strategy Logic
The `ResilienceEngine` evaluates failed operations and selects the appropriate recovery strategy based on error classification:

```text
ALGORITHM: ResilienceHandle(Task)
---------------------------------
1. Inspect Task and system state to determine ErrorType:
   - If destination directory is missing  -> ERR_FOLDER_NOT_FOUND
   - If source file does not exist       -> ERR_FILE_NOT_FOUND
   - Otherwise                           -> ERR_UNKNOWN

2. SWITCH ErrorType:
     CASE ERR_FOLDER_NOT_FOUND:
          a. Execute Auto-Fix: Automatically create missing destination folder (mkdir)
          b. Execute Retry: Re-run Task.execute()
          c. IF Retry succeeded -> RETURN TRUE (Recovered)
          d. ELSE               -> RETURN FALSE (Abort)

     CASE ERR_FILE_NOT_FOUND:
          a. Execute Fallback: Attempt alternative file source path or search directory
          b. IF Fallback succeeded -> RETURN TRUE (Recovered)
          c. ELSE                  -> RETURN FALSE (Abort)

     DEFAULT:
          a. WHILE task.retryCount < task.maxRetries:
               - Increment task.retryCount
               - IF Task.execute() succeeds -> RETURN TRUE
          b. RETURN FALSE (Abort)
```

---

### 3. SmartSort Rule Matching Algorithm
The `SmartSortEngine` evaluates files against a collection of user-defined rules to determine target directory routing:

```text
ALGORITHM: GetSmartSortDestination(FileInfo file)
-------------------------------------------------
1. FOR EACH rule IN SmartSortEngine.rules:
     a. Evaluate: matchStatus = rule.matches(file)
     b. IF matchStatus == TRUE THEN:
          - RETURN rule.getDestination() (First matching rule wins)

2. IF no rule matches:
     - RETURN default destination directory ("Unsorted")
```

#### Specific Rule Evaluation Logic:
- **`FileTypeRule`**: Extracts `file.extension` and performs a case-insensitive string comparison against target extension (e.g., `"pdf"`, `"jpg"`).
- **`SizeRule`**: Compares `file.sizeBytes` against threshold `minSizeBytes`. Returns `TRUE` if file size meets or exceeds threshold.
- **`AgeRule`**: Compares `file.ageDays` against threshold `minAgeDays`. Returns `TRUE` if file age meets or exceeds threshold.

---

### 4. File Copy & Backup Algorithm
For `BackupTask`, binary-safe file copying is performed using chunked stream buffers:

```text
ALGORITHM: PerformFileBackup(SourcePath, DestinationPath)
--------------------------------------------------------
1. IF file.isVirtual == 1 THEN:
     - Simulate creation of "_BAK" file
     - RETURN SUCCESS

2. Open SourcePath in Binary Read mode ("rb")
3. Open DestinationPath in Binary Write mode ("wb")

4. IF either file stream fails to open THEN:
     - RETURN FAILURE (Triggers Resilience Engine)

5. WHILE buffer = Read Chunk from SourceStream:
     - Write buffer Chunk to DestinationStream

6. Close SourceStream and DestinationStream
7. RETURN SUCCESS
```

---

## 📊 Shared Data Model Specifications

### `FileInfo` Attributes
- `name`: Base filename string (e.g., `"assignment.pdf"`).
- `extension`: Extracted file format (e.g., `"pdf"`).
- `sourcePath`: Directory origin (e.g., `"C:\\DOWNLOADS\\"`).
- `destPath`: Directory destination (e.g., `"C:\\DOCUMENTS\\"`).
- `sizeBytes`: File size in bytes.
- `ageDays`: Creation age in days.
- `isVirtual`: Flag toggle (`1` = virtual simulated file for testing, `0` = physical DOS file).

### Task Execution Outcomes
- `SUCCESS`: Task completed without errors.
- `FAILURE`: Operation failed; routes execution to Resilience Engine.
- `SKIPPED`: Operation bypassed safely (e.g., destination directory already exists).

---

## 🛠️ Turbo C++ Implementation Guidelines

1. **Memory Allocation**: Use explicit dynamic allocation (`new`) when adding tasks to `WorkflowManager` and rules to `SmartSortEngine`. Ensure destructors clean up heap memory using `delete`.
2. **String Operations**: Rely on `<string.h>` utilities (`strcpy`, `strcmp`, `strcat`) for string manipulation rather than modern C++ `std::string`.
3. **Console Interface**: Utilize `<conio.h>` functions (`clrscr()`, `getch()`, `gotoxy()`) for screen management and input handling in the main menu loop.
