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

## 🧰 Workflow Management

The main menu supports up to 10 workflows during one program run. Create multiple workflows, select one to view or execute again, and delete workflows you no longer need. Workflows are kept in memory for the current run and are not restored after the program exits.

When selecting a folder, choose a previously saved path, extend a saved path with a subfolder (for example, `CourseProject`), or enter a completely new path. New and extended paths are saved in `PATHS.DAT` for later runs. Enter file names and extensions separately; for example, use `hello` as the name and `txt` as the extension for `hello.txt`.

The Add Tasks menu separates SmartSort setup from sorting a file. Choose **6. Add SmartSort Rules and Destinations** to add rules (for example, `.txt` files go to one folder); existing rules stay in place, and you can append more later. Then choose **7. Add File to SmartSort** to enter the file name, extension, and source folder for a sorting task. You can add several files using the saved rules. When a file matches multiple rules with different destinations, SmartSort copies it to each unique matching destination and removes the source only after all copies succeed. Use **Change SmartSort Rule Destination** in the main menu to update an existing rule later.

File name, extension, and source folder are entered once per workflow, on the first task that needs a file. Other file tasks in that workflow reuse the same file. Successful rename, move, and SmartSort operations update the file details used by later tasks.

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
| **Polymorphism** | `Task* taskList[20]` heterogeneous pointer array in each `WorkflowManager` executing polymorphic tasks dynamically at runtime. |
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
- [X] Implemented `MoveTask::execute()` (file relocation across directory paths)
- [X] Implemented `BackupTask::execute()` (byte-by-byte binary file copy)
- [X] Implemented `VerifyTask::execute()` (file existence and non-zero size verification)
- [X] Implemented `SmartSortTask::execute()` (rule lookup & automatic relocation)
- [X] Implemented `SmartSortEngine::getMatchingDestinations()` (scanning all active rules)
- [X] Implemented `SmartSortEngine::showAllRules()` & rule management
- [X] Implemented `FileTypeRule::matches()` (matching file extensions via `strcmp`)
- [X] Implemented `SizeRule::matches()` (matching files by byte size threshold)
- [X] Implemented `AgeRule::matches()` (matching files by age in days)
- [X] Implement `ResilienceEngine::detectError()` (identifying missing folders/files)
- [X] Implement `ResilienceEngine::autoFix()` (auto-creating missing destination folders)
- [X] Implement `ResilienceEngine::retryTask()` (retry attempts up to `maxRetries`)
---

## 📋 Open Task Checklist

Collaborators can pick any open task below to implement:

### 🔍 SmartSort Rules Engine
-Implementation Complete

### 🛡️ Resilience Engine
- [ ] Implement `ResilienceEngine::handle()` (top-level recovery dispatcher)

### 💻 UI & Interactive Menu Integration
- [x] Create and manage multiple workflows from the main menu (Options 1–3 and 6)
- [x] View workflow tasks and execute a selected workflow again
- [x] View and clear execution logs (Options 4 and 5)
- [x] Select saved folder paths, extend them, or add a new path

---

## 💻 How to Compile & Run in Turbo C++

1. Open **Turbo C++ (Borland IDE)**.
2. Go to **File ➔ Open** and select `FLOWFORG.CPP`.
3. Press **Ctrl + F9** to compile and run.
4. Use the main menu to create workflows, select saved workflows to view or execute, delete workflows, and manage execution logs. Workflows last for the current program run; saved folder paths remain available in `PATHS.DAT`.

> [!TIP]
> Ensure path separators use `\\` when specifying destination folders in DOS mode.
