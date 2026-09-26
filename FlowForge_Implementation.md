# 🏛️ FlowForge — Technical Specification & Architecture Blueprint

<p align="center">
  <b>FlowForge: Workflow Automation, Smart File Organization & Resilience Engine</b><br>
  <i>Object-Oriented Programming Course Project Specification</i>
</p>

---

## 📑 Document Metadata

| Attribute | Specification |
| :--- | :--- |
| **Document Version** | `v1.0.0-release` |
| **Target Compiler** | Turbo C++ 3.0 / 3.1 (Borland 16-bit C++) |
| **Language Standard** | C++98 / Classic C++ with Borland Extensions |
| **Architecture Pattern** | Command Pattern + Strategy Pattern + Chain of Recovery |
| **Primary Domain** | System Utilities & Automated File Management |

---

## 1. System Overview & Problem Statement

### 1.1 Problem Statement
Daily computer operations involve repetitive, multi-step file manipulation tasks:
- Sorting cluttered `Downloads` directories.
- Renaming assignments to standardized naming conventions.
- Creating subject-specific directories and organizing files by type/size/age.
- Generating file backups and verifying backup integrity.

Standard file managers perform isolated operations (copy, move, rename) independently. They lack the capability to compose these operations into reusable, automated workflows equipped with **fault tolerance and self-recovery**.

### 1.2 Proposed Solution
**FlowForge** unifies three core capabilities into a single automated engine:

1. **Workflow Automation**: Group sequential file operations into a single reusable object pipeline.
2. **SmartSort Engine**: Rule-driven file categorization based on file extensions, size thresholds, or creation age.
3. **Resilience Engine**: Automated error detection, auto-fix path creation, retry handling, and fallback execution.

> [!NOTE]
> **Core Mathematical Model**:  
> $$\text{FlowForge} = \text{Workflow Engine} \cup \text{SmartSort Classifier} \cup \text{Resilience Engine}$$

---

## 2. Architectural Design & Component Overview

```text
┌────────────────────────────────────────────────────────────────────────┐
│                              FLOWFORG.CPP                              │
│                 (User Interface & Menu Dispatcher)                     │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │
                    ┌───────────────▼───────────────┐
                    │        WorkflowManager        │
                    │   Pipeline Orchestrator       │
                    └───────────────┬───────────────┘
                                    │
          ┌─────────────────────────┼─────────────────────────┐
          │                         │                         │
┌─────────▼─────────┐     ┌─────────▼─────────┐     ┌─────────▼─────────┐
│    Task Engine    │     │  SmartSort Engine │     │ Resilience Engine │
│  Abstract `Task`  │     │  Abstract `Rule`  │     │ Exception & Auto  │
│  Class Hierarchy  │     │  Class Hierarchy  │     │ Recovery Handler  │
└───────────────────┘     └───────────────────┘     └───────────────────┘
          │                         │                         │
          └─────────────────────────┼─────────────────────────┘
                                    │
                    ┌───────────────▼───────────────┐
                    │            Logger             │
                    │    Persistent Audit Stream    │
                    └───────────────────────────────┘
```

### 2.1 Component Responsibility Matrix

| Component | Responsibility | Key Classes |
| :--- | :--- | :--- |
| **Orchestrator** | Manages workflow execution loop & task queues | `WorkflowManager` |
| **Task Engine** | Encapsulates single file operations | `Task`, `RenameTask`, `MoveTask`, `BackupTask`, `VerifyTask`, `CreateFolderTask` |
| **SmartSort Engine** | Evaluates files against categorization rules | `Rule`, `FileTypeRule`, `SizeRule`, `AgeRule`, `SmartSortEngine` |
| **Resilience Engine**| Detects failures & executes recovery strategies | `ResilienceEngine` |
| **Logger** | Writes persistent execution logs to `FLOW.LOG` | `Logger` |

---

## 3. Data Model & Class Specifications

### 3.1 Core Data Structures (`fileinfo.h`)

```cpp
struct FileInfo {
    char name[64];         // File name (e.g. "assignment.pdf")
    char extension[10];    // Extracted file extension (e.g. "pdf")
    char sourcePath[128];  // Source directory path
    char destPath[128];    // Destination directory path
    long sizeBytes;        // File size in bytes
    int  ageDays;          // Creation age in days
    int  isVirtual;        // 1 = Simulated sandbox file, 0 = Physical DOS file
};

enum TaskResult { SUCCESS, FAILURE, SKIPPED };
enum ErrorType  { ERR_FOLDER_NOT_FOUND, ERR_FILE_NOT_FOUND, ERR_UNKNOWN };
```

---

### 3.2 Task Class Hierarchy (Command Pattern)

The `Task` class forms the abstract interface for all executable pipeline steps.

```cpp
class Task {
protected:
    char taskName[64];
    FileInfo file;
    int maxRetries;
    int retryCount;

public:
    Task(const char* name, FileInfo f, int retries = 2);
    
    // Pure Virtual Functions (Abstraction Contract)
    virtual TaskResult execute() = 0;
    virtual void describe() = 0;

    const char* getTaskName();
    FileInfo    getFile();
    virtual ~Task();
};
```

#### Concrete Task Implementations:
- **`RenameTask`**: Modifies `file.name` using `<stdio.h>` `rename()` or virtual simulation.
- **`CreateFolderTask`**: Creates target directories using `<dir.h>` `mkdir()`.
- **`MoveTask`**: Relocates files across paths by calling `rename(srcFull, destFull)`.
- **`BackupTask`**: Creates a binary duplicate with a `_BAK` suffix using stream buffer operations.
- **`VerifyTask`**: Verifies backup existence (`access()`) and checks non-zero file byte length.
- **`SmartSortTask`**: Queries `SmartSortEngine` to compute destination paths, then delegates to `MoveTask`.

---

### 3.3 Rule Class Hierarchy (Strategy Pattern)

The `Rule` class provides an abstract filter interface for file classification.

```cpp
class Rule {
protected:
    char destinationFolder[128];

public:
    Rule(const char* destFolder);

    // Pure Virtual Interface
    virtual int matches(FileInfo file) = 0;
    virtual void describe() = 0;

    const char* getDestination();
    virtual ~Rule();
};
```

#### Concrete Rule Implementations:
- **`FileTypeRule`**: Performs extension string comparison (`strcmp(file.extension, targetExt)`).
- **`SizeRule`**: Evaluates condition `file.sizeBytes >= minSizeBytes`.
- **`AgeRule`**: Evaluates condition `file.ageDays >= minAgeDays`.

---

### 3.4 Resilience Engine Specification

The `ResilienceEngine` handles runtime errors using a structured decision matrix:

```text
                     ┌───────────────────────────┐
                     │   Task Returns FAILURE    │
                     └─────────────┬─────────────┘
                                   │
                     ┌─────────────▼─────────────┐
                     │    detectError(task)      │
                     └─────────────┬─────────────┘
                                   │
         ┌─────────────────────────┼─────────────────────────┐
         │                         │                         │
┌────────▼─────────┐      ┌────────▼─────────┐      ┌────────▼─────────┐
│ERR_FOLDER_NOT_FND│      │ERR_FILE_NOT_FOUND│      │   ERR_UNKNOWN    │
└────────┬─────────┘      └────────┬─────────┘      └────────┬─────────┘
         │                         │                         │
┌────────▼─────────┐      ┌────────▼─────────┐      ┌────────▼─────────┐
│ Auto-Fix Strategy│      │ Fallback Strategy│      │  Retry Strategy  │
│ (mkdir destPath) │      │ (Alt Source Path)│      │  (Up to Max)     │
└────────┬─────────┘      └────────┬─────────┘      └────────┬─────────┘
         │                         │                         │
         └─────────────────────────┼─────────────────────────┘
                                   │
                     ┌─────────────▼─────────────┐
                     │   Task Retry Successful?  │
                     └───┬───────────────────┬───┘
                         │                   │
                      YES│                 NO│
                         ▼                   ▼
                 ┌───────────────┐   ┌───────────────┐
                 │ Return RECOVER│   │ Return ABORT  │
                 └───────────────┘   └───────────────┘
```

> [!TIP]
> **Resilience Guarantee**: If a target folder is missing, `autoFix()` automatically creates the folder structure using `mkdir()` and re-executes the failed task without aborting the workflow.

---

## 4. Execution Sequence Trace

Below is the step-by-step trace of an execution pipeline ("Organize and Backup Downloads"):

```text
Step 1: SmartSortTask
  ├── Query SmartSortEngine with file "assignment.pdf"
  ├── FileTypeRule matches extension "pdf" -> target "C:\DOCS\"
  └── Perform relocation via MoveTask -> SUCCESS ✓

Step 2: CreateFolderTask
  ├── Target directory: "C:\DOCS\BACKUP\"
  └── Directory already exists -> SKIPPED →

Step 3: MoveTask
  ├── Relocate file to "C:\DOCS\BACKUP\"
  ├── Directory check fails -> FAILURE ✗
  ├── ResilienceEngine triggered:
  │     ├── Detect Error: ERR_FOLDER_NOT_FOUND
  │     ├── Auto-Fix: mkdir("C:\DOCS\BACKUP\")
  │     └── Retry MoveTask -> SUCCESS ✓ (RECOVERED)

Step 4: BackupTask
  ├── Binary byte copy to "assignment_BAK.pdf" -> SUCCESS ✓

Step 5: VerifyTask
  ├── Verify file integrity (size > 0 bytes) -> SUCCESS ✓

============================================================
           WORKFLOW COMPLETED SUCCESSFULLY
============================================================
```

---

## 5. OOP Principles Academic Mapping

| Syllabus Topic | Project Implementation Reference |
| :--- | :--- |
| **Classes & Objects** | Instantiation of `FileInfo`, `WorkflowManager`, `Task`, `Rule`, and `Logger` objects. |
| **Data Encapsulation** | Private attributes (`workflowName`, `taskList`, `logFilePath`) protected from direct manipulation. |
| **Data Abstraction** | Abstract base classes `Task` and `Rule` hiding underlying file operations. |
| **Inheritance** | Derived task hierarchies (`public Task`) and rule hierarchies (`public Rule`). |
| **Polymorphism** | Heterogeneous pointer arrays (`Task* taskList[20]`) invoking `execute()` dynamically. |
| **Dynamic Memory** | Dynamic allocation (`new`) and cleanup (`delete`) in `WorkflowManager` destructors. |
| **File I/O Streams** | Standard C file stream operations (`fopen`, `fprintf`, `fgets`, `fclose`) in `Logger`. |
| **Constructor Overloading** | Default arguments in constructors (`Task(name, file, retries = 2)`). |

---

## 6. Turbo C++ Environment Constraints & Solutions

> [!WARNING]
> Turbo C++ runs in a 16-bit DOS environment with specific memory and library limitations.

1. **8.3 Filename Convention**: Physical DOS mode truncates long file names to 8 characters + 3 extension characters. FlowForge includes a **Virtual Sandbox Mode** (`file.isVirtual = 1`) to allow full testing of long filenames during demonstrations.
2. **Standard Library Constraints**: Uses classic `<iostream.h>`, `<fstream.h>`, `<dir.h>`, `<stdio.h>`, and `<conio.h>` headers instead of modern C++ standard library templates.
3. **Memory Model**: Uses explicit pointer arrays instead of `std::vector` to prevent segment overflow within the 64 KB 16-bit memory limit.

---

*FlowForge Specification Guide — Built for Object-Oriented Programming Course Evaluation.*
