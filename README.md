# 🚀 FlowForge — Workflow Automation & Smart File Organization Engine

<p align="center">
  <img src="https://img.shields.io/badge/Language-C%2B%2B-blue.svg?style=for-the-badge&logo=cplusplus" alt="Language C++">
  <img src="https://img.shields.io/badge/Compiler-Turbo%20C%2B%2B%203.0-orange.svg?style=for-the-badge" alt="Turbo C++">
  <img src="https://img.shields.io/badge/Paradigm-Object--Oriented-green.svg?style=for-the-badge" alt="OOP">
  <img src="https://img.shields.io/badge/Course-OOP%20Project-purple.svg?style=for-the-badge" alt="OOP Course Project">
</p>

---

## 📌 Executive Summary

**FlowForge** is an intelligent, self-recovering workflow automation and file organization engine developed in Turbo C++. 

It addresses the friction of performing manual, repetitive file operations—such as sorting downloaded assignments, renaming documents, moving files across subject directories, creating backups, and verifying file integrity—by allowing users to construct reusable, automated **Workflows** backed by a **Self-Recovering Resilience Engine**.

> [!IMPORTANT]
> **Key Innovation**: Unlike traditional file managers that fail silently or abort on errors, FlowForge monitors every operation and automatically executes recovery strategies (e.g., auto-creating missing destination folders, retrying operations, or applying fallback paths).

---

## 💡 Core Concept

$$\text{FlowForge} = \text{Workflow Automation} + \text{SmartSort} + \text{Resilience Engine}$$

- **Workflow Automation**: Group multiple file tasks into a single executable, reusable pipeline.
- **SmartSort Engine**: Rule-based automatic file classification by extension, size, or creation age.
- **Resilience Engine**: Intelligent error detection and automated self-recovery.

---

## 🏗️ System Architecture

```
┌────────────────────────────────────────────────────────────────────────┐
│                              FLOWFORG.CPP                              │
│                      (Console UI & Main Menu)                          │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │
                    ┌───────────────▼───────────────┐
                    │        WorkflowManager        │
                    │   Manages & Executes Tasks    │
                    └───────────────┬───────────────┘
                                    │
          ┌─────────────────────────┼─────────────────────────┐
          │                         │                         │
┌─────────▼─────────┐     ┌─────────▼─────────┐     ┌─────────▼─────────┐
│    Task Engine    │     │  SmartSort Engine │     │ Resilience Engine │
│ Abstract `Task`   │     │ Abstract `Rule`   │     │ Error Detection & │
│ Base Class        │     │ Base Class        │     │ Auto-Fix Recovery │
└───────────────────┘     └───────────────────┘     └───────────────────┘
          │                         │                         │
          └─────────────────────────┼─────────────────────────┘
                                    │
                    ┌───────────────▼───────────────┐
                    │            Logger             │
                    │   Writes to `FLOW.LOG`        │
                    └───────────────────────────────┘
```

---

## 🧬 OOP Concepts Mapping

| OOP Concept | Implementation Details |
| :--- | :--- |
| **Abstraction** | Pure virtual methods `Task::execute() = 0`, `Task::describe() = 0`, and `Rule::matches() = 0` hide implementation details behind clean interfaces. |
| **Inheritance** | Derived task hierarchy (`RenameTask`, `MoveTask`, `CreateFolderTask`, `BackupTask`, `VerifyTask`, `SmartSortTask`) and rule hierarchy (`FileTypeRule`, `SizeRule`, `AgeRule`). |
| **Polymorphism** | `Task* taskList[20]` heterogeneous pointer array in `WorkflowManager` executing polymorphic tasks dynamically at runtime. |
| **Encapsulation** | Protected data attributes (`taskName`, `file`, `destinationFolder`) accessed strictly via public accessor functions. |
| **File Handling** | Audit trail written to `FLOW.LOG` using C file streams (`fopen`, `fprintf`, `fgets`, `fclose`). |

---

## 📂 Project Structure

```text
FlowForge/
├── FLOWFORG.CPP                # Core C++ source file containing all classes & main loop
├── FlowForge_Implementation.md  # Detailed technical design blueprint & class specifications
└── README.md                   # Project documentation & task roadmap
```

---

## ✅ Foundation Completed (Team Lead)

- [x] `FileInfo` struct (shared file data model)
- [x] `TaskResult` and `ErrorType` enums
- [x] Abstract `Task` base class
- [x] Abstract `Rule` base class
- [x] `Logger` class (writes history to `FLOW.LOG`)
- [x] `WorkflowManager` class skeleton & polymorphic execution loop
- [x] Main interactive menu shell (`conio.h`)
- [x] `RenameTask` class (real file rename & virtual sandbox mode)
- [x] `CreateFolderTask` class (directory creation via `mkdir()`)
- [X] Implement `MoveTask::execute()` (file relocation across directory paths)
- [X] Implement `BackupTask::execute()` (byte-by-byte binary file copy)
- [X] Implement `VerifyTask::execute()` (file existence and non-zero size verification)
- [X] Implement `SmartSortTask::execute()` (rule lookup & automatic relocation)
---

## 📋 Open Task Checklist

Collaborators can pick any open task below to implement:

### 🔍 SmartSort Rules Engine
- [ ] Implement `FileTypeRule::matches()` (matching file extensions via `strcmp`)
- [ ] Implement `SizeRule::matches()` (matching files by byte size threshold)
- [ ] Implement `AgeRule::matches()` (matching files by age in days)
- [ ] Implement `SmartSortEngine::getDestination()` (scanning active rules)
- [ ] Implement `SmartSortEngine::showAllRules()` & rule management

### 🛡️ Resilience Engine
- [ ] Implement `ResilienceEngine::detectError()` (identifying missing folders/files)
- [ ] Implement `ResilienceEngine::autoFix()` (auto-creating missing destination folders)
- [ ] Implement `ResilienceEngine::retryTask()` (retry attempts up to `maxRetries`)
- [ ] Implement `ResilienceEngine::handle()` (top-level recovery dispatcher)

### 💻 UI & Interactive Menu Integration
- [ ] Wire interactive workflow creation in `main()` menu (Option 1)
- [ ] Wire interactive task viewing in `main()` menu (Option 2)
- [ ] Wire execution trigger in `main()` menu (Option 3)
- [ ] Wire log viewer in `main()` menu (Option 4 & 5)

---

## 💻 How to Compile & Run in Turbo C++

1. Open **Turbo C++ (Borland IDE)**.
2. Go to **File ➔ Open** and select `FLOWFORG.CPP`.
3. Press **Ctrl + F9** to compile and run.
4. Navigate the interactive menu using numerical choices.

> [!TIP]
> Ensure path separators use `\\` when specifying destination folders in DOS mode.
