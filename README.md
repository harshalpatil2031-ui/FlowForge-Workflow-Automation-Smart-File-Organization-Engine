# 🚀 FLOWFORGE — Workflow Automation & Smart File Organization Engine

> **Course Project**: Object-Oriented Programming (OOP)  
> **Environment**: Turbo C++ (16-bit Borland C++)  
> **Paradigm**: Object-Oriented Architecture  

---

## 📌 Problem Statement

Every day, we perform repetitive file operations manually: renaming files, sorting Downloads folders, moving documents into subject folders, creating backups, and verifying backups.

Current file managers perform single operations (copy, move, rename), but do not allow combining multiple tasks into an automated workflow with **intelligent failure recovery**.

---

## 💡 Solution: FlowForge

**FlowForge** combines three powerful features into one unified engine:

1. **Workflow Automation**: Define a sequence of file tasks once and execute the entire pipeline with one click.
2. **SmartSort**: Automatically classify and organize files into folders based on custom rules (file type, size, age).
3. **Resilience Engine**: Detects runtime failures (e.g., missing destination folder) and automatically executes recovery strategies (Auto-Fix missing folder, Retry, Fallback, Abort).

$$\text{FlowForge} = \text{Workflow Automation} + \text{SmartSort} + \text{Failure Recovery}$$

---

## 🏗️ System Architecture

```
                      ┌─────────────────────────┐
                      │      FLOWFORG.CPP       │
                      │  (Interactive Menu UI)  │
                      └────────────┬────────────┘
                                   │
                    ┌──────────────▼──────────────┐
                    │       WorkflowManager       │
                    │   Manages & Executes Tasks  │
                    └──────────────┬──────────────┘
                                   │
          ┌────────────────────────┼────────────────────────┐
          │                        │                        │
┌─────────▼─────────┐    ┌─────────▼─────────┐    ┌─────────▼─────────┐
│     Task Engine   │    │  SmartSort Engine │    │ Resilience Engine │
│ Abstract `Task`   │    │ Abstract `Rule`   │    │ Error detection & │
│ Base Class        │    │ Base Class        │    │ Auto-Fix Recovery │
└───────────────────┘    └───────────────────┘    └───────────────────┘
          │                        │                        │
          └────────────────────────┼────────────────────────┘
                                   │
                    ┌──────────────▼──────────────┐
                    │           Logger            │
                    │   Writes to `FLOW.LOG`      │
                    └─────────────────────────────┘
```

---

## 🛠️ OOP Concepts Implemented

- **Abstraction**: Pure virtual functions `Task::execute() = 0`, `Task::describe() = 0`, and `Rule::matches() = 0`.
- **Inheritance**: Derived tasks (`RenameTask`, `MoveTask`, `BackupTask`, `VerifyTask`, `CreateFolderTask`, `SmartSortTask`) and derived rules (`FileTypeRule`, `SizeRule`, `AgeRule`).
- **Polymorphism**: `Task* taskList[20]` array in `WorkflowManager` executing polymorphic tasks dynamically.
- **Encapsulation**: Private and protected data members accessed via public getter/setter methods.
- **File Handling**: Logging history to `FLOW.LOG` and reading/writing log entries using C file streams.

---

## 📂 Project Files

- `FLOWFORG.CPP`: Core C++ source code containing all class definitions, engine structures, and interactive menu.
- `FlowForge_Implementation.md`: Complete architectural reference and implementation blueprint.
- `README.md`: Project documentation and open task checklist.

---

## ✅ Foundation Completed (Team Lead)

- [x] `FileInfo` struct (shared file data model)
- [x] `TaskResult` and `ErrorType` enums
- [x] Abstract `Task` base class
- [x] Abstract `Rule` base class
- [x] `Logger` class (writes history to `FLOW.LOG`)
- [x] `WorkflowManager` class skeleton & polymorphic task execution loop
- [x] Main interactive menu shell (`conio.h`)

---

## 📋 Remaining To-Do Checklist

Collaborators can pick any unchecked task below to implement:

### Task Engine Implementation
- [ ] Implement `RenameTask::execute()` (real file rename & virtual mode handling)
- [ ] Implement `MoveTask::execute()` (file relocation across directory paths)
- [ ] Implement `CreateFolderTask::execute()` (directory creation using `mkdir()`)
- [ ] Implement `BackupTask::execute()` (byte-by-byte binary file copying)
- [ ] Implement `VerifyTask::execute()` (file existence and non-zero size verification)
- [ ] Implement `SmartSortTask::execute()` (rule lookup & automatic file relocation)

### SmartSort Rules Engine
- [ ] Implement `FileTypeRule::matches()` (matching file extensions via `strcmp`)
- [ ] Implement `SizeRule::matches()` (matching files by byte size threshold)
- [ ] Implement `AgeRule::matches()` (matching files by age in days)
- [ ] Implement `SmartSortEngine::getDestination()` (scanning active rules)
- [ ] Implement `SmartSortEngine::showAllRules()` & rule management

### Resilience Engine
- [ ] Implement `ResilienceEngine::detectError()` (identifying missing folders/files)
- [ ] Implement `ResilienceEngine::autoFix()` (automatically creating missing destination folders)
- [ ] Implement `ResilienceEngine::retryTask()` (executing retry attempts up to `maxRetries`)
- [ ] Implement `ResilienceEngine::handle()` (top-level recovery dispatcher)

### UI & Interactive Workflow Management
- [ ] Wire interactive workflow creation in `main()` menu (Option 1)
- [ ] Wire interactive task viewing in `main()` menu (Option 2)
- [ ] Wire execution trigger in `main()` menu (Option 3)
- [ ] Wire log viewer in `main()` menu (Option 4 & 5)

---

## ⚙️ How to Compile & Run in Turbo C++

1. Open **Turbo C++**.
2. Go to **File -> Open** and select `FLOWFORG.CPP`.
3. Press **Ctrl + F9** to compile and run.
4. Use the interactive menu to create, inspect, and execute workflows!
